///////////////////////////////////////////////////////////////////////////////
// this program just uses sicktoolbox to get laser scans, and then publishes
// them as ROS messages
//
// Copyright (C) 2008, Morgan Quigley
//
// I am distributing this code under the BSD license:
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright 
//     notice, this list of conditions and the following disclaimer in the 
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of Stanford University nor the names of its 
//     contributors may be used to endorse or promote products derived from 
//     this software without specific prior written permission.
//   
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.

#include <csignal>
#include <sicklms-1.0/SickLMS.hh>
#include "ros/ros.h"
#include "sensor_msgs/LaserScan.h"
using namespace SickToolbox;
using namespace std;

void publish_scan(ros::Publisher *pub, uint32_t *values, uint32_t num_values, 
                  double scale, ros::Time start, bool inverted)
{
  static int scan_count = 0;
  static double last_print_time = 0;
  sensor_msgs::LaserScan scan_msg;
  scan_msg.header.frame_id = "base_laser";
  scan_count++;
  ros::Time t = start;
  double t_d = t.toSec();
  if (t_d > last_print_time + 1)
  {
    last_print_time = t_d;
    printf("publishing scan %d\n", scan_count);
  }
  if (inverted) {
    scan_msg.angle_min = M_PI/2;
    scan_msg.angle_max = -M_PI/2;
  } else {
    scan_msg.angle_min = -M_PI/2; 
    scan_msg.angle_max = M_PI/2; 
  }
  scan_msg.angle_increment = (scan_msg.angle_max - scan_msg.angle_min) / (double)(num_values-1); 
  //scan_msg.time_increment = 1.0 / 37.5; 
  scan_msg.time_increment = 1.0 / 75.0; //37.5; 
  scan_msg.range_min = 0;
  if (scale == 0.01)
    scan_msg.range_max = 81;
  else if (scale == 0.001)
    scan_msg.range_max = 8.1;
  scan_msg.set_ranges_size(num_values);
  scan_msg.header.stamp = t;
  for (size_t i = 0; i < num_values; i++)
    scan_msg.ranges[i] = (float)values[i] * (float)scale;
/*  
  static double prev_time = 0;
  double cur_time = ros::Time::now().toSec();
  if (rand() % 10 == 0)
    printf("%f\n", cur_time - prev_time);
  prev_time = cur_time;
*/
  pub->publish(scan_msg);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "sicklms");
  string port;
  int baud;
  bool inverted;

  ros::NodeHandle nh;
  ros::Publisher scan_pub = nh.advertise<sensor_msgs::LaserScan>("scan", 1);
  nh.param("sicklms/port", port, string("/dev/ttyUSB0"));
  nh.param("sicklms/baud", baud, 500000);
  nh.param("sicklms/inverted", inverted, true);

  SickLMS::sick_lms_baud_t desired_baud = SickLMS::IntToSickBaud(baud);
  if (desired_baud == SickLMS::SICK_BAUD_UNKNOWN)
  {
    fprintf(stderr, "baud rate must be in {9600, 19200, 38400, 500000}\n");
    return 1;
  }
  uint32_t values[SickLMS::SICK_MAX_NUM_MEASUREMENTS] = {0};
  uint32_t num_values = 0;
  SickLMS sick_lms(port);
  double scale = 0;

  try
  {
    sick_lms.Initialize(desired_baud);
    SickLMS::sick_lms_measuring_units_t u = sick_lms.GetSickMeasuringUnits();
    if (u == SickLMS::SICK_MEASURING_UNITS_CM)
      scale = 0.01;
    else if (u == SickLMS::SICK_MEASURING_UNITS_MM)
      scale = 0.001;
    else
    {
      fprintf(stderr, "bogus measuring unit.\n");
      return 1;
    }
  }
  catch (...)
  {
    printf("initialize failed! are you using the correct device path?\n");
  }
  try
  {
    while (ros::ok())
    {
      ros::Time start = ros::Time::now();
      sick_lms.GetSickScan(values, num_values);
      publish_scan(&scan_pub, values, num_values, scale, start, inverted); 
      ros::spinOnce();
    }
  }
  catch (...)
  {
    printf("woah! error!\n");
  }
  try
  {
    sick_lms.Uninitialize();
  }
  catch (...)
  {
    printf("error during uninitialize\n");
    return 1;
  }
  printf("success.\n");
  
  return 0;
}

