cmake_minimum_required(VERSION 2.8.3)
project(planner_param_config)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)

catkin_package(
   CATKIN_DEPENDS std_msgs
)

ADD_DEFINITIONS(
    -std=c++11
    -D__cplusplus=201103L #for eclipse
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

