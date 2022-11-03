#!/usr/bin/python3
#  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import numpy as np
import cv2
from app_dl_inferer_utils import *
from app_dl_inferer_post_process import *
from threading import Thread
import argparse
import sys
sys.path.append('/opt/edgeai-dl-inferer/dl_inferer_python/')
from edgeai_dl_inferer import *

def parse_command_line_args():
    """
    Function to parse the command line args supplied to the demo.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--model_directory_list", "-d", help="Model Directory", nargs='+')
    parser.add_argument("--test_image", "-i", help="Image for inference")
    parser.add_argument("--alpha", "-a", \
                        help="Alpha value for Semantic Segmentation", type=float)
    parser.add_argument("--viz_threshold", \
                        "-vt", \
                        help="Visualization threashold for Object Detection", \
                        type=float)
    parser.add_argument("--top_n", "-tn", \
                        help="Top N classes for Classification", type=int)
    parser.add_argument("--mode", help="TIDL or ARM mode")

    args = parser.parse_args()

    # AI model directory list
    model_dir_list = args.model_directory_list

    # Image to run inference on
    if args.test_image:
        img_path = args.test_image
    else:
        img_path = '/opt/edge_ai_apps/data/images/0002.jpg'

    # TIDL or ARM mode
    if args.mode:
        mode = args.mode
    else:
        mode = 'TIDL'

    # Alpha value for Semantic Segmentation
    if args.alpha:
        alpha = args.alpha
    else:
        alpha = 0.4

    # Confidence threashold for visualization of bounding boxes in Object Detection
    if args.viz_threshold:
        viz_threshold = args.viz_threshold
    else:
        viz_threshold = 0.6

    # Top N classes to display for Image Classification
    if args.top_n:
        topN = args.top_n
    else:
        topN = 5

    return model_dir_list, img_path, mode, alpha, viz_threshold, topN


def create_runtime(model_config, mode):
    """
    Function to create the open source runtime object
    """
    # Set model file, only needed for ONNX and TFLITE models
    model_path = model_config.model_path

    # Set tidl_artifacts path
    model_dir = model_config.path
    tidl_artifacts = model_dir + "/artifacts"

    # Create runtime
    RunTime = eval(model_config.run_time)

    if(mode == 'TIDL'):
        osrt = RunTime(tidl_artifacts, model_path, 1)
    elif(mode == "ARM"):
        osrt = RunTime(None, model_path, 0)
    else:
        print("Unsupported mode, defaulting to ARM mode")
        osrt = RunTime(None, model_path, 0)
    return osrt

def pre_process_image(model_config, osrt, img_path):
    """
    Function to pre process the input image based on the model params
    """
    # List of all read images
    orig_imgs = []
    # Number of images in one batch
    batch = 1
    # List of input images in one batch
    image_files = [img_path]
    # Number of channel in the image (here 3, for RGB)
    channel = 3

    # Preprocessing
    crop_dim = model_config.crop
    resize_dim = model_config.resize
    reverse_channels = model_config.reverse_channels
    data_layout = model_config.data_layout

    if(data_layout == 'NHWC'):
        shape = [batch, crop_dim[0], crop_dim[1], channel]
    elif(data_layout == 'NCHW'):
        shape = [batch, channel, crop_dim[0], crop_dim[1]]
    else:
        # Unsupported
        shape = None

    # Input numpy array
    input_data = np.zeros(shape)
    for i in range(batch):
        # Read image in BGR format and HWC layout
        img = cv2.imread(image_files[i])
        # Append original image to batch of images
        orig_imgs.append(img.copy())
        # Resize and crop
        if(crop_dim != resize_dim):
            # Resize preserving the aspect ratio
            img = resize_smaller_dim(img, resize_dim[0])
        # Crop the resized image
        img = resize(img, crop_dim[0], crop_dim[1])
        # Model needs RGB image
        if(reverse_channels == False):
            img = channel_swap_bgr_to_rgb(img)
        # HWC to data_layout
        img = change_format(img, 'HWC', data_layout)
        input_data[i] = img

    if(osrt.data_type == np.uint8):
        input_data = np.uint8(input_data)
    elif(osrt.data_type == np.float32):
        input_data = np.float32(input_data)

    return input_data, orig_imgs

def run_inference_and_post_process(osrt, input_data, post_proc, orig_imgs, model_config):
    """
    Funtion to run inference on the input image, post process and then save
    final output.
    """
    for i,data in enumerate(input_data):
        # Run Inference
        outputs = osrt([data])

        # Run post-processing
        image = post_proc(orig_imgs[i], outputs)

        img_name = model_config.task_type \
                            + "_output" \
                            + str(i) \
                            +"_" \
                            + model_config.path.split('model_zoo')[-1].replace('/','') \
                            + ".jpg"

        print(f"Post processed image saved: {img_name}")

        # Save post-processed image
        cv2.imwrite(str(img_name), image)

def main():

    # Parse command line arguments
    model_dir_list, img_path, mode, alpha, viz_threshold, topN = \
                                                parse_command_line_args()

    threads = []

    # Run for the list of models supplied in the command line
    for idx, model_dir in enumerate(model_dir_list):

        # Demo config prepared from the command line args
        demo_config = {
            'model_dir': model_dir,
            'alpha': alpha,
            'viz_threshold': viz_threshold,
            'topN': topN
        }

        # Get pre and post processor configuration
        model_config = ModelConfig(demo_config)

        # Print test configuration
        print_test_banner(model_config, img_path)

        # Create runtime
        osrt = create_runtime(model_config, mode)

        # Pre-process image
        input_data, orig_imgs = pre_process_image(model_config, osrt, img_path)

        # Get post processor class
        post_proc = PostProcess.get(model_config)

        threads.append(Thread(target=run_inference_and_post_process, args=(osrt, \
                        input_data, post_proc, orig_imgs, model_config)))

    # Start all threads
    for t in threads:
        t.start()

    # Wait for all threads to join main
    for t in threads:
        t.join()

if __name__ == '__main__':
    main()