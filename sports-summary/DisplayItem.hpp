// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.

// This struct stores the needed information to represent a single game item
// with picture and text.
// Note that currently the id and image_id are always the same value.

#pragma once

#include <string>

struct DisplayItem {
	int id;
	std::string headline;
	std::string text;
	int image_id;
};
