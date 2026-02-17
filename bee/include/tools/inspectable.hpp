#pragma once
#include <vector>
#include <string>
#include "core/ecs.hpp"

#ifdef BEE_INSPECTOR

namespace bee
{

template <typename T>
class EditorBase
{
public:
    EditorBase() = default;
    virtual ~EditorBase() { m_editors.erase(std::find(m_editors.begin(), m_editors.end(), this)); }
    EditorBase(const EditorBase&) = delete;
    EditorBase& operator=(const EditorBase&) = delete;
    EditorBase(EditorBase&&) = delete;
    EditorBase& operator=(EditorBase&&) = delete;

protected:
    static std::vector<T*> m_editors;
    friend class Inspector;
};

/// <summary>
/// Inherit from this if you want your class to have ImGui behavior in the main Bee toolbar.
/// </summary>
class IToolbar : public EditorBase<IToolbar>
{
public:
    IToolbar();
    virtual void OnToolbar() = 0;
};
template <>
std::vector<IToolbar*> EditorBase<IToolbar>::m_editors;

/// <summary>
/// Inherit from this if you want your class to display (e.g. component) data of the selected Bee ECS entity in the Scene panel.
/// </summary>
class IEntityInspector : public EditorBase<IEntityInspector>
{
public:
    IEntityInspector();
    virtual void OnEntity(entt::entity entity) = 0;
};
template <>
std::vector<IEntityInspector*> EditorBase<IEntityInspector>::m_editors;

/// <summary>
/// Inherit from this if you want your class to have a full panel in the Bee inspector, next to the Scene panel.
/// For example, the Bee Profiler uses this to display profiling output.
/// </summary>
class IPanel : public EditorBase<IPanel>
{
public:
    IPanel();
    virtual void OnPanel() = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetIcon() const = 0;
};
template <>
std::vector<IPanel*> EditorBase<IPanel>::m_editors;

/// <summary>
/// Inherit from this if you want your class to display data in the main Bee stats bar.
/// </summary>
class IStatsBar : public EditorBase<IStatsBar>
{
public:
    IStatsBar();
    virtual void OnStatsBar() = 0;
};
template <>
std::vector<IStatsBar*> EditorBase<IStatsBar>::m_editors;

}  // namespace bee

#else

namespace bee
{

class IToolbar
{
};
class IEntityInspector
{
};
class IPanel
{
};
class IStatsBar
{
};

}  // namespace bee

#endif
