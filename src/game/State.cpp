#include <sstream>
#include <algorithm>

#include "State.hpp"
#include "ServerManager.hpp"
#include "Connection.hpp"

Game::State::State() : nextPlayerId(1) {}
Game::State::~State() {}

i32 Game::State::addPlayer() {
	i32 id = nextPlayerId++;
	players[id] = Player(id);

	std::ostringstream ss;
	ss << "data: {\"type\":\"join\",\"id\":" << id
	   << ",\"x\":0,\"y\":0}\n\n";
	broadcastBuffer += ss.str();
	return id;
}

void Game::State::removePlayer(i32 id) {
	players.erase(id);
	std::ostringstream ss;
	ss << "data: {\"type\":\"leave\",\"id\":" << id << "}\n\n";
	broadcastBuffer += ss.str();
}

void Game::State::movePlayer(i32 id, f64 x, f64 y) {
	std::map<i32, Player>::iterator it = players.find(id);
	if (it == players.end())
		return;
	it->second.setPosition(x, y);

	std::ostringstream ss;
	ss << "data: {\"type\":\"move\",\"id\":" << id
	   << ",\"x\":" << x << ",\"y\":" << y << "}\n\n";
	broadcastBuffer += ss.str();
}

void Game::State::addSSEClient(void* conn) {
	if (std::find(sseClients.begin(), sseClients.end(), conn) == sseClients.end())
		sseClients.push_back(conn);
}

void Game::State::removeSSEClient(void* conn) {
	std::vector<void*>::iterator it = std::find(sseClients.begin(), sseClients.end(), conn);
	if (it != sseClients.end())
		sseClients.erase(it);
}

std::string Game::State::flushEvents() {
	std::string temp;
	temp.swap(broadcastBuffer);
	return temp;
}

void Game::State::broadcastEvents(ServerManager& manager) {
	if (sseClients.empty()) {
		broadcastBuffer.clear();
		return;
	}

	std::string data = flushEvents();
	if (data.empty())
		return;

	static const usize bufSize = 1024;
	for (usize i = 0; i < sseClients.size(); ++i) {
		HTTP::Connection<bufSize>* conn = static_cast<HTTP::Connection<bufSize>*>(sseClients[i]);
		conn->sse_buffer += data;
		manager.markConnectionWritable(conn->fd.client, conn);
	}
}

Game::Player* Game::State::getPlayer(i32 id) {
	return &players[id];
}
