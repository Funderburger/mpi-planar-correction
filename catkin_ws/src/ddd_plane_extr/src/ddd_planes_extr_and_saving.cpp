// #include <boost/filesystem.hpp>

#include <ros/ros.h>
#include <dynamic_reconfigure/server.h>
#include <visualization_msgs/Marker.h>

// Include opencv2
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// #include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/PointCloud.h>

#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/publisher.h>
#include <pcl/ModelCoefficients.h>
#include <pcl_msgs/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/common/eigen.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/range_image/range_image_planar.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/sample_consensus/sac_model_perpendicular_plane.h>
#include <pcl/filters/project_inliers.h>
#include <pcl/segmentation/extract_clusters.h>

#include <pcl/common/common.h>
#include <pcl/common/centroid.h>
#include <pcl/common/pca.h>
#include <pcl/features/fpfh.h>
#include <pcl/features/normal_3d.h>
#include <geometry_msgs/PointStamped.h>

#include <pcl/filters/crop_hull.h>
#include <pcl/surface/concave_hull.h>
#include <pcl/surface/convex_hull.h>
 

#include <iostream>
#include <experimental/filesystem>
#include <iterator>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace cv;
using namespace std;
using namespace pcl_msgs;
// namespace fs = boost::filesystem;
namespace fs = std::experimental::filesystem;

class Planes2Depth
{
public:
    // typedef pcl::PointXYZRGB Point;
    typedef pcl::PointXYZI Point;
    typedef pcl::PointCloud<Point> PointCloud;

    /*
   * constructor
   * setup which topics are passed out (advertise) and to which topics are listend (subscribe)
   */
    Planes2Depth() : it_(nh_)
    {
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

        // "~" means, that the node hand is opened within the private namespace (to get the "own" paraemters)
        ros::NodeHandle private_nh("~");
        // ########################################
        // This should be used without a subscriber
        // ########################################

        // string saveDirPcd = "/home/funderburger/work_ws/calibration_ws/planes_extr_ws/training_data/pcd_gt/";                                  // BEWARE of that last / !!! You don't want to forget that, trust me!
        // string saveDirPcd = "/home/funderburger/work_ws/calibration_ws/planes_extr_ws/catkin_ws/test_dir/pcd_combo_ir/"; // BEWARE of that last / !!! You don't want to forget that, trust me!
        string saveDirPcd = "/home/marian/calibration_ws/monodepth-FPN/MonoDepth-FPN-PyTorch/dataset/training_data/training_data/curvature_grad/camera_pred_planes/"; // BEWARE of that last / !!! You don't want to forget that, trust me!
        string saveDirDepth = "/home/marian/calibration_ws/monodepth-FPN/MonoDepth-FPN-PyTorch/dataset/training_data/training_data/curvature_grad/camera_pred_planes/"; // BEWARE of that last / !!! You don't want to forget that, trust me!
        // string saveDirDepth = "/home/funderburger/work_ws/calibration_ws/planes_extr_ws/catkin_ws/test_dir/depth_gt/"; // BEWARE of that last / !!! You don't want to forget that, trust me!
        long int np = saveDirPcd.length();
        char saveDirPcdArr[np + 1];
        long int nd = saveDirDepth.length();
        char saveDirDepthArr[nd + 1];
        strcpy(saveDirPcdArr, saveDirPcd.c_str());
        strcpy(saveDirDepthArr, saveDirDepth.c_str());
        if (private_nh.getParam("pcd_folder", _param))
        {
            ROS_INFO("Got param: %s", _param.c_str());
            int i = 0;
            fs::path p(_param.c_str());
            typedef vector<fs::path> vec; // store paths,
            vec v;                        // so we can sort them later
            copy(fs::directory_iterator(p), fs::directory_iterator(), back_inserter(v));
            sort(v.begin(), v.end());
            for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
            {
                // string pcd_file = *it.string();
                cout << "pcd_file:" << *it << '\n';
                if (pcl::io::loadPCDFile<pcl::PointXYZ>(*it, *cloud) == -1) //* load the file
                {
                    PCL_ERROR("Couldn't read file %s \n", it);
                }
                // cout << entry.path() << '\n';
                std::cout << "Loaded "
                          << cloud->width * cloud->height
                          << " data points"
                          << std::endl;
                string saveDirP(saveDirPcdArr);
                char numberDirPcdArr[6];
                sprintf(numberDirPcdArr, "%05d", i);
                string pcdFilename = saveDirP + numberDirPcdArr +"_";
                string saveDirD(saveDirDepthArr);
                char numberDirDepthArr[6];
                sprintf(numberDirDepthArr, "%05d", i);
                string noFilterFilename = saveDirD + numberDirDepthArr + "_nofilter.pcd";
                
                getGTPlanes(cloud, pcdFilename,noFilterFilename);
                i++;
            }
        }
        else
        {
            ROS_ERROR("Failed to get param 'pcd_folder' ");
        }
    }

