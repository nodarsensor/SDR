import cv2 as cv
import numpy as np
from pynodar.stereo import disparity_to_pointcloud, write_ply, rectify_with_scp
from pynodar.trinsics import stereo_camera_parameters
from pynodar.utils import readstack

# Read 8-bit disparity map
disp = cv.imread("noder_0000.png", 0)

# Convert from 8-bit to Q11.4 format
disp_s16 = disp.astype(np.int16) * 16

# Read the stereo camera parameters
scp = stereo_camera_parameters("i1.yaml", "i2.yaml", "nodar_0000.yaml")

# Create point cloud from disparity map
pc = disparity_to_pointcloud(disp_s16, scp)

# Read color image
img1, img2 = readstack("nodar_0000.png")

# Rectify color image
img1r, img2r = rectify_with_scp(img1, img2, scp)

# from pynodar.stereo import get_disparity
# disp_s16 = get_disparity(img1r, img2r)

# Save point cloud in ply format
write_ply("pointcloud.ply", pc, img1r)

