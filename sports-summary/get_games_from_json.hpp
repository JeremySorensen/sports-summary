// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.

// This function parses the json file using a json libarary from nlohmann.
// It uses a filter to avoid deserializing most of the JSON and only saves
// what is needed.

#pragma once

#include <vector>
#include <cstdint>
#include "Game.hpp"

std::vector<Game> get_games_from_json(std::vector<uint8_t> json_data);

