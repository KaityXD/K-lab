#include "Crosshair.hpp"
#include <cmath>
#include <algorithm>

namespace klab {

void drawCrosshair(const Window& window, const CrosshairSettings& settings) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    float centerX = std::floor(window.getWidth() / 2.0f);
    float centerY = std::floor(window.getHeight() / 2.0f);
    ImVec2 center(centerX, centerY);

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(settings.color[0], settings.color[1], settings.color[2], settings.color[3]));
    ImU32 outCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings.outlineColor[0], settings.outlineColor[1], settings.outlineColor[2], settings.outlineColor[3]));
    
    float thick = settings.thickness;
    float halfThick = thick / 2.0f;
    float out = settings.outlineThickness;

    auto drawRectWithOutline = [&](float x1, float y1, float x2, float y2) {
        if (settings.outline) {
            drawList->AddRectFilled(ImVec2(x1 - out, y1 - out), ImVec2(x2 + out, y2 + out), outCol);
        }
        drawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col);
    };

    if (settings.circle) {
        if (settings.outline) {
            drawList->AddCircle(center, settings.circleRadius, outCol, 0, thick + out * 2.0f);
        }
        drawList->AddCircle(center, settings.circleRadius, col, 0, thick);
    }

    if (settings.dot) {
        float r = settings.dotSize;
        if (settings.outline) {
            drawList->AddCircleFilled(center, r + out, outCol);
        }
        drawList->AddCircleFilled(center, r, col);
    }

    bool drawTop = settings.drawTop && !settings.tShape;
    bool drawRight = settings.drawRight;
    bool drawBottom = settings.drawBottom;
    bool drawLeft = settings.drawLeft;

    if (drawTop) {
        drawRectWithOutline(centerX - halfThick, centerY - settings.gap - settings.length, 
                           centerX + halfThick, centerY - settings.gap);
    }
    if (drawRight) {
        drawRectWithOutline(centerX + settings.gap, centerY - halfThick, 
                           centerX + settings.gap + settings.length, centerY + halfThick);
    }
    if (drawBottom) {
        drawRectWithOutline(centerX - halfThick, centerY + settings.gap, 
                           centerX + halfThick, centerY + settings.gap + settings.length);
    }
    if (drawLeft) {
        drawRectWithOutline(centerX - settings.gap - settings.length, centerY - halfThick, 
                           centerX - settings.gap, centerY + halfThick);
    }
}

} // namespace klab
