#include <iostream>
#include <gsl/gsl>
#include "gl.h"
#include "Display.hpp"
#include "errors.hpp"

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

    std::vector<DisplayItem> items{
        {0, " !\"#$%&'()*+,-./", "Game zero", 0},
        {1, "0123456789", "Game one", 1},
        {2, ":;<=>?@", "Game two", 2},
        {3, "ABCDEFGHIJKLMN", "Game three", 3},
        {4, "OPQRSTUVWXYZ", "Game four", 4},
        {5, "[\]^_`", "Game five", 5},
        {6, "abcdefghijklmn", "Game six", 6},
        {7, "opqrstuvwxyz", "Game seven", 7},
        {8, "{|}~", "Game eight", 8},
    };

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
