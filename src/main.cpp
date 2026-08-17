#include "LIVMapper.h"

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("laserMapping");
  LIVMapper mapper(node);
  mapper.initializeSubscribersAndPublishers();
  mapper.run();
  rclcpp::shutdown();
  return 0;
}
