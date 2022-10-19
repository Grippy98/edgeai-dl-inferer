/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/* Standard headers. */
#include <string>
#include <filesystem>

/* Module headers. */
#include <dl_inferer/include/ti_dl_inferer.h>
#include <dl_inferer/include/ti_dl_inferer_logger.h>

#if defined(USE_DLR_RT)
#include <dl_inferer/include/ti_dlr_inferer.h>
#endif

#if defined(USE_TENSORFLOW_RT)
#include <dl_inferer/include/ti_tflite_inferer.h>
#endif

#if defined(USE_ONNX_RT)
#include <dl_inferer/include/ti_onnx_inferer.h>
#endif

/**
 * \brief Constant for DLR invalid device Id.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_INVALID                 (-1)

/**
 * \brief Constant for DLR device type CPU.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_CPU                     (1)

/**
 * \brief Constant for DLR device type GPU.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_GPU                     (2)

/**
 * \brief Constant for DLR device type OPENCL.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_OPENCL                  (3)

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{
uint8_t getTypeSize(DlInferType type)
{
    switch (type) {
        case DlInferType_Int8:
        case DlInferType_UInt8:
            return 1;
        case DlInferType_Int16:
        case DlInferType_UInt16:
        case DlInferType_Float16:
            return 2;
        case DlInferType_Int32:
        case DlInferType_UInt32:
        case DlInferType_Float32:
            return 4;
        case DlInferType_Int64:
            return 8;
        default:
            return 0;
    }
}

DlTensor::DlTensor()
{
    DL_INFER_LOG_DEBUG("DEFAULT CONSTRUCTOR\n");
}

DlTensor::DlTensor(const DlTensor& rhs):
    name(rhs.name),
    typeName(rhs.typeName),
    type(rhs.type),
    size(rhs.size),
    numElem (rhs.numElem),
    elemSize(rhs.elemSize),
    dim(rhs.dim),
    shape(rhs.shape),
    data(nullptr),
    dataAllocated(rhs.dataAllocated)
{
    /* Do not point to the other object's data pointer. It
     * should be allocated as needed in the current object.
     */
    DL_INFER_LOG_DEBUG("COPY CONSTRUCTOR\n");
}

void DlTensor::allocateDataBuffer(DLInferer& inferer)
{
    if (dataAllocated)
    {
        delete [] reinterpret_cast<uint8_t*>(data);
    }

    data = inferer.allocate(size);
    dataAllocated = true;
}

void DlTensor::dumpInfo() const
{
    if (name != nullptr)
    {
        DL_INFER_LOG_INFO("    Name          = %s\n", name);
    }

    DL_INFER_LOG_INFO("    Num Elements  = %ld\n", numElem);
    DL_INFER_LOG_INFO("    Element Size  = %d bytes\n", elemSize);
    DL_INFER_LOG_INFO("    Total Size    = %ld bytes\n", size);
    DL_INFER_LOG_INFO("    Num Dims      = %d\n", dim);
    DL_INFER_LOG_INFO("    Type          = %s (Enum: %d)\n", typeName, type);
    DL_INFER_LOG_INFO("    Shape         = ");

    for (int32_t j = 0; j < dim; j++)
    {
        DL_INFER_LOG_INFO_RAW("[%ld] ", shape[j]);
    }

    DL_INFER_LOG_INFO_RAW("\n\n");
}

DlTensor &DlTensor::operator=(const DlTensor& rhs)
{
    if (this != &rhs)
    {
        name     = rhs.name;
        typeName = rhs.typeName;
        type     = rhs.type;
        size     = rhs.size;
        elemSize = rhs.elemSize;
        numElem  = rhs.numElem;
        dim      = rhs.dim;
        shape    = rhs.shape;
        data     = nullptr;
    }

    return *this;
}

DlTensor::~DlTensor()
{
    DL_INFER_LOG_DEBUG("DESTRUCTOR\n");

    /* The allocation is based on the size in bytes andof type
     * uint8_t * to be generic. Given this, the following should
     * be safe to do so.
     */
    if (dataAllocated)
    {
        delete [] reinterpret_cast<uint8_t*>(data);
    }
}

void InfererConfig::dumpInfo()
{
    DL_INFER_LOG_INFO("InfererConfig::Model Path        = %s\n", modelFile.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Artifacts Path    = %s\n", artifactsPath.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Runtime API       = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Device Type       = %s\n", devType.c_str());
    DL_INFER_LOG_INFO_RAW("\n");
}

