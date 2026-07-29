#pragma once

#include "core.hpp"
#include ""

namespace Game {
	struct Player {
		i32			id;
		f64			x;
		f64			y;
		Direction	direction;
		Color		color;
		bool		moving;
	
		Player() : x(0), y(0), direction(DOWN), moving(false) {}
		Player(i32 i) : id(i), x(0), y(0), direction(DOWN), moving(false) {}
	};

}
