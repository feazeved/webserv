#pragma once

#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <sstream>

#include "core.hpp"
#include "game.hpp"
#include "Player.hpp"

namespace Game {

class State {
private:
	std::map<i32, Player>	players;
	std::vector<GameEvent>	pendingEvents;
	std::vector<i32>		sseClients;
	i32						nextPlayerId;

	static const i32		s_spawnX = 0;
	static const i32		s_spawnY = 0;

public:
	State(): nextPlayerId(1) {}

	i32	addPlayer() {
		i32	id = nextPlayerId++;
		players[id] = Player(id);

		GameEvent	ev(
			GameEvent::PLAYER_JOINED,
			id,
			s_spawnX,
			s_spawnY,
			Game::DOWN,
			players[id].color
		);
		pendingEvents.push_back(ev);
		return (id);
	}

	void	removePlayer(i32 id) {
		players.erase(id);

		GameEvent	ev(
			GameEvent::PLAYER_LEFT,
			id
		);
		pendingEvents.push_back(ev);
	}

	void	movePlayer(i32 id, f64 x, f64 y) {
		std::map<i32, Player>::iterator	it = players.find(id);

		if (it == players.end())
			return ;

		// Need to validate if the requested position is valid.
		// Maybe only send request if its valid in the html!
		GameEvent	ev(
			GameEvent::PLAYER_MOVED,
			id,
			x,
			y,
			it->second.direction,
			it->second.color
		);
		pendingEvents.push_back(ev);
	}

	void	update() {

	}

	void	addSSEClient(i32 fd) {
		sseClients.push_back(fd);
	}

	void	removeSSEClient(i32 fd) {
		std::vector<i32>::iterator	it = std::find(sseClients.begin(), sseClients.end(), fd);

		if (it == sseClients.end())
			return ;

		sseClients.erase(it);
	}

	void	broadcastEvents() {
		if (pendingEvents.empty())
			return ;

		std::ostringstream	ss;
		for (usize i = 0; i < pendingEvents.size(); i++) {
			ss << "data: {";
			GameEvent ev = pendingEvents[i];
			switch (ev.type) {
				case GameEvent::PLAYER_JOINED:
					ss << "\"type\":\"join\",\"id\":" << ev.playerId;
					break ;
				case GameEvent::PLAYER_LEFT:
					ss << "\"type\":\"leave\",\"id\":" << ev.playerId;
					break ;
				case GameEvent::PLAYER_MOVED:
					ss << "\"type\":\"move\",\"id\":" << ev.playerId
					   << ",\"x\":" << ev.x << ",\"y\":" << ev.y;
					break ;
			}
			ss << "}\n\n";
		}
		std::string data = ss.str();
		for (usize i = 0; i < sseClients.size(); i++) {
			// mark each fd as writable
			// have this data in the Server Buffer!!!
			// then, with epoll, send it to each fd
		}
		pendingEvents.clear();
	}

	Player*	getPlayer(i32 id) { return (&players[id]); }
	const std::vector<i32>&	getSSEClients() const { return (sseClients); }
};
}
