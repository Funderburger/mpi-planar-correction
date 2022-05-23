# Live demo

## Jetson NX Demo
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
  - `cd workspace/monodepth_ws/`
  - `python test_model_nx.py`
- Terminal 6:
  - You can now visualize the output of the model in `rviz` 
