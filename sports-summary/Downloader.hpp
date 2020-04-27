// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.

// This class uses cURL to download files. There are two ways to do it.
// 'download_file' blocks until the file is downloaded.
// 'add_file' is used to add a file to a list of files to be downloaded
// asynchronosly. Once they are added, 'start_mult_file' is used to start the
// asynchronos downloads and returns immediately. 'get_finished_files' gives
// the ID of any files that finished downloading. (The ID is determined by
// the order in which the files are added using 'add_file', the first one has
// ID 0, next ID 1 and so on.) This class does NOT take ownership of the
// buffers so with the ID, the finished buffer can be used by the calling code.
// The 'cleanup' function can be used to tidy up when finished, but it is not
// necessary to call, as the destructor will clean up if needed.

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
