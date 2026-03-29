#include "Window.hpp"
#include <iostream>

namespace klab {

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_windowX(100), m_windowY(100), m_windowWidth(width), m_windowHeight(height), m_fullscreen(false) {
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, title.c_str(), NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create window. Your GPU might not support OpenGL 3.3 Core." << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0); // Disable VSync
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Note: " << glewGetErrorString(err) << " (Proceeding anyway...)" << std::endl;
    }

    glGetError();

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win) {
            win->m_width = width;
            win->m_height = height;
        }
        glViewport(0, 0, width, height);
    });
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::swapBuffers() const {
    glfwSwapBuffers(m_window);
}

void Window::close() {
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::toggleFullscreen() {
    m_fullscreen = !m_fullscreen;
    
    if (m_fullscreen) {
        glfwGetWindowPos(m_window, &m_windowX, &m_windowY);
        glfwGetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
        
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_window, NULL, m_windowX, m_windowY, m_windowWidth, m_windowHeight, 0);
    }
}

} // namespace klab
