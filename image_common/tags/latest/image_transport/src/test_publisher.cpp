#include <image_transport/image_publisher.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
#include <opencv_latest/CvBridge.h>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "test_publisher");
  ros::NodeHandle n;
  image_transport::ImagePublisher image_pub;
  image_pub.advertise(n, "raw_image", 1);

  //cv::WImageBuffer3_b image( cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR) );
  cv::WImageBuffer1_b image( cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE) );
  sensor_msgs::Image msg;
  sensor_msgs::CvBridge::fromIpltoRosImage(image.Ipl(), msg);
  //msg.encoding = "bgr";
  msg.header.frame_id = "base_link";
  
  ros::Rate loop_rate(5);
  while (n.ok()) {
    image_pub.publish(msg);
    ros::spinOnce();
    loop_rate.sleep();
  }
}
