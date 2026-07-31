#pragma once

#include <core.hpp>
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

	struct GameEvent {
		enum Type { PLAYER_JOINED, PLAYER_MOVED, PLAYER_LEFT };
		Type		type;
		i32			playerId;
		f64			x, y;
		Direction	direction;
		Color		color;

		// New Player or Player moved Event
		GameEvent(Type t, i32 id, f64 x, f64 y, Direction d, Color c) :
			type(t), playerId(id), x(x), y(y), direction(d), color(c) {}

		// Remove Player Event
		GameEvent(Type t, i32 id) :
			type(t), playerId(id) {}
	};
}
