#pragma once

#include <map>
#include <vector>
#include <string>

#include "Player.hpp"

class ServerManager;

namespace HTTP {
	template <usize bufferSize> class Connection;
}

namespace Game {

class State {
public:
	std::vector<void*>      sseClients;
	std::string             broadcastBuffer;

	State();
	~State();

	i32  addPlayer();
	void removePlayer(i32 id);
	void movePlayer(i32 id, f64 x, f64 y);

	void addSSEClient(void* conn);
	void removeSSEClient(void* conn);

	std::string flushEvents();
	void broadcastEvents(ServerManager& manager);

	Player* getPlayer(i32 id);

private:
	std::map<i32, Player>   players;
	i32                     nextPlayerId;

	static const i32 s_spawnX = 0;
	static const i32 s_spawnY = 0;
};

}