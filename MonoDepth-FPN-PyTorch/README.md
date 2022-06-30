# MonoDepth-FPN-PyTorch

[![License][license]][license-url]

A simple end-to-end model that achieves state-of-the-art performance in depth prediction implemented in PyTorch. We used a Feature Pyramid Network (FPN) backbone to estimate depth map from a single input RGB image. We tested the performance of our model on the NYU Depth V2 Dataset (Official Split) and the KITTI Dataset (Eigen Split).

## Requirements
* Python 3.6
* PyTorch 1.7.1
* Open3D 0.11.1
* CUDA v11 (if using CUDA)
* Should work with other configuration too

## To Run

Once you are in `.venv_mono_depth`, run this to train the network:
```
python main_fpn_curv_grad.py --cuda --epochs 20
```

```
python3 main_fpn.py --cuda --bs 6
```
To continue training from a saved model, use
```
python3 main_fpn.py --cuda --bs 6 --r True --checkepoch 10
```
To visualize the reconstructed data, run the jupyter notebook in vis.ipynb.


## Data Processing
### My Custom Dataset

### From rosbag 2 images

* Modify in `save_frames.cpp` the paths according to where you want to save the images and point clouds (`directoryIr, directoryDepth, directoryPcd`).
* Modify in `save_frames.launch` the topics.
* After a `catkin_make`and `source devel/setup.bash` you can safely run:
    - `roslaunch ddd_plane_extr save_frames.launch` 
* Then, in another terminal play your rosbag:
    - `rosbag play your_rosbag.bag`

### Combine data: from 1 channel depth image to 3 channel depth image
* Run: `python combine_ir_depth.py`

### Data augmentation
* Flip images: left-right, up-down and combined thus achieving a `4x` bigger dataset:
    - `python dataset/custom_augmentation.py`

### Split the dataset into train and validation(poorly named test in my case)
* You can set the ratio train:test by setting `n` inside the script:
    - `./split_train_and_test.sh path/to/your/dataset/`

### Train the model

```
CUDA_VISIBLE_DEVICES=1 python main_fpn_curv_grad.py --cuda --bs 1 --epochs 60
```
### Verify planes curvature
* Be sure you are in the venv_mono_depth (conda or pip) environment.
* Starting from a folder with a bunch of depth images combined of 3 channels, e.g: comb_ddd_images.PNG (name of a folder)

* Create original point clouds and model estimated point clouds, from the above images, by runnning:
```
python save_normalized_raw_pred_pairs.py --input_image_path path/to/comb_ddd_images.PNG/ --cuda
```
and by changing in the code two replace statements (.replace("comb_ddd_images.PNG","orig.PCD.test") and .replace("comb_ddd_images.PNG","pred.PCD.test")).

* Next is the plane extraction from both original and preditcted point clouds:
```
roscore
```
```
rosrun ddd_plane_extr ddd_plane_extra_save _pcd_folder:=path/to/orig.PCD.test/ 
```
of course after you modified in `ddd_plane_extra_save.cpp` the `saveDirPcd` variable to `path/to/orig_planes.PCD.test/` and 
the a `regex_replace` statements: `regex_replace(pcd_file,regex("orig.PCD.test"),"pred.PCD.test");`

* After you have the planes, all you have to do is to run `point2plane.py` in order to estimate the curvature gradient:
```
python point2plane.py
```
by changing the `directory` variable to `orig_planes.PCD.test` and change the first `.replace` statement to something like this:
```
.replace("orig_planes.PCD.test","pred_planes.PCD.test").replace("_orig.pcd","_pred.pcd")
```
and the name of the `.csv` file if you want, BUT from within the `.venv_measure_curva` environment.
 
## Model

If you want to find more about the model used, you should take a look here, because this project relies mostly on their work: https://github.com/haofengac/MonoDepth-FPN-PyTorch  

