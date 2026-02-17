#include "tools/inspectable.hpp"

#ifdef BEE_INSPECTOR

namespace bee
{
template <>
std::vector<IToolbar*> EditorBase<IToolbar>::m_editors = {};
template <>
std::vector<IEntityInspector*> EditorBase<IEntityInspector>::m_editors = {};
template <>
std::vector<IPanel*> EditorBase<IPanel>::m_editors = {};
template <>
std::vector<IStatsBar*> EditorBase<IStatsBar>::m_editors = {};

IToolbar::IToolbar() { m_editors.push_back(this); }
IEntityInspector::IEntityInspector() { m_editors.push_back(this); }
IPanel::IPanel() { m_editors.push_back(this); }
IStatsBar::IStatsBar() { m_editors.push_back(this); }

}  // namespace bee

#endif