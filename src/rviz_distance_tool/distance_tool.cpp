#include <rviz_distance_tool/distance_tool.h>
#include <pluginlib/class_list_macros.h>

#pragma GCC diagnostic push   // Supress warnings from library files
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <rviz/display_context.h>
#include <rviz/load_resource.h>
#include <rviz/ogre_helpers/line.h>
#include <rviz/selection/selection_manager.h>
#include <rviz/viewport_mouse_event.h>

#include <OgreSceneNode.h>
#pragma GCC diagnostic pop // Un-supress warnings


rviz_distance_tool::DistanceTool::DistanceTool()
  : state_(SelectionState::Idle)
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

  shortcut_key_ = 'd';
}

void rviz_distance_tool::DistanceTool::activate()
{}

void rviz_distance_tool::DistanceTool::deactivate()
{}

int rviz_distance_tool::DistanceTool::processMouseEvent(rviz::ViewportMouseEvent& event)
{
  int flags = 0;

  Ogre::Vector3 pos;
  const bool success = context_->getSelectionManager()->get3DPoint(event.viewport, event.x, event.y, pos);
  setCursor(success ? hit_cursor_ : std_cursor_);

  switch (state_)
  {
  case SelectionState::Idle:
    if (success && event.leftDown()) // Transition from idle if user clicked on something
    {
      start_ = pos;
      state_ = SelectionState::Tracking;
    }
    break;

  case SelectionState::Tracking:
    if (success)
    {
      configureLines(start_, pos);
      configureStatus(start_, pos);

      if (event.leftDown()) // Transition to finished if user clicked on another thing
      {
        end_ = pos;
        state_ = SelectionState::Finished;
      }
    }
    flags |= Render;
    break;

  case SelectionState::Finished: // Stay here so you can move the camera and come back for your measurement
    configureStatus(start_, end_);

    if (event.leftDown()) // User might start a new selection in this state
    {
      start_ = pos;
      state_ = SelectionState::Tracking;
    }
    flags |= Render;
    break;
  }

  // Unconditionally, the right click brings you back to idle
  if (event.rightUp())
  {
    state_ = SelectionState::Idle;
    start_ = end_ = Ogre::Vector3::ZERO;
    hideLines();
  }

  return flags;
}

void rviz_distance_tool::DistanceTool::configureLines(const Ogre::Vector3 &start, const Ogre::Vector3 &end)
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
}

void rviz_distance_tool::DistanceTool::configureStatus(const Ogre::Vector3 &start, const Ogre::Vector3 &end)
{
  // Write the string for the bottom status bar
  const Ogre::Vector3 delta = end - start;
  std::ostringstream oss;
  oss << "Distance = " << delta.length() << " [x,y,z = " << delta.x << ", " << delta.y << ", " << delta.z << "]";
  setStatus(QString::fromStdString(oss.str()));
}

void rviz_distance_tool::DistanceTool::hideLines()
{
  for (auto& line : lines_) line->setVisible(false);
}

PLUGINLIB_EXPORT_CLASS(rviz_distance_tool::DistanceTool, rviz::Tool)
