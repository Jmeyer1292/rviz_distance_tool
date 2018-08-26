#ifndef RVIZ_DISTANCE_TOOL_H
#define RVIZ_DISTANCE_TOOL_H

#include <rviz/tool.h>
#include <OgreVector3.h>

namespace rviz
{
class Line;
}

namespace rviz_distance_tool
{

class DistanceTool : public rviz::Tool
{
  Q_OBJECT
public:
  DistanceTool();
  virtual ~DistanceTool();

  virtual void onInitialize() override;

  virtual void activate() override;

  virtual void deactivate() override;

  virtual int processMouseEvent(rviz::ViewportMouseEvent& event) override;

private:

  void configureLines(const Ogre::Vector3& start, const Ogre::Vector3& end);
  void configureStatus(const Ogre::Vector3& start, const Ogre::Vector3& end);
  void hideLines();

  // state machine
  enum class SelectionState {
    Idle, Tracking, Finished
  } state_;

  std::array<rviz::Line*, 4> lines_;
  Ogre::Vector3 start_;
  Ogre::Vector3 end_;

  QCursor std_cursor_;
  QCursor hit_cursor_;

};

}


#endif
