#pragma once

#include "Player.hpp"

class Player;

namespace Game {
	enum Direction {
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	enum Color {
		BLUE,
		GREEN,
		RED,
		PINK,
		YELLOW,
	};

	enum EventType {
		JOIN,
		QUIT,
		MOVE
	};

	struct GameEvent {
		EventType	type;
		Player*		player;
	};

}