int32_t InfererConfig::getConfig(const std::string  &modelBasePath,
                                 const bool          enableTidlDelegate
                                )
{
    const string        &paramFile = modelBasePath + "/param.yaml";
    int32_t             status = 0;

    if (!std::filesystem::exists(paramFile))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",paramFile.c_str());
        status = -1;
        return status;
    }

    const YAML::Node    config = YAML::LoadFile(paramFile.c_str());;
    const YAML::Node    &n = config["session"];
    enableTidl = enableTidlDelegate;

    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation.
    */
    if (!n)
    {
        DL_INFER_LOG_ERROR("Inference configuration parameters  missing.\n");
        status = -1;
    }
    else if (!n["model_path"])
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid model path.\n");
        status = -1;
    }
    else if (!n["artifacts_folder"])
    {
        DL_INFER_LOG_ERROR("Artifacts directory path missing.\n");
        status = -1;
    }
    else if (!n["session_name"])
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid run-time API type.\n");
        status = -1;
    }
    else
    {
        /* Initialize the inference configuration parameter object. */
        rtType = n["session_name"].as<string>();

        if (rtType == "tvmdlr")
        {
            modelFile = modelBasePath + "/model";
        }
        else if ((rtType == "tflitert") || (rtType == "onnxrt"))
        {
            const string &modSrc = n["model_path"].as<string>();
            modelFile = modelBasePath + "/" + modSrc;
        }

        const string &artSrc = n["artifacts_folder"].as<string>();
        artifactsPath = modelBasePath + "/artifacts";

        if (n["device_type"])
        {
            devType = n["device_type"].as<string>();
        }
        else
        {
            devType = "CPU";
        }

        if (n["device_id"])
        {
            devId = n["device_id"].as<int32_t>();
        }
        else
        {
            devId = 0;
        }

        if (n["input_data_layout"])
        {
            dataLayout = n["input_data_layout"].as<std::string>();
        }
    }

    return status;
}

