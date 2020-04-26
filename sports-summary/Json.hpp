#pragma once

#include <vector>
#include <string>

struct Game {
	std::string home_team;
	int home_score;
    std::string away_team;
	int away_score;
	bool did_home_win;
	std::string blurb;
	std::string url;
};

class Json {
	std::vector<unsigned char> json_data;
	void update(char* data, int size);
	static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
public:
	std::vector<Game> get_games(std::string date);
};

