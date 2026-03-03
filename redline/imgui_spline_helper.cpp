#include "imgui_spline_helper.hpp"

SplineCanvas::SplineCanvas(float width, float height)
    : m_canvasSize(width, height)
{
}

void SplineCanvas::Begin()
{
    m_drawList = ImGui::GetWindowDrawList();
    m_canvasPos = ImGui::GetCursorScreenPos();

    // Draw background
    m_drawList->AddRectFilled(
        m_canvasPos,
        ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y),
        m_backgroundColor);

    // Check if hovered
    ImVec2 mousePos = ImGui::GetMousePos();
    m_isHovered = mousePos.x >= m_canvasPos.x && mousePos.x <= m_canvasPos.x + m_canvasSize.x &&
                  mousePos.y >= m_canvasPos.y && mousePos.y <= m_canvasPos.y + m_canvasSize.y;
}

void SplineCanvas::End()
{
    // Draw border
    m_drawList->AddRect(
        m_canvasPos,
        ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y),
        IM_COL32(80, 80, 80, 255));

    // Reserve space in ImGui layout
    ImGui::Dummy(m_canvasSize);
}

ImVec2 SplineCanvas::ToScreen(const glm::vec2& localPos) const
{
    return ImVec2(m_canvasPos.x + localPos.x, m_canvasPos.y + localPos.y);
}

glm::vec2 SplineCanvas::GetMousePos() const
{
    ImVec2 mousePos = ImGui::GetMousePos();
    return glm::vec2(mousePos.x - m_canvasPos.x, mousePos.y - m_canvasPos.y);
}

void SplineCanvas::DrawDot(const glm::vec2& pos, float radius, ImU32 color)
{
    m_drawList->AddCircleFilled(ToScreen(pos), radius, color);
}

void SplineCanvas::DrawDot(const glm::vec2& pos, float radius, ImU32 fillColor, ImU32 borderColor, float borderThickness)
{
    ImVec2 screenPos = ToScreen(pos);
    m_drawList->AddCircleFilled(screenPos, radius, fillColor);
    m_drawList->AddCircle(screenPos, radius, borderColor, 0, borderThickness);
}

void SplineCanvas::DrawLine(const glm::vec2& p0, const glm::vec2& p1, ImU32 color, float thickness)
{
    m_drawList->AddLine(ToScreen(p0), ToScreen(p1), color, thickness);
}

void SplineCanvas::DrawBezier(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                               ImU32 color, float thickness, int segments)
{
    m_drawList->AddBezierCubic(
        ToScreen(p0), ToScreen(p1), ToScreen(p2), ToScreen(p3),
        color, thickness, segments);
}

void SplineCanvas::DrawPolyline(const std::vector<glm::vec2>& points, ImU32 color, float thickness, bool closed)
{
    if (points.size() < 2) return;

    std::vector<ImVec2> screenPoints;
    screenPoints.reserve(points.size());
    for (const auto& p : points)
    {
        screenPoints.push_back(ToScreen(p));
    }

    m_drawList->AddPolyline(
        screenPoints.data(),
        static_cast<int>(screenPoints.size()),
        color,
        closed ? ImDrawFlags_Closed : ImDrawFlags_None,
        thickness);
}

void SplineCanvas::DrawSpline(const std::vector<glm::vec2>& points, ImU32 color, float thickness, int segmentsPerCurve)
{
    if (points.size() < 2) return;

    if (points.size() == 2)
    {
        DrawLine(points[0], points[1], color, thickness);
        return;
    }

    // Catmull-Rom spline interpolation
    std::vector<ImVec2> splinePoints;

    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        // Get 4 control points for catmull-rom
        glm::vec2 p0 = (i == 0) ? points[0] : points[i - 1];
        glm::vec2 p1 = points[i];
        glm::vec2 p2 = points[i + 1];
        glm::vec2 p3 = (i + 2 >= points.size()) ? points[points.size() - 1] : points[i + 2];

        // Generate points along this segment
        for (int j = 0; j < segmentsPerCurve; ++j)
        {
            float t = static_cast<float>(j) / static_cast<float>(segmentsPerCurve);
            float t2 = t * t;
            float t3 = t2 * t;

            // Catmull-Rom basis functions
            glm::vec2 point = 0.5f * (
                (2.0f * p1) +
                (-p0 + p2) * t +
                (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
            );

            splinePoints.push_back(ToScreen(point));
        }
    }

    // Add final point
    splinePoints.push_back(ToScreen(points.back()));

    if (splinePoints.size() >= 2)
    {
        m_drawList->AddPolyline(
            splinePoints.data(),
            static_cast<int>(splinePoints.size()),
            color,
            ImDrawFlags_None,
            thickness);
    }
}

void SplineCanvas::DrawGrid(glm::ivec2 divisions, ImU32 color)
{
    float stepX = m_canvasSize.x / static_cast<float>(divisions.x);
    float stepY = m_canvasSize.y / static_cast<float>(divisions.y);

    // Vertical lines
    for (int i = 1; i < divisions.x; ++i)
    {
        float x = static_cast<float>(i) * stepX;
        DrawLine(glm::vec2(x, 0), glm::vec2(x, m_canvasSize.y), color, 1.0f);
    }

    // Horizontal lines
    for (int i = 1; i < divisions.y; ++i)
    {
        float y = static_cast<float>(i) * stepY;
        DrawLine(glm::vec2(0, y), glm::vec2(m_canvasSize.x, y), color, 1.0f);
    }
}

void SplineCanvas::DrawLabeledGrid(glm::ivec2 divisions, glm::vec2 xRange, glm::vec2 yRange,
                                    ImU32 gridColor, ImU32 textColor)
{
    float stepX = m_canvasSize.x / static_cast<float>(divisions.x);
    float stepY = m_canvasSize.y / static_cast<float>(divisions.y);

    char buf[32];

    // Vertical lines + X axis labels (speed)
    for (int i = 0; i <= divisions.x; ++i)
    {
        float x = static_cast<float>(i) * stepX;
        if (i > 0 && i < divisions.x)
            DrawLine(glm::vec2(x, 0), glm::vec2(x, m_canvasSize.y), gridColor, 1.0f);

        float value = xRange.x + (xRange.y - xRange.x) * (static_cast<float>(i) / static_cast<float>(divisions.x));
        snprintf(buf, sizeof(buf), "%.0f", value);
        ImVec2 textSize = ImGui::CalcTextSize(buf);
        ImVec2 pos = ToScreen(glm::vec2(x - textSize.x * 0.5f, m_canvasSize.y + 2.0f));
        m_drawList->AddText(pos, textColor, buf);
    }

    // Horizontal lines + Y axis labels (force), skip bottom (i == divisions.y) to avoid duplicate zero
    for (int i = 0; i < divisions.y; ++i)
    {
        float y = static_cast<float>(i) * stepY;
        if (i > 0)
            DrawLine(glm::vec2(0, y), glm::vec2(m_canvasSize.x, y), gridColor, 1.0f);

        // Y is flipped: top of canvas = max value, bottom = min
        float t = 1.0f - (static_cast<float>(i) / static_cast<float>(divisions.y));
        float value = yRange.x + (yRange.y - yRange.x) * t;
        snprintf(buf, sizeof(buf), "%.0f", value);
        ImVec2 textSize = ImGui::CalcTextSize(buf);
        ImVec2 pos = ToScreen(glm::vec2(-textSize.x - 4.0f, y - textSize.y * 0.5f));
        m_drawList->AddText(pos, textColor, buf);
    }
}
