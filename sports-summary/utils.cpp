#include <fstream>
#include <iostream>
#include <ctime>
#include "utils.hpp"
#include "errors.hpp"

using std::vector;
using std::string;
using std::ifstream;

vector<uint8_t> read_all_bytes(const char* filename) {
	ifstream input(filename, std::ios::binary | std::ios::ate);
	if (!input) {
		file_error((string("Could not open file '") + filename + "'").c_str(), __FILE__, __LINE__);
	}

	auto pos = input.tellg();
	auto length = static_cast<size_t>(pos);
	input.seekg(0, std::ios::beg);
	vector<uint8_t> bytes(length);
	input.read(reinterpret_cast<char*>(bytes.data()), length);
	return bytes;
}

std::string read_all_text(const char* filename) {
	// the type of newline doesn't matter so we avoid the cost of translating them by opening in binary
	ifstream input(filename, std::ios::binary | std::ios::ate);
	if (!input) {
		file_error((string("Could not open file '") + filename + "'").c_str(), __FILE__, __LINE__);
	}

	auto pos = input.tellg();
	auto length = static_cast<size_t>(pos);
	input.seekg(0, std::ios::beg);
	string text(length, ' ');
	input.read(text.data(), length);
	return text;
}

std::string get_today_date_string() {
	time_t rawtime;
	tm timeinfo;
	char buffer[11];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, 80, "%F", &timeinfo);

	return std::string { buffer };
}