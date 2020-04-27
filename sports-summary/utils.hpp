#pragma once

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> read_all_bytes(const char* filename);

std::string read_all_text(const char* filename);

std::string get_today_date_string();
