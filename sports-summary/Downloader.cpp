// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// See header file for more information.

#include "Downloader.hpp"
#include "errors.hpp"

using std::string;
using std::vector;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto buffer = reinterpret_cast<vector<uint8_t>*>(userp);
    auto data = reinterpret_cast<uint8_t*>(contents);
    for (size_t i = 0; i < nmemb; ++i) {
        buffer->push_back(data[i]);
    }
    return nmemb;
}

Downloader::Downloader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    multi_curl = curl_multi_init();
    if (multi_curl == nullptr) {
        lib_error("Could not init multi-cURL", __FILE__, __LINE__);
    }
}

void Downloader::cleanup() {
    for (auto&& pair : curl_to_id) {
        curl_multi_remove_handle(multi_curl, pair.first);
        curl_easy_cleanup(pair.first);
    }
    if (multi_curl != nullptr) {
        curl_multi_cleanup(multi_curl);
        multi_curl = nullptr;
    }
    curl_to_id.clear();
}

Downloader::~Downloader() { cleanup(); }

void Downloader::download_file(const string& url, vector<uint8_t>& buffer) {
    auto curl = curl_easy_init();
    if (curl == nullptr) {
        lib_error("Could not init cURL", __FILE__, __LINE__);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        download_error(curl_easy_strerror(res), __FILE__, __LINE__);
    }
    curl_easy_cleanup(curl);
}

void Downloader::add_file(const string& url, vector<uint8_t>& buffer) {
    auto handle = curl_easy_init();
    int id = curl_to_id.size();
    curl_to_id[handle] = id;

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);

    auto result = curl_multi_add_handle(multi_curl, handle);
    if (result != CURLMcode::CURLM_OK)
    {
        lib_error(curl_multi_strerror(result), __FILE__, __LINE__);
    }
}

void Downloader::start_multi_file() {
    int running_handles;
    auto result = curl_multi_perform(multi_curl, &running_handles);
    if (result != CURLMcode::CURLM_OK)
    {
        lib_error(curl_multi_strerror(result), __FILE__, __LINE__);
    }
}

vector<int> Downloader::get_finished_files() {
    int running_handles;

    auto result = curl_multi_perform(multi_curl, &running_handles);
    if (result != CURLMcode::CURLM_OK)
    {
        lib_error(curl_multi_strerror(result), __FILE__, __LINE__);
    }

    vector<int> ids;

    int num_messages = 0;
    auto messages = curl_multi_info_read(multi_curl, &num_messages);
    for (int i = 0; i < num_messages; ++i) {
        if (messages[i].msg == CURLMSG_DONE) {
            if (messages[i].data.result != CURLcode::CURLE_OK) {
                lib_error(curl_multi_strerror(result), __FILE__, __LINE__);
            }
            auto handle = messages[i].easy_handle;
            ids.push_back(curl_to_id[handle]);
            curl_to_id.erase(handle);
            curl_multi_remove_handle(multi_curl, handle);
            curl_easy_cleanup(handle);
        }
    }

    return ids;
}