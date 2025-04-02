#ifndef RENDERENGINE_KEYBOARDINPUT_H
#define RENDERENGINE_KEYBOARDINPUT_H

#include <GLFW/glfw3.h>

class KeyboardInput {
public:
    explicit KeyboardInput(GLFWwindow *window);

    bool isKeyPressed(int keyCode);

private:
    GLFWwindow *m_window = nullptr;
};

#endif