#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "curl/curl.h"

class Downloader {
	CURLM* multi_curl = nullptr;
	std::unordered_map<CURL*, int> curl_to_id;
public:
	Downloader();

	~Downloader();

	Downloader(Downloader&&) = delete;
	Downloader(const Downloader&) = delete;

	static void download_file(const std::string& url, std::vector<uint8_t>& buffer);

	void add_file(const std::string& url, std::vector<uint8_t>& buffer);

	void start_multi_file();

	std::vector<int> get_finished_files();

	void cleanup();

};
