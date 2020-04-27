// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// See header file for more information.

#include "get_games_from_json.hpp"
#include "errors.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

using std::string;
using std::vector;

static bool callback(int depth, json::parse_event_t event, json& parsed)
{
	if (event == json::parse_event_t::key) {
		switch (depth) {
		case 0:
			return true;
		case 1:
			return parsed == json("dates");
		case 2:
			return true;
		case 3:
			return parsed == json("date")
				|| parsed == json("totalGames")
				|| parsed == json("games");
		case 4:
			return true;
		case 5:
			return parsed == json("teams")
				|| parsed == json("content");
		case 6:
			return parsed == json("away")
				|| parsed == json("home")
				|| parsed == json("editorial");
		case 7:

			return parsed == json("score")
				|| parsed == json("team")
				|| parsed == json("isWinner")
				|| parsed == json("recap");
		case 8:
			return parsed == json("name")
				|| parsed == json("home");
		case 9:
			return parsed == json("photo")
				|| parsed == json("headline");
		case 10:
			return parsed == json("cuts");
		case 11:
			return parsed == json("320x180");
		case 12:
			return parsed == json("src");
		default:
			return false;
		}
	}
	else {
		return true;
	}
};

static string make_headline(
	string home,
	string away,
	int home_score,
	int away_score,
	bool home_did_win) {

	if (home_did_win) {
		return home + " beat " + away + " at home "
			+ std::to_string(home_score) + "-" + std::to_string(away_score);
	}
	else {
		return away + " win " + std::to_string(away_score) + "-"
			+ std::to_string(home_score) + " at " + home;
	}
}

std::vector<Game> get_games_from_json(vector<uint8_t> json_data) {

	json j_filtered = json::parse(json_data, callback);

	auto dates = j_filtered["dates"];

	auto info = dates[0];

	auto date = info["date"].get<string>();
	auto num_games = info["totalGames"].get<int>();

	vector<Game> games;
	games.reserve(num_games);

	int id = 0;
	for (auto && game : info["games"]) {
		auto away = game["teams"]["away"];
		auto away_score = away["score"].get<int>();
		auto away_name = away["team"]["name"].get<string>();
		auto home = game["teams"]["home"];
		auto home_score = home["score"].get<int>();
		auto home_name = home["team"]["name"].get<string>();
		auto did_home_win = home["isWinner"].get<bool>();
		auto editorial = game["content"]["editorial"]["recap"]["home"];
		auto blurb = editorial["headline"].get<string>();
		auto url = editorial["photo"]["cuts"]["320x180"]["src"].get<string>();
		games.push_back({
			DisplayItem {
				id,
				make_headline(home_name, away_name, home_score, away_score, did_home_win),
				blurb,
				id,
			},
			url }
		);
		++id;
	}
		
	return games;
}