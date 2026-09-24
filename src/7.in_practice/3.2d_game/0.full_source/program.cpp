/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "resource_manager.h"

#include <iostream>
#include <memory>
#include <string>

// GLFW function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The Width of the screen
const unsigned int SCREEN_WIDTH = 800;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 600;

std::unique_ptr<Game> Breakout;

int main(int argc, char *argv[])
{
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // OpenGL configuration
    // --------------------
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // initialize game
    // ---------------
    Breakout = std::make_unique<Game>(SCREEN_WIDTH, SCREEN_HEIGHT);
    Breakout->Init();
    if (smokeTest)
    {
        for (unsigned int level = 0; level < Breakout->Levels.size(); ++level)
        {
            Breakout->Level = level;
            Breakout->ResetLevel();
            if (Breakout->Levels[level].Bricks.empty())
            {
                std::cerr << "Failed to reload level " << level << std::endl;
                Breakout.reset();
                ResourceManager::Clear();
                glfwTerminate();
                return 1;
            }
        }
        Breakout->Level = 0;
        Breakout->State = GAME_ACTIVE;
        Breakout->Keys[GLFW_KEY_SPACE] = true;
        std::cout << "Smoke test: all four levels reload successfully" << std::endl;
    }
    const double startTime = glfwGetTime();
    bool smokeFailed = false;
    bool checkedViewport = false;

    // deltaTime variables
    // -------------------
    float deltaTime = 0.0f;
    float lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0)
            continue;

        // manage user input
        // -----------------
        Breakout->ProcessInput(deltaTime);

        // update game state
        // -----------------
        Breakout->Update(deltaTime);

        // render
        // ------
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Breakout->Render();
        if (smokeTest)
        {
            if (!checkedViewport)
            {
                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                smokeFailed = viewport[0] != 0 || viewport[1] != 0 ||
                    viewport[2] != framebufferWidth || viewport[3] != framebufferHeight;
                // Read the actual back buffer: all four quadrants must contain the scene.
                glReadBuffer(GL_BACK);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                for (int y = 1; y <= 3; y += 2)
                    for (int x = 1; x <= 3; x += 2)
                    {
                        unsigned char pixel[3] = {};
                        glReadPixels(framebufferWidth * x / 4, framebufferHeight * y / 4,
                            1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
                        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
                            smokeFailed = true;
                    }
                glPixelStorei(GL_PACK_ALIGNMENT, 4);
                std::cout << "Framebuffer " << framebufferWidth << 'x' << framebufferHeight
                    << ": viewport and four-quadrant check " << (smokeFailed ? "FAILED" : "passed") << std::endl;
                checkedViewport = true;
            }
            for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
            {
                std::cerr << "OpenGL error: " << error << std::endl;
                smokeFailed = true;
            }
            if (smokeFailed)
                glfwSetWindowShouldClose(window, true);
        }

        glfwSwapBuffers(window);
        if (smokeTest && glfwGetTime() - startTime >= 6.0)
            glfwSetWindowShouldClose(window, true);
    }

    Breakout.reset(); // release GPU and audio resources while the context is alive

    // delete all resources as loaded using the resource manager
    // ---------------------------------------------------------
    ResourceManager::Clear();

    glfwTerminate();
    return smokeFailed ? 1 : 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // when a user presses the escape key, we set the WindowShouldClose property to true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            Breakout->Keys[key] = true;
        else if (action == GLFW_RELEASE)
        {
            Breakout->Keys[key] = false;
            Breakout->KeysProcessed[key] = false;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
