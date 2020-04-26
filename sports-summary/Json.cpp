#include "Json.hpp"
#include "errors.hpp"

#include "curl/curl.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

using std::string;
using std::vector;

size_t Json::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	auto self = reinterpret_cast<Json*>(userp);
    self->update(reinterpret_cast<char*>(contents), nmemb);
    return nmemb;
}

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

void Json::update(char* data, int size) {
    for (int i = 0; i < size; ++i) {
        json_data.push_back(data[i]);
    }
}

std::vector<Game> Json::get_games(std::string date_string) {

    string url_start = "http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=";
    string url_suffix = "&sportId=1";
    string url = url_start + date_string + url_suffix;

    auto curl = curl_easy_init();

    if (curl == nullptr) {
        lib_error("Could not init cURL", __FILE__, __LINE__);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        download_error(curl_easy_strerror(res), __FILE__, __LINE__);
    }

	curl_easy_cleanup(curl);

	json j_filtered = json::parse(json_data, callback);

	auto dates = j_filtered["dates"];

	auto info = dates[0];

	auto date = info["date"].get<string>();
	auto num_games = info["totalGames"].get<int>();

	vector<Game> games;
	games.reserve(num_games);

	for (auto&& game : info["games"]) {
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
		games.push_back({ home_name, home_score, away_name, away_score, did_home_win, blurb, url });
	}
		
	return games;
}