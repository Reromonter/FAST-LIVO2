/*
 * ROS2 compatibility helpers for the FAST-LIVO2 ROS2 port.
 *
 * Provides the small time/logging conveniences the original ROS1 code relied
 * on (header.stamp.toSec(), ros::Time().fromSec(), ROS_* logging macros).
 */

#ifndef FAST_LIVO_ROS2_COMPAT_H_
#define FAST_LIVO_ROS2_COMPAT_H_

#include <builtin_interfaces/msg/time.hpp>
#include <rclcpp/rclcpp.hpp>
#include <cassert>

inline double stamp2sec(const builtin_interfaces::msg::Time &t)
{
  return static_cast<double>(t.sec) + static_cast<double>(t.nanosec) * 1e-9;
}

inline builtin_interfaces::msg::Time sec2stamp(double sec)
{
  builtin_interfaces::msg::Time t;
  t.sec = static_cast<int32_t>(sec);
  t.nanosec = static_cast<uint32_t>((sec - t.sec) * 1e9);
  return t;
}

#include <geometry_msgs/msg/quaternion.hpp>
#include <cmath>

/// replaces tf::createQuaternionMsgFromRollPitchYaw
inline geometry_msgs::msg::Quaternion rpy2quat(double roll, double pitch, double yaw)
{
  const double cy = std::cos(yaw * 0.5), sy = std::sin(yaw * 0.5);
  const double cp = std::cos(pitch * 0.5), sp = std::sin(pitch * 0.5);
  const double cr = std::cos(roll * 0.5), sr = std::sin(roll * 0.5);
  geometry_msgs::msg::Quaternion q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;
  return q;
}

#define FAST_LIVO_LOGGER rclcpp::get_logger("fast_livo")
#define ROS_INFO(...) RCLCPP_INFO(FAST_LIVO_LOGGER, __VA_ARGS__)
#define ROS_WARN(...) RCLCPP_WARN(FAST_LIVO_LOGGER, __VA_ARGS__)
#define ROS_ERROR(...) RCLCPP_ERROR(FAST_LIVO_LOGGER, __VA_ARGS__)
#define ROS_ASSERT(cond) assert(cond)

#endif // FAST_LIVO_ROS2_COMPAT_H_