    ~Planes2Depth() {}

    void getGTPlanes(pcl::PointCloud<pcl::PointXYZ>::Ptr inputCloud, string pcd_file_name, string no_filter_file_name)
    {
        // string only_points_path = "my_full_custom_pcdmultiP1.pcd";

        // Set variables for the current data set,
        // because these are different depending on the camera range
        // ################################## //
        //       multiP_pico_2m_50.bag
        // ################################## //
        _distanceThreshold = 0.0000130;
        // _okDistanceThreshold = 0.005;
        _clusterTolerance = 0.00006;
        _minClusterSize = 13000;
        _maxClusterSize = 250000;
        _maxIterations = 1000;
        _remained_pointcloud = 0.2;
        // _alpha = 0.01;


        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud1(new pcl::PointCloud<pcl::PointXYZ>);
        pcl::copyPointCloud(*inputCloud, *cloud1);

        std::cerr << "#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#" << std::endl;
        std::cerr << "Point cloud sizeeeeeeeeeeeeeeeee: " << cloud1->size() << std::endl;

        // cleaning the point cloud of NaNs in order to further apply the Euclidean cluster extraction
        boost::shared_ptr<std::vector<int>> indices(new std::vector<int>);
        pcl::removeNaNFromPointCloud(*cloud1, *indices);
        pcl::ExtractIndices<pcl::PointXYZ> extract;
        extract.setInputCloud(cloud1);
        extract.setIndices(indices);
        extract.setNegative(false);
        extract.filter(*cloud1);

        // ///////////////////////////////////////////////////////////////////////////////////////////

        // Preparing the plane extraction
        pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
        pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
        // Create the segmentation object
        pcl::SACSegmentation<pcl::PointXYZ> seg;
        // Optional
        seg.setOptimizeCoefficients(true);
        // Mandatory
        seg.setModelType(pcl::SACMODEL_PLANE);
        seg.setMethodType(pcl::SAC_RANSAC);
        seg.setMaxIterations(_maxIterations);
        seg.setDistanceThreshold(_distanceThreshold);

        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_rest(new pcl::PointCloud<pcl::PointXYZ>());

        int i = 0, nr_points = (int)cloud1->size(); ////////////////// ?????????????????????????
        std::cerr << "Point cloud size: " << nr_points << std::endl;

        // While nr_points% of the original cloud is still there
        while (cloud1->size() > _remained_pointcloud * nr_points)
        {
            // Extract inliers
            pcl::ExtractIndices<pcl::PointXYZ> plane_extracter;

            pcl::PointCloud<pcl::PointXYZ>::Ptr plane_only(new pcl::PointCloud<pcl::PointXYZ>());
            // Extracting the plane using RANSAC
            seg.setInputCloud(cloud1);
            seg.segment(*inliers, *coefficients);

            std::vector<int> ransac_plane_indices;
            ransac_plane_indices = inliers->indices;

            if (inliers->indices.size() == 0)
            {
                PCL_ERROR("Could not estimate a planar model for the given dataset.\n");
            }

            plane_extracter.setInputCloud(cloud1);
            plane_extracter.setIndices(inliers);
            plane_extracter.setNegative(false);
            // Get the points associated with the planar surface
            plane_extracter.filter(*plane_only);
            std::cerr << "PointCloud representing the planar component: " << plane_only->width * plane_only->height << " data points." << std::endl;

            // ////////////////////////////////////////////////////////////////////////

            // Extracting indices of detected clusters for identifying the only plane
            pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
            tree->setInputCloud(plane_only);

            std::vector<pcl::PointIndices> cluster_indices;
            pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
            ec.setClusterTolerance(_clusterTolerance); // in meters
            ec.setMinClusterSize(_minClusterSize);
            ec.setMaxClusterSize(_maxClusterSize);
            ec.setSearchMethod(tree);
            ec.setInputCloud(plane_only);

            std::vector<int> POIIndices;

            ec.extract(cluster_indices);

            // here should not be more than a single cluster extracted per plane
            int j = 0;
            for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin(); it != cluster_indices.end(); ++it)
            {
                pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZ>);
                for (const auto &idx : it->indices)
                {
                    cloud_cluster->push_back((*plane_only)[idx]); //*
                    POIIndices.push_back(ransac_plane_indices[idx]);
                }
                cloud_cluster->width = cloud_cluster->size();
                cloud_cluster->height = 1;
                cloud_cluster->is_dense = true;

                std::cout << "PointCloud representing the Cluster: " << cloud_cluster->size() << " data points." << std::endl;

                j++;
            }
            i++;

