#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

namespace klab {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;
    void close();
    void toggleFullscreen();

    bool isFullscreen() const { return m_fullscreen; }
    GLFWwindow* getNativeWindow() const { return m_window; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    GLFWwindow* m_window;
    int m_width, m_height;
    int m_windowX, m_windowY;
    int m_windowWidth, m_windowHeight;
    bool m_fullscreen = false;
};

} // namespace klab
