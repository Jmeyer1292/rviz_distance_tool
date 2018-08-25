#include <rviz_distance_tool/distance_tool.h>
#include <pluginlib/class_list_macros.h>

#pragma GCC diagnostic push   // Supress warnings from library files
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <rviz/viewport_mouse_event.h>
#include <rviz/display_context.h>
#include <rviz/selection/selection_manager.h>
#include <rviz/ogre_helpers/line.h>
#include <rviz/load_resource.h>

#include <OgreSceneNode.h>
#pragma GCC diagnostic pop // Un-supress warnings


rviz_distance_tool::DistanceTool::DistanceTool()
  : state_(SelectionState::START)
  , lines_{{nullptr, nullptr, nullptr, nullptr}}
{}

rviz_distance_tool::DistanceTool::~DistanceTool()
{
  for (auto& line : lines_) delete line;
}

void rviz_distance_tool::DistanceTool::onInitialize()
{
  for (auto& line : lines_)
    line = new rviz::Line(context_->getSceneManager());

  lines_[0]->setColor(1.0f, 1.0f, 1.0f, 1.0f); // aggregate
  lines_[1]->setColor(1.0f, 0.0f, 0.0f, 1.0f); // x trans
  lines_[2]->setColor(0.0f, 1.0f, 0.0f, 1.0f); // y trans
  lines_[3]->setColor(0.0f, 0.0f, 1.0f, 1.0f); // z trans

  std_cursor_ = rviz::getDefaultCursor();
  hit_cursor_ = rviz::makeIconCursor("package://rviz/icons/crosshair.svg");
}

void rviz_distance_tool::DistanceTool::activate()
{
  state_ = SelectionState::START;
}

void rviz_distance_tool::DistanceTool::deactivate()
{}

int rviz_distance_tool::DistanceTool::processMouseEvent(rviz::ViewportMouseEvent& event)
{
  Ogre::Vector3 pos;
  const bool success = context_->getSelectionManager()->get3DPoint(event.viewport, event.x, event.y, pos);
  setCursor(success ? hit_cursor_ : std_cursor_);

  int flags = 0;

  switch (state_)
  {
  case SelectionState::START:
//    if (!success) setLinesAndStatus(start_, end_);
    break;
  case SelectionState::END:
    if (success) setLinesAndStatus(start_, pos);
    break;
  }

  if (event.leftUp() && success)
  {
    switch (state_)
    {
    case SelectionState::START:
      start_ = pos;
      state_ = SelectionState::END;
      break;
    case SelectionState::END:
      end_ = pos;
      state_ = SelectionState::START;
      setLinesAndStatus(start_, end_);
      break;
    }

    flags |= Render;
  }


  if (event.rightUp())
  {
    state_ = SelectionState::START;
    hideLines();
  }

  return flags;
}

void rviz_distance_tool::DistanceTool::setLinesAndStatus(const Ogre::Vector3 &start, const Ogre::Vector3 &end)
{
  const Ogre::Vector3 delta = end - start;

  // Draw first line from start to end
  lines_[0]->setPoints(start, end);

  // Draw line from start to +X
  Ogre::Vector3 start_pt = start;
  Ogre::Vector3 end_pt = start_pt + Ogre::Vector3(delta.x, 0.0, 0.0);
  lines_[1]->setPoints(start_pt, end_pt);

  // From +X to +X, +Y
  start_pt = end_pt;
  end_pt = end_pt + Ogre::Vector3(0, delta.y, 0);
  lines_[2]->setPoints(start_pt, end_pt);

  // From +X, +Y to end
  start_pt = end_pt;
  end_pt = end_pt + Ogre::Vector3(0, 0, delta.z);
  lines_[3]->setPoints(start_pt, end_pt);

  // Write the string for the bottom status bar
  std::ostringstream oss;
  oss << "Distance = " << delta.length() << " [x,y,z = " << delta.x << ", " << delta.y << ", " << delta.z << "]";
  setStatus(QString::fromStdString(oss.str()));
}

void rviz_distance_tool::DistanceTool::hideLines()
{
  for (auto& line : lines_) line->setVisible(false);
}

PLUGINLIB_EXPORT_CLASS(rviz_distance_tool::DistanceTool, rviz::Tool)
