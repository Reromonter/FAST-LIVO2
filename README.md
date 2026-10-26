# FAST-LIVO2 (ROS 2 port)

## FAST-LIVO2: Fast, Direct LiDAR-Inertial-Visual Odometry

**This fork is a ROS 2 (Humble) port.** The estimator itself is untouched; everything that talked to ROS 1 was
rewritten, and the two external catkin dependencies (Sophus, `rpg_vikit`) are now vendored under `thirdparty/`. See
[Changes in this fork](#6-changes-in-this-fork) for the full list. If you are looking for the original catkin/ROS 1
version, use [upstream](https://github.com/hku-mars/FAST-LIVO2).

### 📢 News

- 🔓 **2025-01-23**: Code released!  
- 🎉 **2024-10-01**: Accepted by **T-RO '24**!  
- 🚀 **2024-07-02**: Conditionally accepted.

### 📬 Contact

For further inquiries or assistance, please contact [zhengcr@connect.hku.hk](mailto:zhengcr@connect.hku.hk).

## 1. Introduction

FAST-LIVO2 is an efficient and accurate LiDAR-inertial-visual fusion localization and mapping system, demonstrating significant potential for real-time 3D reconstruction and onboard robotic localization in severely degraded environments.

**Developer**: [Chunran Zheng 郑纯然](https://github.com/xuankuzcr)

<div align="center">
    <img src="pics/Framework.png" width = 100% >
</div>

### 1.1 Related video

Our accompanying video is now available on [**Bilibili**](https://www.bilibili.com/video/BV1Ezxge7EEi) and [**YouTube**](https://youtu.be/6dF2DzgbtlY).

### 1.2 Related paper

[FAST-LIVO2: Fast, Direct LiDAR-Inertial-Visual Odometry](https://arxiv.org/pdf/2408.14035)  

[FAST-LIVO2 on Resource-Constrained Platforms](https://arxiv.org/pdf/2501.13876)  

[FAST-LIVO: Fast and Tightly-coupled Sparse-Direct LiDAR-Inertial-Visual Odometry](https://arxiv.org/pdf/2203.00893)

[FAST-Calib: LiDAR-Camera Extrinsic Calibration in One Second](https://www.arxiv.org/pdf/2507.17210)

### 1.3 Our hard-synchronized equipment

We open-source our handheld device, including CAD files, synchronization scheme, STM32 source code, wiring instructions, and sensor ROS driver. Access these resources at this repository: [**LIV_handhold**](https://github.com/xuankuzcr/LIV_handhold).

### 1.4 Our associate dataset: FAST-LIVO2-Dataset
Our associate dataset [**FAST-LIVO2-Dataset**](https://connecthkuhk-my.sharepoint.com/:f:/g/personal/zhengcr_connect_hku_hk/ErdFNQtjMxZOorYKDTtK4ugBkogXfq1OfDm90GECouuIQA?e=KngY9Z) used for evaluation is also available online. These are ROS 1 bags — convert them to rosbag2 (e.g. with `rosbags-convert`) before use.

### 1.5 Our LiDAR-camera calibration method
The [**FAST-Calib**](https://github.com/hku-mars/FAST-Calib) toolkit is recommended. Its output extrinsic parameters can be directly filled into the YAML file. 

## 2. Prerequisites

### 2.1 Ubuntu and ROS 2

Tested on Ubuntu 22.04 with ROS 2 Humble ([installation](https://docs.ros.org/en/humble/Installation.html)), plus:

```bash
sudo apt install \
  ros-humble-pcl-conversions ros-humble-cv-bridge ros-humble-tf2-ros \
  ros-humble-nav-msgs ros-humble-visualization-msgs
```

### 2.2 PCL && Eigen && OpenCV && Boost && OpenMP

```bash
sudo apt install libpcl-dev libeigen3-dev libopencv-dev libopencv-contrib-dev libboost-all-dev libomp-dev
```

PCL>=1.8, Eigen>=3.3.4, OpenCV>=4.2 — all satisfied by the Ubuntu 22.04 packages.

### 2.3 Sophus and Vikit — bundled, nothing to install

Upstream asked you to install Sophus system-wide and to clone `rpg_vikit` into your catkin workspace. Neither is needed
here:

- `thirdparty/vikit_common` is an `ament_cmake` port of `rpg_vikit`'s `vikit_common` (the camera models and math
  helpers). It also bundles and exports the old **non-templated** Sophus (`strasdat/Sophus` @ `a621ff2`) that vikit and
  FAST-LIVO2 are written against, so a system Sophus is neither required nor used.
- `vikit_ros` is gone entirely. Its only use was the camera loader, which is now
  [`include/utils/camera_loader.h`](include/utils/camera_loader.h) reading the node's `camera.*` parameters.

### 2.4 Livox messages

The node depends on a **ROS 2 `livox_ros_driver` interface package** providing `livox_ros_driver/msg/CustomMsg`. It is a
hard dependency even if you only run a non-Livox LiDAR, because the message type is named in the code. Upstream vendored
plain C++ headers for it (`include/livox_ros_driver/*.h`); that does not work under ROS 2, which needs generated
typesupport to deserialize the message — so those headers were deleted and the real package is used instead. Any ROS 2
`livox_ros_driver` package with `CustomMsg` will do; in this workspace the one vendored in the `faster-lio` submodule
(`thirdparty/faster-lio/thirdparty/livox_ros_driver`) is reused so there is a single definition rather than two that can
drift apart.

## 3. Build

The package is `ament_cmake` and builds with colcon. **colcon does not descend into a package directory**, so the
vendored `thirdparty/vikit_common` has to be moved next to `fast_livo` in the workspace rather than left nested inside
it:

```bash
mkdir -p ~/ros2_ws/src
cp -r FAST-LIVO2 ~/ros2_ws/src/fast_livo
cp -r <path-to>/livox_ros_driver ~/ros2_ws/src/livox_ros_driver

mv ~/ros2_ws/src/fast_livo/thirdparty/vikit_common ~/ros2_ws/src/vikit_common
rm -rf ~/ros2_ws/src/fast_livo/thirdparty

cd ~/ros2_ws
source /opt/ros/humble/setup.bash
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

[`docker/Dockerfile_fast_livo2`](../../docker/Dockerfile_fast_livo2) in `lunarleaper-state-estimation` does exactly
this and is the reference build.

Build options:

- `-DFAST_LIVO_NATIVE_ARCH=ON` — add `-march=native` / `-mcpu=native -mtune=native -ffast-math`. Upstream did this
  unconditionally, which is wrong whenever the binary is built somewhere other than where it runs (Docker images, CI,
  cross-builds for the Pi, a different NUC generation). **Off by default**; turn it on only for a build on the target
  machine itself. The `ARM_ARCH` / `X86_ARCH` defines still follow `CMAKE_SYSTEM_PROCESSOR` either way.
- mimalloc is used if `find_package(mimalloc)` succeeds, otherwise silently skipped (unchanged from upstream).

After the build you get `fastlivo_mapping` in `install/fast_livo/lib/fast_livo` (runnable with `ros2 run`) and the
`config/` directory in `install/fast_livo/share/fast_livo`.

## 4. Run

`fastlivo_mapping` is a normal ROS 2 node (node name `laserMapping`). All configuration comes from the node's parameter
interface:

```bash
ros2 run fast_livo fastlivo_mapping --ros-args --params-file /path/to/your_params.yaml
```

Then play a rosbag2 in another terminal with `ros2 bag play`.

### 4.1 Parameter files

**The `config/*.yaml` files shipped here are still the upstream ROS 1 `rosparam` format and cannot be passed to
`--params-file`.** They are kept for their values (extrinsics, per-dataset tuning). A ROS 2 params file mirrors their
layout, but the whole thing must be wrapped in the ROS 2 preamble, and nested keys become dot-separated parameter names
(`common: {lid_topic: ...}` → `common.lid_topic`):

```yaml
/**:
  ros__parameters:
    common:
      lid_topic: /livox/lidar
      imu_topic: /livox/imu
      img_topic: /left_camera/image
      img_en: 1
      lidar_en: 1
    camera:
      cam_model: Pinhole
      cam_width: 640
      cam_height: 400
      cam_fx: 401.28
      # ...
    preprocess:
      lidar_type: 1
      scan_line: 6
      blind: 0.8
    # ...
```

Three things to watch when converting a `config/*.yaml` by hand:

- **The camera block moved into the same file.** Upstream loaded `config/camera_*.yaml` into the node's private ROS 1
  namespace and read it through `vikit_ros`. Here the camera model is read from `camera.*` parameters on the node, so
  the contents of `camera_pinhole.yaml` & co. belong under a `camera:` key in the params file.
- **Types are strict.** ROS 2 has no float parameter type, so everything upstream declared as `float` is declared as a
  `double`. Write `sigma_num: 3.0`, not `3` — an int where a double is declared is a hard failure at startup. This bites
  on `lio.sigma_num`, `vio.img_point_cov`, `vio.outlier_threshold`, `debug.plot_time` and `local_map.sliding_thresh`.
  `lio.layer_init_num` is the one integer array (`[5, 5, 5, 5, 5]`).
- **Topics are absolute, outputs are relative.** `common.lid_topic` / `common.imu_topic` / `common.img_topic` are used
  verbatim, while every publisher uses a relative name and therefore lands under the node's namespace.

A complete, working example for a Hesai + VectorNav + OAK-D rig is
[`config/fast_livo2/hesai_vectornav_oak.yaml`](../../config/fast_livo2/hesai_vectornav_oak.yaml) in
`lunarleaper-state-estimation`.

### 4.2 Namespacing

Because the publishers are relative now, a namespace remap gives the whole estimator its own subtree — which is how it
runs in this workspace:

```bash
ros2 run fast_livo fastlivo_mapping --ros-args \
  --remap __ns:=/fast_livo2 \
  --params-file .../hesai_vectornav_oak.yaml
```

### 4.3 Topics

Published (shown without namespace):

| topic | type | notes |
| --- | --- | --- |
| `cloud_registered` | `sensor_msgs/msg/PointCloud2` | world cloud; RGB-coloured in LIVO mode |
| `aft_mapped_to_init` | `nav_msgs/msg/Odometry` | the estimated pose, in the **IMU** frame |
| `path` | `nav_msgs/msg/Path` | |
| `rgb_img` | `sensor_msgs/msg/Image` | visual-map debug view; LIVO mode only |
| `mavros/vision_pose/pose` | `geometry_msgs/msg/PoseStamped` | |
| `cloud_effected` | `sensor_msgs/msg/PointCloud2` | only with `publish.pub_effect_point_en` |
| `planes` | `visualization_msgs/msg/MarkerArray` | only with `publish.pub_plane_en` |
| `LIVO2/imu_propagate` | `nav_msgs/msg/Odometry` | only with `uav.imu_rate_odom` |

`Laser_map`, `cloud_visual_sub_map_before`, `visualization_marker`, `planner_normal`, `voxels`, `dyn_obj`,
`dyn_obj_removed` and `dyn_obj_dbg_hist` are advertised but never published to — dead publishers inherited from
upstream, kept so the port stays a diff of the original.

Subscriptions use `rclcpp::SensorDataQoS()` (best effort, keep-last 200 for LiDAR and images, 2000 for IMU), which is
compatible with both reliable and best-effort publishers. The LiDAR subscription is `livox_ros_driver/msg/CustomMsg`
when `preprocess.lidar_type` is `AVIA` (1) and `sensor_msgs/msg/PointCloud2` otherwise, decided at startup.

The node also **always** broadcasts `camera_init` → `aft_mapped` on `/tf`; there is no option to disable it. Keep that
in mind if something else in your system owns that edge.

### 4.4 Output files

Everything the node writes — `Log/pcd/`, `Log/image/`, `Log/result/`, `Log/Colmap/` and the debug matrices — goes to a
path relative to `ROOT_DIR`, which CMake bakes in as the **source** directory at build time. Keep the source tree and
its existing `Log/` subdirectories around if you enable `pcd_save.pcd_save_en`, `pcd_save.colmap_output_en`,
`image_save.img_save_en` or `evo.pose_output_en`; the node does not create the directories itself.

## 5. License

The source code of this package is released under the [**GPLv2**](http://www.gnu.org/licenses/) license. For commercial use, please contact me at <zhengcr@connect.hku.hk> and Prof. Fu Zhang at <fuzhang@hku.hk> to discuss an alternative license.

## 6. Changes in this fork

### Build system

- catkin → `ament_cmake`; `package.xml` bumped to format 3. Plain-cmake (non-ROS) builds were never supported and still
  are not.
- `find_package(catkin COMPONENTS ...)` replaced by individual `find_package` calls plus `ament_target_dependencies` on
  every library and the executable.
- `Sophus` and `rpg_vikit` dropped as external dependencies in favour of `thirdparty/vikit_common` (see
  [2.3](#23-sophus-and-vikit--bundled-nothing-to-install)). `vikit_ros` and `image_transport` are gone.
- `-march/-mcpu=native` moved behind the `FAST_LIVO_NATIVE_ARCH` option, off by default (see [3](#3-build)).
- `CMAKE_BUILD_TYPE` is no longer force-set to `Release`; colcon's `--cmake-args` now wins.
- `-Wno-deprecated-declarations` added — cv_bridge and PCL both emit deprecation noise on Humble.
- Installs the libraries, `fastlivo_mapping` (into `lib/fast_livo`) and `config/`.

### ROS 1 → ROS 2 API

- `LIVMapper` takes an `rclcpp::Node::SharedPtr` instead of a `ros::NodeHandle&` and holds it as `node_`;
  `initializeSubscribersAndPublishers(nh, it)` → `initializeSubscribersAndPublishers()`, and the publisher arguments
  dropped off the `publish_*` methods since they use the members. `main.cpp` does `rclcpp::init` / `shutdown` around a
  node named `laserMapping`.
- `run()` spins with `rclcpp::spin_some(node_)` on an `rclcpp::Rate`.
- Parameters read via `declare_parameter` instead of `nh.param`, with `/` replaced by `.` in every name — in
  `readParameters()` and in `loadVoxelConfig()`, which now takes an `rclcpp::Node&`. `lio.layer_init_num` goes through
  `std::vector<int64_t>` (ROS 2's only integer-array type) and is copied into the `std::vector<int>` the config struct
  wants.
- `ros::Subscriber`/`ros::Publisher` → typed `rclcpp::Subscription`/`rclcpp::Publisher`. The LiDAR subscription is
  split into `sub_pcl_livox` / `sub_pcl_std` because the two are now distinct types.
- All publishers use relative topic names (they were absolute `/cloud_registered`, `/aft_mapped_to_init`, …), so a
  namespace remap moves the whole node. Two oversized queues were trimmed: `LIVO2/imu_propagate` 10000 → 100, `planes`
  10000 → 10.
- `image_transport::Publisher` → a plain `sensor_msgs/msg/Image` publisher. There are no `compressed`/`theora`
  transports for `rgb_img` any more.
- `vk::camera_loader::loadFromRosNs("laserMapping", ...)` → `vk::camera_loader::loadFromNode(*node_, ...)` in the new
  [`include/utils/camera_loader.h`](include/utils/camera_loader.h), which declares `camera.*` parameters on the node.
- `tf::TransformBroadcaster` (a function-local static) → an owned `tf2_ros::TransformBroadcaster` member.
- New [`include/utils/ros2_compat.h`](include/utils/ros2_compat.h) carries the small shims:
  `header.stamp.toSec()` → `stamp2sec()`, `ros::Time().fromSec()` → `sec2stamp()`,
  `tf::createQuaternionMsgFromRollPitchYaw()` → `rpy2quat()`, and `ROS_INFO/WARN/ERROR` → `RCLCPP_*`,
  `ROS_ASSERT` → `assert`.
- `ros::Timer` at 0.004 s → `create_wall_timer(4ms)` for the IMU-rate propagation callback.
- The vendored `include/livox_ros_driver/CustomMsg.h` / `CustomPoint.h` were deleted in favour of a real dependency on
  the `livox_ros_driver` interface package (see [2.4](#24-livox-messages)).
- Message types throughout: `sensor_msgs::Imu` → `sensor_msgs::msg::Imu`, `::ConstPtr` → `::ConstSharedPtr`,
  `visualization_msgs::Marker::CYLINDER` → `visualization_msgs::msg::Marker::CYLINDER`, `ros::Duration()` →
  `builtin_interfaces::msg::Duration()`, and so on.

### Fixes made along the way

- `time_list` in `IMU_Processing.h` was a non-inline free function defined in a header (`const bool time_list(...)`);
  it is `inline` now, which is what the multi-TU ament build requires.
- `Preprocess::xt32_handler` returns early on an empty cloud; the loops below it index `points[0]` and
  `points[plsize - 1]`.
- `Preprocess::pub_func` removed along with the `pub_full`/`pub_surf`/`pub_corn` members — it built a `PointCloud2` and
  threw it away, and the publishers were never advertised.
- `VoxelMapManager::pubVoxelMap` no longer creates an `ros::Rate loop(500)` and sleeps on it; that was a 2 ms stall in
  the mapping thread for no reason.
- The visual map is pruned by age. Upstream never removed a `VisualPoint`, and every `Feature` holds its whole frame
  through `img_`, so memory grew with every processed frame (~0.5 GB/min at 640×400, 20 Hz) until the host ran out.
  A point is stamped whenever it projects into the view; once a second, points not seen for `vio.map_point_max_age`
  seconds (double, default `30.0`, `<= 0` disables) are deleted along with their features and frames. Points that come
  back into view are re-created from the LiDAR map.

### Not ported

`launch/*.launch`, `rviz_cfg/fast_livo2.rviz` (an RViz 1 config), `scripts/` and the `config/*.yaml` parameter files are
the upstream ROS 1 files. They are left in the tree for reference but are neither installed nor usable as-is.
