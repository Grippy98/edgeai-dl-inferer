/*
 *  Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Standard headers. */
#include <string>

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

int32_t InfererConfig::getInfererConfig(const YAML::Node   &config,
                                        const std::string  &modelBasePath,
                                        const bool          enableTidlDelegate
                                        )
{
    const YAML::Node    &n = config["session"];
    int32_t             status = 0;

    enableTidl = enableTidlDelegate;

    // Validate the parsed yaml configuration and create the configuration 
    // for the inference object creation.
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
        // Initialize the inference configuration parameter object
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
    }

    return status;
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

} // namespace ti::dl_inferer
