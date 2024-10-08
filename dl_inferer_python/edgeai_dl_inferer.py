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

"""
A class is defined for each Run Time supported by TIDL
constructor of the classes will load the model and returns
the object, returned object can be called with inputs to
get the inference
currently supported Run Times
1. tvmdlr
2. tflitert
3. onnxrt
"""

import numpy as _np
import threading as _threading
import os as _os
import yaml as _yaml

try:
    from dlr import DLRModel as _DLRModel

    class tvmdlr:
        """
        Abstracts the tvmdlr Run Time
        """

        def __init__(self, artifacts, model_path, enable_tidl):
            """
            Create a DLR runtime object
            Args:
                artifacts: TIDL artifacts, can be None if enable_tidl == false
                model_path: Path to tvm model file
                enable_tidl: Enable TIDL acceleration
            """
            if not enable_tidl:
                print("[ERROR] CPU Execution mode is not supported for TVM")
                return None
            self.model = _DLRModel(artifacts, "cpu")
            self.input_names = self.model.get_input_names()
            self._lock = _threading.Lock()
            self.data_type = _np.dtype(self.model.get_input_dtype(0))

        def __call__(self, input_img):
            """
            Do inference on given input image
            Args:
                input_img: Input image to do inference on
            """
            with self._lock:
                return self.model.run({self.input_names[0]: input_img})

except ImportError:
    pass

try:
    import tflite_runtime.interpreter as _tflitert_interpreter

    class tflitert:
        """
        Abstracts the tflitert Run Time
        """

        def __init__(self, artifacts, model_path, enable_tidl):
            """
            Create a TFLite runtime object
            Args:
                artifacts: TIDL artifacts, can be None if enable_tidl == false
                model_path: Path to tflite model file
                enable_tidl: Enable TIDL acceleration
            """
            if enable_tidl:
                delegate_options = {
                    "tidl_tools_path": "null",
                    "artifacts_folder": artifacts,
                    "import": "no",
                }
                tidl_delegate = [
                    _tflitert_interpreter.load_delegate(
                        "/usr/lib/libtidl_tfl_delegate.so", delegate_options
                    )
                ]
                self.interpreter = _tflitert_interpreter.Interpreter(
                    model_path, experimental_delegates=tidl_delegate
                )
            else:
                self.interpreter = _tflitert_interpreter.Interpreter(model_path)

            self.interpreter.allocate_tensors()
            self.input_details = self.interpreter.get_input_details()
            self.output_details = self.interpreter.get_output_details()
            self._lock = _threading.Lock()
            self.data_type = self.input_details[0]["dtype"]

        def __call__(self, input_img):
            """
            Do inference on given input image
            Args:
                input_img: Input image to do inference on
            """
            with self._lock:
                self.interpreter.set_tensor(self.input_details[0]["index"], input_img)
                self.interpreter.invoke()
                results = [
                    self.interpreter.get_tensor(output_detail["index"])
                    for output_detail in self.output_details
                ]
                return results

except ImportError:
    pass

try:
    import onnxruntime as _onnxruntime

    class onnxrt:
        """
        Abstracts the onnxrt Run Time
        """

        def __init__(self, artifacts, model_path, enable_tidl):
            """
            Create a ONNX runtime object
            Args:
                artifacts: TIDL artifacts, can be None if enable_tidl == false
                model_path: Path to onnx model file
                enable_tidl: Enable TIDL acceleration
            """
            if enable_tidl:
                runtime_options = {
                    "tidl_tools_path": "null",
                    "artifacts_folder": artifacts,
                }
                sess_options = _onnxruntime.SessionOptions()
                ep_list = ["TIDLExecutionProvider", "CPUExecutionProvider"]
                self.interpreter = _onnxruntime.InferenceSession(
                    model_path,
                    providers=ep_list,
                    provider_options=[runtime_options, {}],
                    sess_options=sess_options,
                )
            else:
                ep_list = ["CPUExecutionProvider"]
                self.interpreter = _onnxruntime.InferenceSession(
                    model_path,
                    providers=ep_list,
                )

            input_details = self.interpreter.get_inputs()
            self.input_name = input_details[0].name
            self.data_type = _np.dtype(input_details[0].type[7:-1])
            # default float in numpy is 64bit, in onnxruntime its 32 and no float64
            if self.data_type == _np.float64:
                self.data_type = _np.float32
            self._lock = _threading.Lock()

        def __call__(self, input_img):
            """
            Do inference on given input image
            Args:
                input_img: Input image to do inference on
            """
            with self._lock:
                return self.interpreter.run(None, {self.input_name: input_img})

