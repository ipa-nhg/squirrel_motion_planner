#include <squirrel_8dof_visualizer/squirrel_8dof_visualizer.h>

namespace SquirrelMotionPlanning
{

Visualizer::Visualizer() :
    nhPrivate("~"), rate(30), robotMarkerPublisher("robot_trajectory_visualization"), jointStatesArm(new sensor_msgs::JointState)
{
  subscriberTrajectory = nh.subscribe("squirrel_8dof_planner_node/robot_trajectory", 1, &Visualizer::subscriberTrajectoryHandler, this);
  subscriberVisibility = nhPrivate.subscribe("set_visibility", 1, &Visualizer::subscriberVisibilityHandler, this);
  subscriberRate = nhPrivate.subscribe("set_rate", 1, &Visualizer::subscriberRateHandler, this);
  robotID = robotMarkerPublisher.addRobotFromDescription("robot_description_vis");
  std_msgs::ColorRGBA color;
  color.a = 0.2;
  color.b = 1.0;
  robotMarkerPublisher.setRobotColor(robotID, color, rviz_robot_marker::RvizRobotMarkerPublisher::FORCE_COLOR);

  transformBase.frame_id_ = "origin";
  transformBase.child_frame_id_ = "base_link";
  transformBase.setIdentity();

  transformBaseTranslation.setZero();
  transformBaseRotation.setIdentity();

  jointStatesArm->position.resize(5, 0.0);
  jointStatesArm->name.resize(5);
  jointStatesArm->name[0] = "arm_joint1";
  jointStatesArm->name[1] = "arm_joint2";
  jointStatesArm->name[2] = "arm_joint3";
  jointStatesArm->name[3] = "arm_joint4";
  jointStatesArm->name[4] = "arm_joint5";

  poseCurrent = 0;
  finalPoseState = false;
  visible = true;
}

void Visualizer::run()
{
  while (ros::ok())
  {
    rate.sleep();
    ros::spinOnce();

    if (!visible || poses.size() <= 1)
      continue;

    transformBaseTranslation[0] = poses[poseCurrent][0];
    transformBaseTranslation[1] = poses[poseCurrent][1];
    transformBaseRotation.setRPY(0.0, 0.0, poses[poseCurrent][2]);

    transformBase.setOrigin(transformBaseTranslation);
    transformBase.setBasis(transformBaseRotation);

    jointStatesArm->position[0] = poses[poseCurrent][3];
    jointStatesArm->position[1] = poses[poseCurrent][4];
    jointStatesArm->position[2] = poses[poseCurrent][5];
    jointStatesArm->position[3] = poses[poseCurrent][6];
    jointStatesArm->position[4] = poses[poseCurrent][7];

    robotMarkerPublisher.setRobotPose(robotID, transformBase, jointStatesArm);

    if (poseCurrent == poses.size() - 1)
    {
      if (!finalPoseState)
      {
        finalPoseState = true;
        finalPoseTime = ros::WallTime::now();
      }
      else if (ros::WallTime::now() - finalPoseTime >= ros::WallDuration(2.0))
      {
        finalPoseState = false;
        poseCurrent = 0;
      }
    }
    else
      ++poseCurrent;
  }
}

void Visualizer::subscriberTrajectoryHandler(const std_msgs::Float64MultiArray &msg)
{
  if (msg.layout.dim[1].size != 8)
  {
    ROS_WARN("Wrong robot trajectory dimension. Expected '8', but received '%d'.", msg.layout.dim[1].size);
    return;
  }
  else if (msg.layout.dim[0].size * msg.layout.dim[1].size != msg.data.size())
  {
    ROS_WARN("Specified layout does not match dimension of data.");
    return;
  }

  poses.resize(msg.layout.dim[0].size, std::vector<Real>(8));

  UInt counter = 0;
  for (UInt i = 0; i < msg.layout.dim[0].size; ++i)
    for (UInt j = 0; j < 8; ++j)
    {
      poses[i][j] = msg.data[counter];
      ++counter;
    }

  poseCurrent = 0;
  finalPoseState = false;
}

void Visualizer::subscriberRateHandler(const std_msgs::Float64 &msg)
{
  rate = ros::Rate(msg.data);
}

void Visualizer::subscriberVisibilityHandler(const std_msgs::Bool &msg)
{
  visible = msg.data;
  if (visible)
    robotMarkerPublisher.showRobot(robotID);
  else
    robotMarkerPublisher.hideRobot(robotID);
}

} //namespace SquirrelMotionPlanning
