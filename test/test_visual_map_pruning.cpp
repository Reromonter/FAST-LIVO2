#include <gtest/gtest.h>

#include "vio.h"

namespace
{

// Counts how many of the image buffers handed to features are still referenced.
int liveImageRefs(const cv::Mat &img) { return img.u ? img.u->refcount - 1 : 0; }

VisualPoint *addPoint(VIOManager &vio, const V3D &pos, double seen, const cv::Mat &img)
{
  VisualPoint *pt = new VisualPoint(pos);
  pt->last_seen_time_ = seen;
  Feature *ftr = new Feature(pt, new float[64], V2D(0, 0), V3D(0, 0, 1), SE3(), 0);
  ftr->img_ = img;
  pt->addFrameRef(ftr);
  vio.insertPointIntoVoxelMap(pt);
  return pt;
}

size_t mapPoints(const VIOManager &vio)
{
  size_t n = 0;
  for (const auto &voxel : vio.feat_map) n += voxel.second->voxel_points.size();
  return n;
}

class VisualMapPruning : public ::testing::Test
{
protected:
  void SetUp() override { vio.visual_submap = new SubSparseMap; }
  VIOManager vio;
};

TEST_F(VisualMapPruning, EvictsOnlyPointsOlderThanMaxAge)
{
  vio.map_point_max_age = 30.0;
  cv::Mat old_img(400, 640, CV_8UC1), new_img(400, 640, CV_8UC1);
  addPoint(vio, V3D(0.1, 0.1, 0.1), 10.0, old_img);
  addPoint(vio, V3D(0.2, 0.2, 0.2), 10.0, old_img); // same voxel as the first
  VisualPoint *kept = addPoint(vio, V3D(0.3, 0.1, 0.1), 80.0, new_img); // same voxel, still fresh
  addPoint(vio, V3D(5.0, 5.0, 5.0), 70.5, new_img); // own voxel, seen just within the age

  ASSERT_EQ(mapPoints(vio), 4u);
  EXPECT_EQ(liveImageRefs(old_img), 2);

  EXPECT_EQ(vio.pruneVisualMap(100.0), 2);
  EXPECT_EQ(mapPoints(vio), 2u);
  EXPECT_EQ(vio.feat_map.size(), 2u);
  // Deleting the points released the frame their features held.
  EXPECT_EQ(liveImageRefs(old_img), 0);
  EXPECT_EQ(liveImageRefs(new_img), 2);
  bool found = false;
  for (const auto &voxel : vio.feat_map)
    for (VisualPoint *pt : voxel.second->voxel_points) found |= (pt == kept);
  EXPECT_TRUE(found);
}

TEST_F(VisualMapPruning, DropsVoxelsThatBecomeEmpty)
{
  vio.map_point_max_age = 1.0;
  cv::Mat img(400, 640, CV_8UC1);
  addPoint(vio, V3D(0.1, 0.1, 0.1), 0.0, img);
  addPoint(vio, V3D(3.0, 0.1, 0.1), 0.0, img);

  EXPECT_EQ(vio.pruneVisualMap(10.0), 2);
  EXPECT_TRUE(vio.feat_map.empty());
  EXPECT_EQ(liveImageRefs(img), 0);
}

TEST_F(VisualMapPruning, ResetsSubmapAfterEviction)
{
  vio.map_point_max_age = 1.0;
  cv::Mat img(400, 640, CV_8UC1);
  VisualPoint *pt = addPoint(vio, V3D(0.1, 0.1, 0.1), 0.0, img);
  vio.visual_submap->voxel_points.push_back(pt);

  EXPECT_EQ(vio.pruneVisualMap(10.0), 1);
  EXPECT_TRUE(vio.visual_submap->voxel_points.empty());
}

TEST_F(VisualMapPruning, NonPositiveMaxAgeDisablesEviction)
{
  vio.map_point_max_age = 0.0;
  cv::Mat img(400, 640, CV_8UC1);
  VisualPoint *pt = addPoint(vio, V3D(0.1, 0.1, 0.1), 0.0, img);
  vio.visual_submap->voxel_points.push_back(pt);

  EXPECT_EQ(vio.pruneVisualMap(1e6), 0);
  EXPECT_EQ(mapPoints(vio), 1u);
  EXPECT_EQ(vio.visual_submap->voxel_points.size(), 1u);
}

} // namespace
