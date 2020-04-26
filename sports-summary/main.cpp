#include <iostream>
#include <ctime>
#include <gsl/gsl>
#include "gl.h"
#include "Display.hpp"
#include "errors.hpp"
#include "Json.hpp"

struct Info {
    int num_ids;
    int current_id;
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
    glfwInit();
    auto glfw_terminator = gsl::finally([] { glfwTerminate(); });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

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

    ImageManager image_manager("texture.jpg");
    TextManager text_manager("text_atlas.png");

    time_t rawtime;
    tm timeinfo;
    char buffer[11];

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    strftime(buffer, 80, "%F", &timeinfo);

    std::string date_string{ buffer };

    // no pictures today, use old date
    date_string = "2018-06-10";

    Json json;
    auto games = json.get_games(date_string);

    std::vector<DisplayItem> items;
    items.reserve(games.size());

    int id = 0;
    for (auto&& game : games) {

        std::string headline;
        if (game.did_home_win) {
            headline = game.home_team + " beat " + game.away_team + " at home " 
                + std::to_string(game.home_score) + "-" + std::to_string(game.away_score);
        }
        else {
            headline = game.away_team + " win " + std::to_string(game.away_score) + "-"
                + std::to_string(game.home_score) + " at " + game.home_team;
        }

        items.push_back({ id, headline, game.blurb, id });
        ++id;
    }

    Display display{ image_manager, text_manager, items, width, height };

    Info info{ items.size(), 0 };
    glfwSetWindowUserPointer(window, &info);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        display.draw(info.current_id);

        glfwSwapBuffers(window);
    }

    return 0;
}
