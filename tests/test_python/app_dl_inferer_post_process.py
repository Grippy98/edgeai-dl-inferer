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

from classnames import *
import cv2 as _cv2
import numpy as _np
import copy as _copy

_np.set_printoptions(threshold=_np.inf, linewidth=_np.inf)

class PostProcess:
    """
    Class to create a post process context
    """
    def __init__(self, model_config):
        self.model_config = model_config

    def get(model_config):
        """
        Create a object of a subclass based on the task type
        """
        if (model_config.task_type == "classification"):
            return PostProcessClassification(model_config)
        elif (model_config.task_type == "detection"):
            return PostProcessDetection(model_config)
        elif (model_config.task_type == "segmentation"):
            return PostProcessSegmentation(model_config)
        elif (model_config.task_type == 'human_pose_estimation'):
            return PostProcessPoseEstimation(model_config)

class PostProcessClassification(PostProcess):
    def __init__(self, model_config):
        super().__init__(model_config)
        self.classnames = eval(model_config.dataset)

    def __call__(self, img, results):
        """
        Post process function for classification
        Args:
            img: Input frame in BGR
            results: output of inference
        """
        results = _np.squeeze(results)
        img = self.overlay_topN_classnames(img, results)

        return img

    def overlay_topN_classnames(self, frame, results):
        """
        Process the results of the image classification model and draw text
        describing top 5 detected objects on the image.

        Args:
            frame (numpy array): Input image in BGR format where the overlay
            should be drawn
            results (numpy array): Output of the model run
        """
        orig_width = frame.shape[1]
        orig_height = frame.shape[0]
        row_size = 40 * orig_width//1280
        font_size = orig_width/1280
        N = self.model_config.topN
        topN_classes = _np.argsort(results)[:(-1 * N) - 1:-1]
        _cv2.putText(frame, "Top %d detected classes:" % N, (5, 2 * row_size), \
                    _cv2.FONT_HERSHEY_SIMPLEX, font_size, (0, 255, 0), 2)
        row = 3
        print(f"Top {N} classes:")
        for idx in topN_classes:
            class_name = self.classnames.get(idx + self.model_config.label_offset)
            print(class_name)
            _cv2.putText(frame, \
                class_name, (5, row_size * row), _cv2.FONT_HERSHEY_SIMPLEX, \
                font_size, (255, 255, 0), 2)
            row = row + 1

        return frame

class PostProcessDetection(PostProcess):

    def __init__(self, model_config):
        super().__init__(model_config)
        self.classnames = eval(model_config.dataset)

    def __call__(self, img, results):
        """
        Post process function for detection
        Args:
            img: Input frame in BGR
            results: output of inference
        """
        for i,r in enumerate(results):
            r = _np.squeeze(r)
            if r.ndim == 1:
                r = _np.expand_dims(r, 1)
            results[i] = r

        if self.model_config.shuffle_indices:
            results_reordered = []
            for i in self.model_config.shuffle_indices:
                results_reordered.append(results[i])
            results = results_reordered

        if results[-1].ndim < 2:
            results = results[:-1]

        bbox = _np.concatenate(results, axis = -1)

        if self.model_config.formatter:
            if (self.model_config.ignore_index == None):
                bbox_copy = _copy.deepcopy(bbox)
            else:
                bbox_copy = _copy.deepcopy(\
                                    _np.delete(bbox, self.model_config.ignore_index, 1))
            bbox[..., self.model_config.formatter["dst_indices"]] = \
                             bbox_copy[..., self.model_config.formatter["src_indices"]]

        if not self.model_config.normalized_detections:
            bbox[..., (0, 2)] /= self.model_config.resize[0]
            bbox[..., (1, 3)] /= self.model_config.resize[1]

        for b in bbox:
            if b[5] > self.model_config.viz_threshold:
                class_name = self.classnames[self.model_config.label_offset[int(b[4])]]
                img = self.overlay_bounding_box(img, b, class_name)

        return img

    def overlay_bounding_box(self, frame, box, class_name):
        """
        draw bounding box at given co-ordinates.

        Args:
            frame (numpy array): Input image in BGR where the overlay should be
            drawn
            bbox : Bounding box co-ordinates in format [X1 Y1 X2 Y2]
            class_name : Name of the class to overlay
        """
        box = [int(box[0] * frame.shape[1]),\
               int(box[1] * frame.shape[0]),\
               int(box[2] * frame.shape[1]),\
               int(box[3] * frame.shape[0])]
        box_color = (20, 220, 20)
        text_color = (0, 0, 0)
        _cv2.rectangle(frame, (box[0], box[1]), (box[2], box[3]), box_color, 2)
        _cv2.rectangle(frame, (int((box[2] + box[0])/2) - 5, \
        int((box[3] + box[1])/2) + 5), (int((box[2] + box[0])/2) + 160,\
        int((box[3] + box[1])/2) - 15), box_color, -1)
        _cv2.putText(frame, class_name, (int((box[2] + box[0])/2), \
        int((box[3] + box[1])/2)), _cv2.FONT_HERSHEY_SIMPLEX, 0.5, \
        text_color)

        return frame