void PreprocessImageConfig::dumpInfo()
{
    DL_INFER_LOG_INFO_RAW("\n");
    DL_INFER_LOG_INFO("PreprocessImageConfig::modelName       = %s\n", modelName.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::rtType          = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::taskType        = %s\n", taskType.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::dataLayout      = %s\n", dataLayout.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::inDataWidth     = %d\n", inDataWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::inDataHeight    = %d\n", inDataHeight);
    DL_INFER_LOG_INFO("PreprocessImageConfig::resizeWidth     = %d\n", resizeWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::resizeHeight    = %d\n", resizeHeight);
    DL_INFER_LOG_INFO("PreprocessImageConfig::outDataWidth    = %d\n", outDataWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::outDataHeight   = %d\n", outDataHeight);
    DL_INFER_LOG_INFO("PreprocessImageConfig::inputTensorType = Enum %d\n", inputTensorType);

    DL_INFER_LOG_INFO("PreprocessImageConfig::mean          = [");
    for (uint32_t i = 0; i < mean.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %f", mean[i]);
    }

    DL_INFER_LOG_INFO_RAW(" ]\n");

    DL_INFER_LOG_INFO("PreprocessImageConfig::scale         = [");
    for (uint32_t i = 0; i < scale.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %f", scale[i]);
    }

    DL_INFER_LOG_INFO_RAW(" ]\n\n");
}


int32_t PreprocessImageConfig::getConfig(const std::string &modelBasePath)
{
    const string        &paramFile = modelBasePath + "/param.yaml";
    int32_t             status = 0;

    if (!std::filesystem::exists(paramFile))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",paramFile.c_str());
        status = -1;
        return status;
    }

    string model = modelBasePath;
    if (model.back() == '/')
    {
        model.pop_back();
    }
    modelName = std::filesystem::path(model).filename();

    const YAML::Node    yaml = YAML::LoadFile(paramFile.c_str());

    const YAML::Node   &session = yaml["session"];
    const YAML::Node   &task = yaml["task_type"];
    const YAML::Node   &preProc = yaml["preprocess"];

    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation
    */
    if (!preProc)
    {
        DL_INFER_LOG_ERROR("Preprocess configuration parameters missing.\n");
        status = -1;
    }
    else if (!session["input_mean"])
    {
        DL_INFER_LOG_ERROR("Mean value specification missing.\n");
        status = -1;
    }
    else if (!session["input_scale"])
    {
        DL_INFER_LOG_ERROR("Scale value specification missing.\n");
        status = -1;
    }
    else if (!preProc["data_layout"])
    {
        DL_INFER_LOG_ERROR("Data layout specification missing.\n");
        status = -1;
    }
    else if (!preProc["resize"])
    {
        DL_INFER_LOG_ERROR("Resize specification missing.\n");
        status = -1;
    }
    else if (!preProc["crop"])
    {
        DL_INFER_LOG_ERROR("Crop specification missing.\n");
        status = -1;
    }
    /* Check if crop information exists. */
    if (status == 0)
    {
        rtType   = session["session_name"].as<string>();
        taskType = task.as<string>();

        // Read the width and height values
        const YAML::Node &cropNode = preProc["crop"];

        if (cropNode.Type() == YAML::NodeType::Sequence)
        {
            outDataHeight = cropNode[0].as<int32_t>();
            outDataWidth  = cropNode[1].as<int32_t>();
        }
        else if (cropNode.Type() == YAML::NodeType::Scalar)
        {
            outDataHeight = cropNode.as<int32_t>();
            outDataWidth  = outDataHeight;
        }

        // Read the data layout
        dataLayout = preProc["data_layout"].as<std::string>();

        if(preProc["reverse_channels"])
        {
            reverseChannel = preProc["reverse_channels"].as<bool>();
        }

        // Read the mean values
        const YAML::Node &meanNode = session["input_mean"];
        if (!meanNode.IsNull())
        {
            for (uint32_t i = 0; i < meanNode.size(); i++)
            {
                mean.push_back(meanNode[i].as<float>());
            }
        }

        // Read the scale values
        const YAML::Node &scaleNode = session["input_scale"];
        if (!scaleNode.IsNull())
        {
            for (uint32_t i = 0; i < scaleNode.size(); i++)
            {
                scale.push_back(scaleNode[i].as<float>());
            }
        }

        // Read the width and height values
        const YAML::Node &resizeNode = preProc["resize"];

        if (resizeNode.Type() == YAML::NodeType::Sequence)
        {
            resizeHeight = resizeNode[0].as<int32_t>();
            resizeWidth  = resizeNode[1].as<int32_t>();
        }
        else if (resizeNode.Type() == YAML::NodeType::Scalar)
        {
            int32_t resize = resizeNode.as<int32_t>();

            if (resize != outDataHeight || resize != outDataWidth)
            {
                int32_t minVal = std::min(inDataHeight, inDataWidth);

                /* tiovxmultiscaler dosen't support odd resolutions */
                resizeHeight = (((inDataHeight * resize)/minVal) >> 1) << 1;
                resizeWidth  = (((inDataWidth * resize)/minVal) >> 1) << 1;
            }
            else
            {
                resizeHeight = resize;
                resizeWidth  = resize;
            }
        }

        if (mean.size() != scale.size())
        {
            DL_INFER_LOG_ERROR("The sizes of mean and scale vectors do not match.\n");
            status = -1;
        }
    }

    return status;
}

void PostprocessImageConfig::dumpInfo()
{
    DL_INFER_LOG_INFO_RAW("\n");
    DL_INFER_LOG_INFO("PostprocessImageConfig::modelName      = %s\n", modelName.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::rtType         = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::taskType       = %s\n", taskType.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::dataLayout     = %s\n", dataLayout.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::inDataWidth    = %d\n", inDataWidth);
    DL_INFER_LOG_INFO("PostprocessImageConfig::inDataHeight   = %d\n", inDataHeight);
    DL_INFER_LOG_INFO("PostprocessImageConfig::outDataWidth   = %d\n", outDataWidth);
    DL_INFER_LOG_INFO("PostprocessImageConfig::outDataHeight  = %d\n", outDataHeight);
    DL_INFER_LOG_INFO("PostprocessImageConfig::vizThreshold   = %f\n", vizThreshold);
    DL_INFER_LOG_INFO("PostprocessImageConfig::alpha          = %f\n", alpha);
    DL_INFER_LOG_INFO("PostprocessImageConfig::normDetect     = %d\n", normDetect);
    DL_INFER_LOG_INFO("PostprocessImageConfig::labelOffsetMap = [ ");

    for (const auto& [key, value] : labelOffsetMap)
    {
        DL_INFER_LOG_INFO_RAW("(%d, %d) ", key, value);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");

    DL_INFER_LOG_INFO("PostprocessImageConfig::formatter = [ ");

    for (uint32_t i = 0; i < formatter.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %d", formatter[i]);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");

    DL_INFER_LOG_INFO("PostprocessImageConfig::resultIndices = [ ");

    for (uint32_t i = 0; i < resultIndices.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %d", resultIndices[i]);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");
}

int32_t PostprocessImageConfig::getConfig(const std::string      &modelBasePath)
{
    const string        &paramFile = modelBasePath + "/param.yaml";
    int32_t             status = 0;

    if (!std::filesystem::exists(paramFile))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",paramFile.c_str());
        status = -1;
        return status;
    }

    string model = modelBasePath;
    if (model.back() == '/')
    {
        model.pop_back();
    }
    modelName = std::filesystem::path(model).filename();

    const YAML::Node    yaml = YAML::LoadFile(paramFile.c_str());

    const YAML::Node   &session = yaml["session"];
    const YAML::Node   &task = yaml["task_type"];
    const YAML::Node   &postProc = yaml["postprocess"];

    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation
    */
    if (!session)
    {
        DL_INFER_LOG_WARN("Inference configuration parameters  missing.\n");
        status = -1;
    }
    else if (!postProc)
    {
        DL_INFER_LOG_WARN("Postprocess configuration parameters missing.\n");
        status = -1;
    }
    else if (!task)
    {
        DL_INFER_LOG_WARN("Tasktype configuration parameters missing.\n");
        status = -1;
    }

    if (status == 0)
    {
        rtType   = session["session_name"].as<string>();
        taskType = task.as<string>();

        // Read the data layout
        if (postProc["data_layout"])
        {
            dataLayout = postProc["data_layout"].as<std::string>();
        }

        if (postProc["formatter"] && postProc["formatter"]["src_indices"])
        {
            const YAML::Node &formatterNode = postProc["formatter"]["src_indices"];

            /* default is assumed to be [0 1 2 3 4 5] which means
             * [x1y1 x2y2 label score].
             *
             * CASE 1: Only 2 values are specified. These are assumed to
             *         be "label" and "score". Keep [0..3] same as the default
             *         values but overwrite [4,5] with these two values.
             *
             * CASE 2: Only 4 values are specified. These are assumed to
             *         be "x1y1" and "x2y2". Keep [4,5] same as the default
             *         values but overwrite [0..3] with these four values.
             *
             * CASE 3: All 6 values are specified. Overwrite the defaults.
             *
             */
            if (formatterNode.size() == 2)
            {
                formatter[4] = formatterNode[0].as<int32_t>();
                formatter[5] = formatterNode[1].as<int32_t>();
            }
            else if ((formatterNode.size() == 6) ||
                     (formatterNode.size() == 4))
            {
                for (uint8_t i = 0; i < formatterNode.size(); i++)
                {
                    formatter[i] = formatterNode[i].as<int32_t>();
                }
            }
            else
            {
                DL_INFER_LOG_ERROR("formatter specification incorrect.\n");
                status = -1;
            }
        }

        if (postProc["ignore_index"] && !postProc["ignore_index"].IsNull())
        {
            ignoreIndex = postProc["ignore_index"].as<int32_t>();
        }

        if (postProc["normalized_detections"])
        {
            normDetect = postProc["normalized_detections"].as<bool>();
        }

        if (postProc["shuffle_indices"])
        {
            const YAML::Node indicesNode = postProc["shuffle_indices"];

            for (uint8_t i = 0; i < indicesNode.size(); i++)
            {
                resultIndices[i] = indicesNode[i].as<int32_t>();
            }
        }

        const YAML::Node   &metric = yaml["metric"];

        if (metric && metric["label_offset_pred"])
        {
            // Read the width and height values
            const YAML::Node &offset = metric["label_offset_pred"];

            if (offset.Type() == YAML::NodeType::Scalar)
            {
                /* Use "0" key to store the value. */
                labelOffsetMap[0] = offset.as<int32_t>();
            }
            else if (offset.Type() == YAML::NodeType::Map)
            {
                for (const auto& it : offset)
                {
                    if (it.second.Type() == YAML::NodeType::Scalar)
                    {
                        labelOffsetMap[it.first.as<int32_t>()] =
                            it.second.as<int32_t>();
                    }
                }
            }
            else
            {
                DL_INFER_LOG_ERROR("label_offset_pred specification incorrect.\n");
                status = -1;
            }
        }

        //Read the dataset name
        if (yaml["input_dataset"]["name"])
        {
            dataset = yaml["input_dataset"]["name"].as<std::string>();
            getClassNames(modelBasePath);
        }
    }

    return status;
}

void PostprocessImageConfig::getClassNames(const std::string &modelBasePath)
{
    const string        &datasetFile = modelBasePath + "/dataset.yaml";

    if (!std::filesystem::exists(datasetFile))
    {
        DL_INFER_LOG_WARN("The file [%s] does not exist.\n",datasetFile.c_str());
        return;
    }

    const YAML::Node    yaml = YAML::LoadFile(datasetFile.c_str());

    const YAML::Node   &categories = yaml["categories"];

    // Validate the parsed yaml configuration
    if (!categories)
    {
        DL_INFER_LOG_WARN("Parameter categories missing in dataset file.\n");
        return;
    }

    std::string     name;
    int32_t         id;

    classnames[0] = "None";

    for (YAML::Node data : categories)
    {
        id = data["id"].as<int32_t>();
        name = data["name"].as<std::string>();

        if (data["supercategory"])
        {
            name = data["supercategory"].as<std::string>() + "/" + name;
        }

        classnames[id] = name;
    }
}

DLInferer* DLInferer::makeInferer(const InfererConfig &config)
{
    DLInferer  *inter = nullptr;
    int32_t     status = 0;

    if (config.modelFile.empty())
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid model path.\n");
        status = -1;
    }
    else if (config.rtType.empty())
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid run-time API type.\n");
        status = -1;
    }
#if defined(USE_TENSORFLOW_RT)
    else if (config.rtType == DL_INFER_RTTYPE_TFLITE)
    {
        if (config.artifactsPath.empty())
        {
            DL_INFER_LOG_ERROR("Missing model artifacts path.\n");
            status = -1;
        }
        else
        {
            inter = new TFLiteInferer(config.modelFile, 
                                      config.artifactsPath,
                                      config.enableTidl);
        }
    }
#endif

#if defined(USE_DLR_RT)
    else if (config.rtType == DL_INFER_RTTYPE_DLR)
    {
        const std::string  &s = config.devType;
        int32_t             devType = DLR_DEVTYPE_INVALID;

        if (config.devId < 0)
        {
            DL_INFER_LOG_ERROR("Missing device Id.\n");
            status = -1;
        }
        else if (s.empty())
        {
            DL_INFER_LOG_ERROR("Missing device type.\n");
            status = -1;
        }
        else if (s == "CPU")
        {
            devType = DLR_DEVTYPE_CPU;
        }
        else if (s == "GPU")
        {
            devType = DLR_DEVTYPE_GPU;
        }
        else if (s == "OPENCL")
        {
            devType = DLR_DEVTYPE_OPENCL;
        }

        if (status == 0)
        {
            inter = new DLRInferer(config.artifactsPath, 
                                   devType, 
                                   config.devId, 
                                   config.enableTidl);
        }
    }
#endif

#if defined(USE_ONNX_RT)
    else if (config.rtType == DL_INFER_RTTYPE_ONNX)
    {
        if (config.artifactsPath.empty())
        {
            DL_INFER_LOG_ERROR("Missing model artifacts path.\n");
            status = -1;
        }
        else
        {
            inter = new ORTInferer(config.modelFile,
                                   config.artifactsPath,
                                   config.enableTidl);
        }
    }
#endif
    else
    {
        DL_INFER_LOG_ERROR("Unsupported RT API.\n");
    }

    return inter;
}

int32_t DLInferer::createBuffers(const VecDlTensor    *ifInfoList,
                                 VecDlTensorPtr       &vecVar,
                                 bool                 allocate)
{
    vecVar.reserve(ifInfoList->size());

    for (uint64_t i = 0; i < ifInfoList->size(); i++)
    {
        const DlTensor *ifInfo = &ifInfoList->at(i);
        DlTensor   *obj = new DlTensor(*ifInfo);

        /* Allocate data buffer. */
        if (allocate)
            obj->allocateDataBuffer(*this);

        vecVar.push_back(obj);
    }

    return 0;
}

} // namespace ti::dl_inferer
