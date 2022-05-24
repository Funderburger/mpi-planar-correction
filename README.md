## Jetson NX Live Demo
0. You'll need a conda environment with `python 3.6` and `pytorch 1.8.0`.
1. Download the pretrained model in `monodepth_nx` from here: https://drive.google.com/file/d/1atIKjMLbIPJzzYNh8vQB-P5DzABjUnfz/view?usp=sharing
2. Turn on the 3DSmartCamera (more info here: https://wiki.analog.com/resources/eval/user-guides/ad-3dsmartcam1-prz/ug_system_setup)
3. You'll need to connect your NX to the 3D SmartCamera Wi-Fi.
4. Once you are on the same (camera) network, you can do the following from the NX board:
- Terminal 1:
  - `ssh -X analog@172.16.1.1`
  - `cd ~/Workspace/aditof_sdk/build`
  - `sudo ./apps/server/aditof-server`
- Terminal 2:
  - `roscore`
- Terminal 3 (if you did not previously installed the aditof_sdk on your NX, you can do so by following these [instructions](https://github.com/analogdevicesinc/aditof_sdk) for the Linux platform with ROS support or directly write your own SD card):
  - `cd ~/workspace/aditof_ws/aditof_sdk/build/catkin_ws`
  - `source devel/setup.bash`
  - `rosrun aditof_roscpp aditof_camera_node 172.16.1.1`
- Terminal 4:
  - `rosrun rqt_reconfigure rqt_reconfigure` - inn order to set the right camera parameters (_depth_data_format_ on **MONO16** & _threshold_ = `60`
- Terminal 5:
  - `conda activate your_environment_name`
  - `cd workspace/monodepth_nx/`
  - `python test_model_nx.py`
- Terminal 6 (if you want to see the predicted point cloud and not just the depth image):
  - `source /opt/ros/melodic/setup.bash`
  - `python launch/syn_before_launch.py`
- Terminal 7:
  - `roslaunch launch/predicted_depth_2_pcd.launch` 
- Terminal 6:
  - You can now visualize the output of the model in `rviz` 

## Jetson NX ROSBAG Demo
If you don't have a camera, you can test the demo, by using this rosbag file: https://drive.google.com/file/d/14GfPFG4v82CsiTs9C0rkmPTvCin_3Y02/view?usp=sharing .
You only need to replace the commands from T1 and T2 (and of course skipping Terminal 4):
- Terminal 1:
  - `roscore`
- Terminal 2:
  - `rosbag play -l adi3DSmartCamPlanar.bag`