class PostProcessSegmentation(PostProcess):

    def __call__(self, img, results):
        """
        Post process function for segmentation
        Args:
            img: Input frame in BGR
            results: output of inference
        """
        img = self.blend_segmentation_mask(img, results[0])

        return img

    def blend_segmentation_mask(self, frame, results):
        """
        Process the result of the semantic segmentation model and return
        an image color blended with the mask representing different color
        for each class

        Args:
            frame (numpy array): Input image in BGR format which should be
            blended
            results (numpy array): Results of the model run
        """

        mask = _np.squeeze(results)

        if len(mask.shape) > 2:
            mask = mask[0]

        # Resize the mask to the original image for blending
        org_image_bgr   = frame
        org_width = frame.shape[1]
        org_height = frame.shape[0]

        mask_image_bgr = self.gen_segment_mask(mask)
        mask_image_bgr = _cv2.resize(mask_image_bgr, (org_width, org_height), \
                                                interpolation=_cv2.INTER_LINEAR)

        blend_image = _cv2.addWeighted(mask_image_bgr, 1 - self.model_config.alpha, \
                                             org_image_bgr, self.model_config.alpha, 0)

        return blend_image

    def gen_segment_mask(self, inp):
        """
        Generate the segmentation mask from the result of semantic segmentation
        model. Creates an BGR image with different colors for each class.

        Args:
            inp (numpy array): Result of the model run
        """

        r_map = (inp * 10).astype(_np.uint8)
        g_map = (inp * 20).astype(_np.uint8)
        b_map = (inp * 30).astype(_np.uint8)

        return _cv2.merge((r_map, g_map, b_map))

