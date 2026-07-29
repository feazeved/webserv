#pragma once

#include <map>
#include <vector>

#include "HTTP.hpp"
#include "core.hpp"
#include "Connection.hpp"

namespace Game {

class State {
private:
	std::map<i32, HTTP::Player>		players;
	std::vector<HTTP::GameEvent>	events;
	i32								nextPlayerId;

public:
	State(): nextPlayerId(1) {}

	i32	addPlayer() {
		i32	id = nextPlayerId++;

		players[id] = Game::Player(id);

		return (id);
	}

	void	removePlayer();
	void	movePlayer();
	void	update();
	void	serializePendingEvents();
};
}
