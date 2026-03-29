#include "Input.hpp"
#include <iostream>

namespace klab {

GLFWwindow* Input::m_window = nullptr;
double Input::m_lastX = 0, Input::m_lastY = 0;
double Input::m_offsetX = 0, Input::m_offsetY = 0;
bool Input::m_firstMouse = true;
bool Input::m_cursorVisible = true;
double Input::m_lastSampleTime = 0;
float Input::m_avgSampleInterval = 0;

void Input::init(GLFWwindow* window) {
    m_window = window;
    glfwSetCursorPosCallback(window, mouseCallback);
    
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    m_lastSampleTime = glfwGetTime();
}

void Input::update() {
}

bool Input::isKeyPressed(int key) {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(int button) {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Input::getMouseOffset(double& x, double& y) {
    x = m_offsetX;
    y = m_offsetY;
    m_offsetX = 0;
    m_offsetY = 0;
}

float Input::getMousePollingRate() {
    return m_avgSampleInterval > 0 ? 1.0f / m_avgSampleInterval : 0;
}

float Input::getMouseLatency() {
    return m_avgSampleInterval * 1000.0f; // Average interval in ms
}

float Input::getTimeSinceLastSample() {
    return (float)(glfwGetTime() - m_lastSampleTime) * 1000.0f;
}

void Input::setCursorVisible(bool visible) {
    if (m_cursorVisible == visible) return;
    
    m_cursorVisible = visible;
    if (visible) {
        // Welcoming the cursor back from the shadow realm
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        int w, h;
        glfwGetWindowSize(m_window, &w, &h);
        glfwSetCursorPos(m_window, w / 2.0, h / 2.0);
    } else {
        // Banish the cursor to the shadow realm (-10000, -10000)
        // If it tries to escape, GLFW_CURSOR_DISABLED will break its legs
        glfwSetCursorPos(m_window, -10000.0, -10000.0);
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_firstMouse = true;
    }
}

bool Input::isCursorVisible() {
    return m_cursorVisible;
}

void Input::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    double currentTime = glfwGetTime();
    double interval = currentTime - m_lastSampleTime;
    m_lastSampleTime = currentTime;

    // Simple moving average for stability
    if (interval > 0 && interval < 0.1) { // Ignore spikes
        m_avgSampleInterval = m_avgSampleInterval * 0.95f + (float)interval * 0.05f;
    }

    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
        return;
    }

    if (!m_cursorVisible) {
        m_offsetX += xpos - m_lastX;
        m_offsetY += m_lastY - ypos;
    }

    m_lastX = xpos;
    m_lastY = ypos;
}

} // namespace klab