class PostProcessPoseEstimation(PostProcess):
    def __init__(self, model_config):
        super().__init__(model_config)

        # This list is used to identify the color of a bounding box.
        self._CLASS_COLOR_MAP = [
            (0, 0, 255) , # Person (blue).
            (255, 0, 0) ,  # Bear (red).
            (0, 255, 0) ,  # Tree (lime).
            (255, 0, 255) ,  # Bird (fuchsia).
            (0, 255, 255) ,  # Sky (aqua).
            (255, 255, 0) ,  # Cat (yellow).
        ]

        # This list is used to identify the color of the keypoint
        self.palette = _np.array([[255, 128, 0], [255, 153, 51], [255, 178, 102],
                            [230, 230, 0], [255, 153, 255], [153, 204, 255],
                            [255, 102, 255], [255, 51, 255], [102, 178, 255],
                            [51, 153, 255], [255, 153, 153], [255, 102, 102],
                            [255, 51, 51], [153, 255, 153], [102, 255, 102],
                            [51, 255, 51], [0, 255, 0], [0, 0, 255], [255, 0, 0],
                            [255, 255, 255]])

        # This list gives the information that which two keypoints needs to connect.
        self.skeleton = [[16, 14], [14, 12], [17, 15], [15, 13], [12, 13], [6, 12],
                    [7, 13], [6, 7], [6, 8], [7, 9], [8, 10], [9, 11], [2, 3],
                    [1, 2], [1, 3], [2, 4], [3, 5], [4, 6], [5, 7]]
        # Choosing color of a link
        self.pose_limb_color = self.palette[[9, 9, 9, 9, 7, 7, 7, 0, 0, 0, 0, 0, 16, 16, 16, 16, 16, 16, 16]]
        # Choosing color of a keypoint
        self.pose_kpt_color = self.palette[[16, 16, 16, 16, 16, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9]]
        self.radius = 5

    def __call__(self, img, result):
        """
        Post process function for pose estimation
        Args:
            img: Input frame
            result: output of inference
        """
        output = _np.squeeze(result[0])

        # No of rows represents no of persons in that frame, first 4 columns gives the co-ordinates of rectangle
        # 4 : score, 5 : label, from 6 to end : co-ordeinates of 17 keypoints and its confidence score.
        det_bboxes, det_scores, det_labels, kpts = _np.array(output[:, 0:4]), _np.array(output[:, 4]),\
                                                         _np.array(output[:, 5]), _np.array(output[:, 6:])
        for idx in range(len(det_bboxes)):
            det_bbox = det_bboxes[idx]
            kpt = kpts[idx]
            if det_scores[idx] > self.model_config.viz_threshold:
                color_map = self._CLASS_COLOR_MAP[int(det_labels[idx])]
                # Resizing the co-ordinates
                det_bbox[..., (0, 2)] /= self.model_config.resize[0]
                det_bbox[..., (1, 3)] /= self.model_config.resize[1]
                det_bbox[..., (0, 2)] *= img.shape[1]
                det_bbox[..., (1, 3)] *= img.shape[0]
                # Drawing rectangle
                img = _cv2.rectangle(img, (det_bbox[0], det_bbox[1]), (det_bbox[2], det_bbox[3]), color_map[::-1], 2)
                _cv2.putText(img, "id:{}".format(int(det_labels[idx])), (int(det_bbox[0]+5),\
                                int(det_bbox[1])+15), _cv2.FONT_HERSHEY_SIMPLEX, 0.5, color_map[::-1], 2)
                _cv2.putText(img, "score:{:2.1f}".format(det_scores[idx]), (int(det_bbox[0] + 5), \
                                int(det_bbox[1]) + 30), _cv2.FONT_HERSHEY_SIMPLEX, 0.5, color_map[::-1], 2)
                img = self.plot_skeleton_kpts(img, kpt)
        return img

    def plot_skeleton_kpts(self, img, kpts, steps=3):
        """
        Draw the skeleton like structure
        by joining the appropriate key points with lines
        Args:
            img: Input frame.
            kpts (numoy array): Contains the co-ordinates and confidence score of a single person.
            steps: by default 3 values are needed to represent each keypoint (x_cord, y_cord, conf).
        """

        num_kpts = len(kpts) // steps
        for kid in range(num_kpts):
            r, g, b = self.pose_kpt_color[kid]
            x_coord, y_coord = kpts[steps * kid], kpts[steps * kid + 1]
            # Resizing
            x_coord = x_coord * img.shape[1]/self.model_config.resize[0]
            y_coord = y_coord * img.shape[0]/self.model_config.resize[1]
            conf = kpts[steps * kid + 2]
            if conf > 0.5:
                # Drawing circle
                _cv2.circle(img, (int(x_coord), int(y_coord)), self.radius+3, (int(r), int(g), int(b)), -1)

        for sk_id, sk in enumerate(self.skeleton):
            r, g, b = self.pose_limb_color[sk_id]
            pos1 = (int(kpts[(sk[0]-1)*steps]), int(kpts[(sk[0]-1)*steps+1]))
            pos1 = (int(pos1[0] * img.shape[1]/self.model_config.resize[0]), int(pos1[1] * img.shape[0]/self.model_config.resize[1]))
            pos2 = (int(kpts[(sk[1]-1)*steps]), int(kpts[(sk[1]-1)*steps+1]))
            pos2 = (int(pos2[0] * img.shape[1]/self.model_config.resize[0]), int(pos2[1] * img.shape[0]/self.model_config.resize[1]))
            conf1 = kpts[(sk[0]-1)*steps+2]
            conf2 = kpts[(sk[1]-1)*steps+2]
            if conf1 > 0.5 and conf2 > 0.5:
                # Connecting two keypoints with line
                _cv2.line(img, pos1, pos2, (int(r), int(g), int(b)), thickness=2)
        return img