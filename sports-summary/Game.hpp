// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.

// This is the result of parsing the JSON, and is a regular item, plus the
// URL of the image to be downloaded.

#pragma once

#include <string>
#include "DisplayItem.hpp"

struct Game {
	DisplayItem item;
	std::string url;
};
