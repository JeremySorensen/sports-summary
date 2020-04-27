// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// Functions to read from a file, and to get todays date in the form 
// yyyy-mm-dd.

#pragma once

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> read_all_bytes(const char* filename);

std::string read_all_text(const char* filename);

std::string get_today_date_string();