except ImportError:
    pass

class ModelConfig:
    """
    Class to parse and store model parameters
    """
    count = 0
    def __init__(self, model_path, enable_tidl):
        """
        Constructor of Model class. Prases param.yaml file present in model
        directory and creates corresponding runtime objects
        Args:
            model_path: Dir of the
        """
        self.path = model_path
        self.model_name = _os.path.basename(_os.path.dirname(self.path + '/'))
        with open(self.path  + '/param.yaml', 'r') as f:
            params = _yaml.safe_load(f)
        #metrics
        self.label_offset = 0
        if 'metric' in params:
            if 'label_offset_pred' in params['metric']:
                self.label_offset = params['metric']['label_offset_pred']
        #preprocess params
        resize = params['preprocess']['resize']
        if (type(resize) == int):
            self.resize = (resize, resize)
        else:
            self.resize = resize[-1::-1]
        crop = params['preprocess']['crop']
        if (type(crop) == int):
            self.crop = (crop, crop)
        else:
            self.crop = crop[-1::-1]
        if (params['session']['input_optimization'] == True):
            self.mean = None
            self.scale = None
        else:
            self.mean = params['session']['input_mean']
            self.scale = params['session']['input_scale']
        self.reverse_channels = params['preprocess']['reverse_channels']
        self.data_layout = params['preprocess']['data_layout']
        #session params
        self.run_time = params['session']['session_name']
        if isinstance(params['session']['model_path'], list):
            self.model_path = self.path + '/' + \
                                      params['session']['model_path'][0]
        else:
            self.model_path = self.path + '/' + params['session']['model_path']
        self.artifacts = self.path + '/' + params['session']['artifacts_folder']
        #postprocess params
        self.formatter = None
        if 'formatter' in params['postprocess']:
            self.formatter = params['postprocess']['formatter']
        self.ignore_index = None
        if 'ignore_index' in params['postprocess']:
            self.ignore_index = params['postprocess']['ignore_index']
        self.normalized_detections = False
        if 'normalized_detections' in params['postprocess']:
            self.normalized_detections = \
                                  params['postprocess']['normalized_detections']
        self.shuffle_indices = None
        if 'shuffle_indices' in params['postprocess']:
            self.shuffle_indices = params['postprocess']['shuffle_indices']
        # dataset
        if 'input_dataset' in params and 'name' in params['input_dataset']:
            self.dataset = params['input_dataset']['name']
            self.classnames = self.get_class_names()
        self.task_type = params['task_type']
        self.enable_tidl = enable_tidl
        # Create Runtime
        RunTime = eval(self.run_time)
        self.run_time = RunTime(self.artifacts, self.model_path, self.enable_tidl)
        self.data_type = self.run_time.data_type
        # Set Default values of some params
        self.alpha = 0.4
        self.viz_threshold = 0.5
        self.topN = 5

    def get_class_names(self):
        if (not _os.path.exists(self.path  + '/dataset.yaml')):
            return None

        with open(self.path  + '/dataset.yaml', 'r') as f:
            dataset = _yaml.safe_load(f)

        classnames = {}
        classnames[0] = None
        for data in dataset["categories"]:
            id = data['id']
            name = data['name']
            if 'supercategory' in data:
                name = data['supercategory'] + '/' + name
            classnames[id] = name
        return classnames