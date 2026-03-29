#pragma once
#include <GLFW/glfw3.h>

namespace klab {

class Input {
public:
    static void init(GLFWwindow* window);
    static void update();

    static bool isKeyPressed(int key);
    static bool isMouseButtonPressed(int button);
    static void getMouseOffset(double& x, double& y);
    static float getMousePollingRate();
    static float getMouseLatency(); // Average time between samples in ms
    static float getTimeSinceLastSample(); // How long ago we got the last packet in ms
    
    static void setCursorVisible(bool visible);
    static bool isCursorVisible();

private:
    static GLFWwindow* m_window;
    static double m_lastX, m_lastY;
    static double m_offsetX, m_offsetY;
    static bool m_firstMouse;
    static bool m_cursorVisible;

    static double m_lastSampleTime;
    static float m_avgSampleInterval;

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
};

} // namespace klab
