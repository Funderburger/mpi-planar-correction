import rospy
from sensor_msgs.msg import Image, CameraInfo
from cv_bridge import CvBridge 
import cv2

import numpy as np
import time
import sys

import message_filters

import std_msgs.msg


net = None
args = None
info_pub = None
image_pub = None

bridge = CvBridge()

def callback_sync(image, camera_info):
    global info_pub
    global image_pub
    # print("e")

    try:
        pass
    except Exception as err:
        print(err)
    
    frame = bridge.imgmsg_to_cv2(image)
    frame = frame/1000.0

    scaled_image = bridge.cv2_to_imgmsg(frame)
    
    camera_info.header.frame_id = "base_link"
    
    scaled_image.header = camera_info.header
    
    #img = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
    #img2 = np.array(img, dtype=np.float32)
    #cv2.waitKey(1)

    #image_pub.publish(bridge.cv2_to_imgmsg(img2,'32FC1'))
    image_pub.publish(scaled_image)
    info_pub.publish(camera_info)


def start_node():
    global info_pub
    global image_pub

    rospy.init_node('sync_timestamps')
    rospy.loginfo('sync_timestamps started')
    
    info_sub = message_filters.Subscriber("/aditof_roscpp/aditof_camera_info", CameraInfo)
    #image_sub = message_filters.Subscriber("/aditof_roscpp/image_rect_color", Image)

    image_sub = message_filters.Subscriber("/predicted_depth_images", Image)
    # image_sub = message_filters.Subscriber("/pico_zense/depth/image_raw", Image)
    print("teva")
    ts = message_filters.ApproximateTimeSynchronizer([image_sub, info_sub], 10, 60, allow_headerless=False)
    ts.registerCallback(callback_sync)
    print("t2")

    image_pub = rospy.Publisher('/aditof_pred_image_sync', Image, queue_size=10)
    info_pub = rospy.Publisher('/aditof_camera_info_pred_sync', CameraInfo, queue_size=10)
    print("t3")

if __name__ == '__main__':
    try:
        start_node()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
