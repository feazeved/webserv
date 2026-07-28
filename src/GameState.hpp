#pragma once

#include <map>
#include <vector>

#include "HTTP.hpp"
#include "core.hpp"
#include "http/Connection.hpp"

class GameState {
private:
	std::map<int, HTTP::Player>		players;
	std::vector<HTTP::GameEvent>	events;

public:
	void	addPlayer();
	void	removePlayer();
	void	movePlayer();
	void	update();
	void	serializePendingEvents();
};
