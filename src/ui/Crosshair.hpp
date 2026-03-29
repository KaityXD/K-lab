#pragma once

#include <imgui.h>
#include "../engine/Window.hpp"

namespace klab {

struct CrosshairSettings {
    float thickness = 2.0f;
    float length = 10.0f;
    float gap = 5.0f;
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool dot = false;
    float dotSize = 2.0f;
    bool circle = false;
    float circleRadius = 10.0f;
    bool tShape = false;
    bool drawTop = true;
    bool drawRight = true;
    bool drawBottom = true;
    bool drawLeft = true;
    bool outline = true;
    float outlineThickness = 1.0f;
    float outlineColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};

void drawCrosshair(const Window& window, const CrosshairSettings& settings);

} // namespace klab
