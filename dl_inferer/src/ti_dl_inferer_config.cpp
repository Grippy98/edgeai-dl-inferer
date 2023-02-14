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

/* Third-party headers. */
#include <yaml-cpp/yaml.h>

/* Module headers. */
#include <ti_dl_inferer_config.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{

void InfererConfig::dumpInfo()
{
    DL_INFER_LOG_INFO("InfererConfig::Model Path        = %s\n", modelFile.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Artifacts Path    = %s\n", artifactsPath.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Runtime API       = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Device Type       = %s\n", devType.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Enable TIDL       = %d\n", enableTidl);
    DL_INFER_LOG_INFO("InfererConfig::Core Number       = %d\n", coreNumber);
    DL_INFER_LOG_INFO_RAW("\n");
}

int32_t InfererConfig::getConfig(const std::string  &modelBasePath,
                                 const bool          enableTidlDelegate,
                                 const int           coreNum
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
    coreNumber = coreNum;


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

} // namespace ti::dl_inferer