#pragma once

#include <vector>
#include <cstdint>
#include "Game.hpp"

std::vector<Game> get_games_from_json(std::vector<uint8_t> json_data);

