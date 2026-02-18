#pragma once

#include <vector>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

class SplineCanvas
{
public:
    SplineCanvas(float width = 200.0f, float height = 200.0f);

    // Call at start of drawing
    void Begin();

    // Call at end of drawing
    void End();

    // Draw a filled dot
    void DrawDot(const glm::vec2& pos, float radius = 6.0f, ImU32 color = IM_COL32(255, 255, 255, 255));
    void DrawDot(const glm::vec2& pos, float radius, ImU32 fillColor, ImU32 borderColor, float borderThickness = 1.0f);

    // Draw a cubic bezier curve
    void DrawBezier(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                    ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 2.0f, int segments = 32);

    // Draw a polyline through points
    void DrawPolyline(const std::vector<glm::vec2>& points, ImU32 color = IM_COL32(255, 255, 255, 255),
                      float thickness = 2.0f, bool closed = false);

    // Draw a smooth spline through points (catmull-rom style)
    void DrawSpline(const std::vector<glm::vec2>& points, ImU32 color = IM_COL32(255, 255, 255, 255),
                    float thickness = 2.0f, int segmentsPerCurve = 16);

    // Draw a line
    void DrawLine(const glm::vec2& p0, const glm::vec2& p1, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);

    // Draw background grid
    void DrawGrid(glm::ivec2 divisions = {4, 4}, ImU32 color = IM_COL32(100, 100, 100, 255));

    // Draw labeled grid with axis values
    // xRange/yRange: the data range each axis represents (e.g. 0-200 for speed)
    // yFlipped: if true, y labels go from max at top to 0 at bottom
    void DrawLabeledGrid(glm::ivec2 divisions, glm::vec2 xRange, glm::vec2 yRange,
                         ImU32 gridColor = IM_COL32(45, 48, 55, 255),
                         ImU32 textColor = IM_COL32(160, 160, 170, 255));

    // Set background color (call before Begin or use in Begin)
    void SetBackgroundColor(ImU32 color) { m_backgroundColor = color; }

    // Getters
    glm::vec2 GetCanvasPos() const { return glm::vec2(m_canvasPos.x, m_canvasPos.y); }
    glm::vec2 GetCanvasSize() const { return glm::vec2(m_canvasSize.x, m_canvasSize.y); }

    // Check if mouse is hovering canvas
    bool IsHovered() const { return m_isHovered; }

    // Get mouse position relative to canvas (0,0 = top-left of canvas)
    glm::vec2 GetMousePos() const;

    // Convert local position to screen position
    ImVec2 ToScreen(const glm::vec2& localPos) const;

private:
    ImVec2 m_canvasPos;
    ImVec2 m_canvasSize;
    ImDrawList* m_drawList = nullptr;
    ImU32 m_backgroundColor = IM_COL32(40, 40, 40, 255);
    bool m_isHovered = false;
};
