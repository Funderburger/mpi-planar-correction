## Jetson NX Live Demo
0. Download the pretrained model in `monodepth_nx` from here: https://drive.google.com/file/d/1atIKjMLbIPJzzYNh8vQB-P5DzABjUnfz/view?usp=sharing
1. Turn on the 3DSmartCamera (more info here: https://wiki.analog.com/resources/eval/user-guides/ad-3dsmartcam1-prz/ug_system_setup)
2. You'll need to connect your NX to the 3D SmartCamera Wi-Fi.
3. Once you are on the same (camera) network, you can do the following from the NX board:
- Terminal 1:
  - `ssh -X analog@172.16.1.1`
  - `cd ~/Workspace/aditof_sdk/build`
  - `sudo ./apps/server/aditof-server`
- Terminal 2:
  - `roscore`
- Terminal 3:
  - `cd ~/workspace/aditof_ws/aditof_sdk/build/catkin_ws`
  - `source devel/setup.bash`
  - `rosrun aditof_roscpp aditof_camera_node 172.16.1.1`
- Terminal 4:
  - `rosrun rqt_reconfigure rqt_reconfigure` - order to set the right camera parameters
- Terminal 5:
  - `cd workspace/monodepth_nx/`
  - `python test_model_nx.py`
- Terminal 6:
  - You can now visualize the output of the model in `rviz` 

## Jetson NX ROSBAG Demo
In order tIf you don't have a camera, you can test the demo, by using this rosbag file: https://drive.google.com/file/d/14GfPFG4v82CsiTs9C0rkmPTvCin_3Y02/view?usp=sharing .
You only need to replace the commands from T1 and T2:
- Terminal 1:
  - `roscore`
- Terminal 2:
  - `rosbag play -l adi3DSmartCamPlanar.bag`
