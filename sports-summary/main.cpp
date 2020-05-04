// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// Main entry point. All the GLFW code is here, after setting everything up,
// The main loop does two things. It keeps polling the downloader to download
// the individual game images. It also responds to user input and calls the
// display code and swaps the buffers. While there are files to download,
// glfwWaitEventsTimeout is used to give ~60 fps. Once they are all downloaded,
// there is no reason to redraw except in response to input, so glfwWaitEvents
// is used instead.

#include <iostream>
#include <string>
#include <gsl/gsl>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "errors.hpp"
#include "utils.hpp"
#include "Downloader.hpp"
#include "get_games_from_json.hpp"
#include "Display.hpp"
#include "OpenGlDrawer.hpp"

using std::string;
using std::vector;

struct Info {
    size_t num_ids;
    size_t current_id;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        auto info = reinterpret_cast<Info*>(glfwGetWindowUserPointer(window));
        if (info->current_id > 0) {
            info->current_id -= 1;
        }
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        auto info = reinterpret_cast<Info*>(glfwGetWindowUserPointer(window));
        if (info->current_id < info->num_ids - 1) {
            info->current_id += 1;
        }
    }
}

int main()
{
    string date_string = get_today_date_string();
    // There aren't any picture right now so used the canned date:
    date_string = "2018-05-9";
    string url_start = "http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=";
    string url_suffix = "&sportId=1";
    string url = url_start + date_string + url_suffix;

    glfwInit();
    auto glfw_terminator = gsl::finally([] { glfwTerminate(); });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto monitor = glfwGetPrimaryMonitor();
    auto video_mode = glfwGetVideoMode(monitor);
    auto width = video_mode->width;
    auto height = video_mode->height;

#ifdef _DEBUG
    GLFWwindow* window = glfwCreateWindow(width, height, "Baseball - Sports Summary", nullptr, nullptr);
#else
    GLFWwindow* window = glfwCreateWindow(width, height, "Baseball - Sports Summary", monitor, nullptr);
#endif

    glfwMakeContextCurrent(window);
    if (window == nullptr)
    {
        ui_error("Error creating window", __FILE__, __LINE__);
    }

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        ui_error("Error loading OpenGL", __FILE__, __LINE__);
    }

    ImageManager image_manager("texture.jpeg");
    TextManager text_manager("text_atlas.png");

    OpenGlDrawer drawer(width, height, text_manager.get_texture());

    Display display(drawer, text_manager, image_manager, width, height);
    display.draw(0);

    glfwSwapBuffers(window);

    Downloader downloader;

    vector<uint8_t> json_data;

    downloader.download_file(url, json_data);

    auto games = get_games_from_json(json_data);

    image_manager.set_max_images(games.size());

    vector<vector<uint8_t>> image_buffers; // this is the owned version
    image_buffers.reserve(games.size());

    vector<DisplayItem> items;
    items.reserve(games.size());

    for (size_t id = 0; id < games.size(); ++id) {
        image_buffers.push_back(vector<uint8_t>{});
        downloader.add_file(games[id].url, image_buffers[id]);
        items.push_back(games[id].item);
    }
    downloader.start_multi_file();

    display.set_items(items);

    Info info{ items.size(), 0 };
    glfwSetWindowUserPointer(window, &info);

    std::unordered_set<int> images_left;
    images_left.reserve(items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        images_left.insert(static_cast<int>(i));
    }
    int try_again = 0;
    const int NUM_TRIES = 30;
    while (!glfwWindowShouldClose(window))
    {
        if (try_again < NUM_TRIES && images_left.size() > 0) {

            for (int image_id : downloader.get_finished_files()) {
                image_manager.add_jpeg(image_id, image_buffers[image_id]);
                images_left.erase(image_id);
                // don't need the buffer anymore
                image_buffers[image_id].clear();
            }
            ++try_again;
            if (try_again == NUM_TRIES) {
                for (int image_id : images_left) {
                    if (image_buffers[image_id].size() > 1000) {
                        image_manager.add_jpeg(image_id, image_buffers[image_id]);
                    }
                    // don't need the buffer anymore
                    image_buffers[image_id].clear();
                }
                // clean up
                image_buffers.clear();
                downloader.cleanup();
            }
            glfwWaitEventsTimeout(0.15);
        }
        else {
            glfwWaitEvents();
        }
        

        display.draw(info.current_id);

        glfwSwapBuffers(window);
    }

    return 0;
}
