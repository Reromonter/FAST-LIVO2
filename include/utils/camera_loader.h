/*
 * camera_loader.h — ROS2 port of vikit_ros' camera loader.
 *
 * Reads the camera model from the node's parameters (prefix "camera.")
 * instead of the ROS1 parameter server namespace.
 */

#ifndef FAST_LIVO_CAMERA_LOADER_H_
#define FAST_LIVO_CAMERA_LOADER_H_

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <vikit/abstract_camera.h>
#include <vikit/atan_camera.h>
#include <vikit/equidistant_camera.h>
#include <vikit/omni_camera.h>
#include <vikit/pinhole_camera.h>
#include <vikit/polynomial_camera.h>

namespace vk {
namespace camera_loader {

/// Load a camera model from the node's "camera.*" parameters.
inline bool loadFromNode(rclcpp::Node &node, vk::AbstractCamera *&cam)
{
  auto dparam = [&node](const std::string &name, double def) {
    return node.declare_parameter<double>("camera." + name, def);
  };
  auto iparam = [&node](const std::string &name, int def) {
    return node.declare_parameter<int>("camera." + name, def);
  };

  std::string cam_model = node.declare_parameter<std::string>("camera.cam_model", "Pinhole");
  bool res = true;
  if (cam_model == "Ocam")
  {
    cam = new vk::OmniCamera(node.declare_parameter<std::string>("camera.cam_calib_file", ""));
  }
  else if (cam_model == "Pinhole")
  {
    cam = new vk::PinholeCamera(
        iparam("cam_width", 0), iparam("cam_height", 0), dparam("scale", 1.0),
        dparam("cam_fx", 0.0), dparam("cam_fy", 0.0),
        dparam("cam_cx", 0.0), dparam("cam_cy", 0.0),
        dparam("cam_d0", 0.0), dparam("cam_d1", 0.0),
        dparam("cam_d2", 0.0), dparam("cam_d3", 0.0));
  }
  else if (cam_model == "EquidistantCamera")
  {
    cam = new vk::EquidistantCamera(
        iparam("cam_width", 0), iparam("cam_height", 0), dparam("scale", 1.0),
        dparam("cam_fx", 0.0), dparam("cam_fy", 0.0),
        dparam("cam_cx", 0.0), dparam("cam_cy", 0.0),
        dparam("k1", 0.0), dparam("k2", 0.0), dparam("k3", 0.0), dparam("k4", 0.0));
  }
  else if (cam_model == "PolynomialCamera")
  {
    cam = new vk::PolynomialCamera(
        iparam("cam_width", 0), iparam("cam_height", 0),
        dparam("cam_fx", 0.0), dparam("cam_fy", 0.0),
        dparam("cam_cx", 0.0), dparam("cam_cy", 0.0),
        dparam("cam_skew", 0.0),
        dparam("k2", 0.0), dparam("k3", 0.0), dparam("k4", 0.0),
        dparam("k5", 0.0), dparam("k6", 0.0), dparam("k7", 0.0));
  }
  else if (cam_model == "ATAN")
  {
    cam = new vk::ATANCamera(
        iparam("cam_width", 0), iparam("cam_height", 0),
        dparam("cam_fx", 0.0), dparam("cam_fy", 0.0),
        dparam("cam_cx", 0.0), dparam("cam_cy", 0.0), dparam("cam_d0", 0.0));
  }
  else
  {
    cam = nullptr;
    res = false;
  }
  return res;
}

} // namespace camera_loader
} // namespace vk

#endif // FAST_LIVO_CAMERA_LOADER_H_
