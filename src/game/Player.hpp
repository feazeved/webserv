#pragma once

#include "core.hpp"
#include "game.hpp"

namespace Game {
class Player {
public:
	i32			id;
	f64			x;
	f64			y;
	Direction	direction;
	Color		color;
	bool		moving;

public:
	Player() : x(0), y(0), direction(DOWN), moving(false) {}
	Player(i32 i) : id(i), x(0), y(0), direction(DOWN), moving(false) {}

	void	setPosition(f64 new_x, f64 new_y) {
		x = new_x;
		y = new_y;
	}
};
}
