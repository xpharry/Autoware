/*
 *  Copyright (c) 2015, Nagoya University
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 * this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of Autoware nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>

#include <pcl/filters/voxel_grid.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include "autoware_msgs/ConfigVoxelGridFilter.h"

#include <points_downsampler/PointsDownsamplerInfo.h>

#include <chrono>

#include "points_downsampler.h"

#define MAX_MEASUREMENT_RANGE 200.0

ros::Publisher filtered_points_pub;

// Leaf size of VoxelGrid filter.
static double voxel_leaf_size = 2.0;
static double voxel_leaf_size_step = 0.2;
static double min_voxel_leaf_size = 0.2;
static double max_voxel_leaf_size = 3.0;
static int min_points_size = 1500;
static int max_points_size = 2500;
static bool use_dynamic_leaf_size = false;

static ros::Publisher points_downsampler_info_pub;
static points_downsampler::PointsDownsamplerInfo points_downsampler_info_msg;

static std::chrono::time_point<std::chrono::system_clock> filter_start,
    filter_end;

static bool _output_log = false;
static std::ofstream ofs;
static std::string filename;

static std::string POINTS_TOPIC;
static double measurement_range = MAX_MEASUREMENT_RANGE;

static void
config_callback(const autoware_msgs::ConfigVoxelGridFilter::ConstPtr &input) {
  use_dynamic_leaf_size = input->use_dynamic_leaf_size;

  if (use_dynamic_leaf_size == false) {
    voxel_leaf_size = input->voxel_leaf_size;
  }

  measurement_range = input->measurement_range;
  voxel_leaf_size_step = input->voxel_leaf_size_step;
  min_voxel_leaf_size = input->min_voxel_leaf_size;
  max_voxel_leaf_size = input->max_voxel_leaf_size;
  min_points_size = input->min_points_size;
  max_points_size = input->max_points_size;
}

static void scan_callback(const sensor_msgs::PointCloud2::ConstPtr &input) {
  pcl::PointCloud<pcl::PointXYZI> scan;
  pcl::fromROSMsg(*input, scan);

  if (measurement_range != MAX_MEASUREMENT_RANGE) {
    scan = removePointsByRange(scan, 0, measurement_range);
  }

  pcl::PointCloud<pcl::PointXYZI>::Ptr scan_ptr(
      new pcl::PointCloud<pcl::PointXYZI>(scan));
  pcl::PointCloud<pcl::PointXYZI>::Ptr filtered_scan_ptr(
      new pcl::PointCloud<pcl::PointXYZI>());

  sensor_msgs::PointCloud2 filtered_msg;

  filter_start = std::chrono::system_clock::now();

  // if voxel_leaf_size < 0.1 voxel_grid_filter cannot down sample (It is
  // specification in PCL)
  if (voxel_leaf_size < 0.1) {
    voxel_leaf_size = 0.1;
  }
  if (min_voxel_leaf_size < 0.1) {
    min_voxel_leaf_size = 0.1;
  }

  if (use_dynamic_leaf_size == true) {
    if (voxel_leaf_size < min_voxel_leaf_size) {
      voxel_leaf_size = min_voxel_leaf_size;
    }
    if (voxel_leaf_size > max_voxel_leaf_size) {
      voxel_leaf_size = max_voxel_leaf_size;
    }
  }

  pcl::VoxelGrid<pcl::PointXYZI> voxel_grid_filter;
  voxel_grid_filter.setLeafSize(voxel_leaf_size, voxel_leaf_size,
                                voxel_leaf_size);
  voxel_grid_filter.setInputCloud(scan_ptr);
  voxel_grid_filter.filter(*filtered_scan_ptr);

  if (use_dynamic_leaf_size == true) {
    if (filtered_scan_ptr->points.size() < min_points_size) {
      for (; voxel_leaf_size > min_voxel_leaf_size;
           voxel_leaf_size -= voxel_leaf_size_step) {
        pcl::VoxelGrid<pcl::PointXYZI> voxel_grid_filter;
        voxel_grid_filter.setLeafSize(voxel_leaf_size, voxel_leaf_size,
                                      voxel_leaf_size);
        voxel_grid_filter.setInputCloud(scan_ptr);
        voxel_grid_filter.filter(*filtered_scan_ptr);
        if (filtered_scan_ptr->points.size() > min_points_size) {
          break;
        }
      }
    }

    else if (filtered_scan_ptr->points.size() > max_points_size) {
      for (; voxel_leaf_size < max_voxel_leaf_size;
           voxel_leaf_size += voxel_leaf_size_step) {
        pcl::VoxelGrid<pcl::PointXYZI> voxel_grid_filter;
        voxel_grid_filter.setLeafSize(voxel_leaf_size, voxel_leaf_size,
                                      voxel_leaf_size);
        voxel_grid_filter.setInputCloud(scan_ptr);
        voxel_grid_filter.filter(*filtered_scan_ptr);
        if (filtered_scan_ptr->points.size() < max_points_size) {
          break;
        }
      }
    }
  }

  std::cout << "voxel_leaf_size:" << voxel_leaf_size << std::endl;
  std::cout << "points.size:" << filtered_scan_ptr->points.size() << std::endl;
  std::cout << std::endl;

  pcl::toROSMsg(*filtered_scan_ptr, filtered_msg);
  filter_end = std::chrono::system_clock::now();

  filtered_msg.header = input->header;
  filtered_points_pub.publish(filtered_msg);

  points_downsampler_info_msg.header = input->header;
  points_downsampler_info_msg.filter_name = "voxel_grid_filter";
  points_downsampler_info_msg.measurement_range = measurement_range;
  points_downsampler_info_msg.original_points_size = scan.size();
  if (voxel_leaf_size >= 0.1) {
    points_downsampler_info_msg.filtered_points_size =
        filtered_scan_ptr->size();
  } else {
    points_downsampler_info_msg.filtered_points_size = scan_ptr->size();
  }
  points_downsampler_info_msg.original_ring_size = 0;
  points_downsampler_info_msg.filtered_ring_size = 0;
  points_downsampler_info_msg.exe_time =
      std::chrono::duration_cast<std::chrono::microseconds>(filter_end -
                                                            filter_start)
          .count() /
      1000.0;
  points_downsampler_info_pub.publish(points_downsampler_info_msg);

  if (_output_log == true) {
    if (!ofs) {
      std::cerr << "Could not open " << filename << "." << std::endl;
      exit(1);
    }
    ofs << points_downsampler_info_msg.header.seq << ","
        << points_downsampler_info_msg.header.stamp << ","
        << points_downsampler_info_msg.header.frame_id << ","
        << points_downsampler_info_msg.filter_name << ","
        << points_downsampler_info_msg.original_points_size << ","
        << points_downsampler_info_msg.filtered_points_size << ","
        << points_downsampler_info_msg.original_ring_size << ","
        << points_downsampler_info_msg.filtered_ring_size << ","
        << points_downsampler_info_msg.exe_time << "," << std::endl;
  }
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "voxel_grid_filter");

  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");

  private_nh.getParam("points_topic", POINTS_TOPIC);
  private_nh.getParam("output_log", _output_log);
  if (_output_log == true) {
    char buffer[80];
    std::time_t now = std::time(NULL);
    std::tm *pnow = std::localtime(&now);
    std::strftime(buffer, 80, "%Y%m%d_%H%M%S", pnow);
    filename = "voxel_grid_filter_" + std::string(buffer) + ".csv";
    ofs.open(filename.c_str(), std::ios::app);
  }

  // Publishers
  filtered_points_pub =
      nh.advertise<sensor_msgs::PointCloud2>("/filtered_points", 10);
  points_downsampler_info_pub =
      nh.advertise<points_downsampler::PointsDownsamplerInfo>(
          "/points_downsampler_info", 1000);

  // Subscribers
  ros::Subscriber config_sub =
      nh.subscribe("config/voxel_grid_filter", 10, config_callback);
  ros::Subscriber scan_sub = nh.subscribe(POINTS_TOPIC, 10, scan_callback);

  ros::spin();

  return 0;
}
