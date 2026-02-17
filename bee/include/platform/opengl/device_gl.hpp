#pragma once

struct GLFWwindow;
struct GLFWmonitor;

// We cannot make some member functions static due to different implementations per platform. Ignore this linter warning.
// NOLINTBEGIN(readability-convert-member-functions-to-static)
namespace bee
{
class Device
{
public:
    bool CanClose() { return true; }
    void RequestClose();
    bool ShouldClose();
    GLFWwindow* GetWindow();
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
    void SetWindowSize(int width, int height);
    void BeginFrame() {}
    void EndFrame() {}
    float GetMonitorUIScale() const;

private:
    friend class EngineClass;
    Device();
    ~Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

    void Update();

    GLFWwindow* m_window = nullptr;
    GLFWmonitor* m_monitor = nullptr;
    bool m_vsync = true;
    bool m_fullscreen = false;
    int m_width = -1;
    int m_height = -1;
};
}  // namespace bee

// NOLINTEND(readability-convert-member-functions-to-static)