#pragma once

#include <map>
#include <vector>

#include "core.hpp"
#include "game.hpp"
#include "Player.hpp"

namespace Game {

class State {
private:
	std::map<i32, Player>	players;
	std::vector<GameEvent>	events;
	i32						nextPlayerId;

public:
	State(): nextPlayerId(1) {}

	i32	addPlayer() {
		i32	id = nextPlayerId;
		nextPlayerId += 1;
		players[id] = Player(id);
		return (id);
	}

	void	removePlayer(i32 id) {
		players.erase(id);
	}

	void	movePlayer(i32 id, f64 x, f64 y) {
		Player*	player = &players[id];

	//	if (inbounds)
			player->setPosition(x, y);
	}

	void	update() {

	}

	void	serializePendingEvents() {

	}
};
}
