cmake_minimum_required(VERSION 3.5)
project(vikit_common)

# ROS2/ament port of rpg_vikit's vikit_common (ROS-free vision/math utilities).
# Also bundles and exports the old non-templated Sophus (strasdat/Sophus @ a621ff2)
# that vikit and FAST-LIVO2 are written against.

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif ()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(ament_cmake REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(
        include
        sophus
        ${EIGEN3_INCLUDE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)

add_library(sophus SHARED
        sophus/sophus/so2.cpp
        sophus/sophus/so3.cpp
        sophus/sophus/se2.cpp
        sophus/sophus/se3.cpp
        sophus/sophus/scso3.cpp
        sophus/sophus/sim3.cpp
)

add_library(${PROJECT_NAME} SHARED
        src/atan_camera.cpp
        src/omni_camera.cpp
        src/math_utils.cpp
        src/vision.cpp
        src/performance_monitor.cpp
        src/robust_cost.cpp
        src/user_input_thread.cpp
        src/pinhole_camera.cpp
        src/equidistant_camera.cpp
        src/polynomial_camera.cpp
        src/homography.cpp
        src/img_align.cpp
)
target_link_libraries(${PROJECT_NAME} sophus ${OpenCV_LIBS})

# Consumers pick these up through the exported targets; without them the
# imported target carries no include directories at all.
foreach(target ${PROJECT_NAME} sophus)
  target_include_directories(${target} PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sophus>
          $<INSTALL_INTERFACE:include>
  )
endforeach()

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY sophus/sophus DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS ${PROJECT_NAME} sophus
        EXPORT export_${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME} sophus)
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(OpenCV Eigen3)

ament_package()