            // POI and scraps
            pcl::PointCloud<pcl::PointXYZ>::Ptr scraps_cloud(new pcl::PointCloud<pcl::PointXYZ>);

            // now we need to extract the prior detected plane
            pcl::PointIndices::Ptr clusterInliersIndices(new pcl::PointIndices);
            clusterInliersIndices->indices = POIIndices;

            // Extract Inliers from the input cloud
            pcl::ExtractIndices<pcl::PointXYZ> cluster_extracter;
            cluster_extracter.setInputCloud(cloud1);
            cluster_extracter.setIndices(clusterInliersIndices);
            cluster_extracter.setNegative(false); // Keep only the inliers (i.e. the plane)
            cluster_extracter.filter(*plane_only);

            cluster_extracter.setNegative(true); // Remove the inliers (i.e. remove only the plane and keep the rest)
            cluster_extracter.filter(*scraps_cloud);
            // //////////////////////////////////////////////////////////////////////////
            // Statistical outlier remover for a plane

            pcl::PointCloud<pcl::PointXYZ>::Ptr plane_filtered (new pcl::PointCloud<pcl::PointXYZ>);

            std::cerr << "Cloud before filtering: " << std::endl;
            std::cerr << *plane_only << std::endl;


            if (plane_only->size() > 0){
                pcl::io::savePCDFileASCII(pcd_file_name+ "_just_plane.pcd", *plane_only);
                std::cerr << "Saved " << plane_filtered->size() << " data points to just_plane" << std::endl;
                // Create the filtering object
                pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
                sor.setInputCloud (plane_only);
                sor.setMeanK (50);
                sor.setStddevMulThresh (1.5);
                sor.filter (*plane_filtered);

                std::cerr << "Cloud after filtering: " << std::endl;
                std::cerr << *plane_filtered << std::endl;

                // save the plane to a lonely file
                char number[6];
                sprintf(number, "%01d", i);
                string pcd_version = "_orig";
                pcl::io::savePCDFileASCII(pcd_file_name+ number + pcd_version +".pcd", *plane_filtered);
                std::cerr << "Saved " << plane_filtered->size() << " data points to" << pcd_file_name << std::endl;

                sor.setNegative (true);
                sor.filter (*plane_filtered);
                pcl::io::savePCDFileASCII(pcd_file_name+ number + pcd_version + "_nofilter.pcd", *plane_filtered);
                std::cerr << "Saved " << plane_filtered->size() << " data points to" << pcd_file_name << std::endl;
                
                // //////////////////////////////////////////////////////

                // adding the scraps to  the original point cloud
                cloud1->points = scraps_cloud->points;
            }

            // in case something goes wrong... (because it does, trust me, when there are too few points in some point cloud it gets stuck in this loop) so a maximum of 6 planes should be enough, so stop the loop
            if (i == 5 || plane_only->size()==0)
            {
                break;
            }
        }


    }

private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    ros::Subscriber sub_;
    // ros::Publisher pub_;
    // ros::Publisher pub2_;
    // ros::Publisher pub3_;
    // ros::Publisher pub4_;

    // image_transport::Publisher pub2_;
    // calibration parameters
    // double K[9] = {385.8655956930966, 0.0, 342.3593021849471,
    //                0.0, 387.13463636528166, 233.38372018194542,
    //                0.0, 0.0, 1.0};

    // EEPROM parameters
    // double K[9] = {386.804, 0.0, 341.675,
    //                0.0, 384, 238.973,
    //                0.0, 0.0, 1.0};

    // // Pico parameters
    // double K[9] = {460.585, 0.0, 334.080,
    //                 0.0, 460.268, 169.807,
    //                 0.0, 0.0, 1.0};

    // Pico parameters OK!!!!!!!!!!!!!!
    double K[9] = {460.585, 0.0, 334.081,
                   0.0, 460.268, 169.808,
                   0.0, 0.0, 1.0};

    double centerX_ = K[2];
    double centerY_ = K[5];
    double focalX_ = K[0];
    double focalY_ = K[4];
    // int height_ = 480;
    int height_ = 360; // only for pico (I guess it has its name for a reason! XD)
    int width_ = 640;
    int pixel_pos_x, pixel_pos_y;
    float z, u, v;
    Mat cv_image;
    std::vector<Point2d> imagePoints;
    float _distanceThreshold;
    float _okDistanceThreshold;
    float _clusterTolerance;
    float _planesDelay;
    int _minClusterSize;
    int _maxClusterSize;
    float _remained_pointcloud;
    int _maxIterations;
    float _alpha;
    string _param;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "ddd_plane_extr");

    Planes2Depth c2d;

    return 0;
}
