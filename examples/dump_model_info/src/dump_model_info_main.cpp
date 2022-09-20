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
#include <signal.h>
#include <getopt.h>

/* Module headers. */
#include <dl_inferer/include/ti_dl_inferer.h>
#include <dl_inferer/include/ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer;
using namespace ti::dl_inferer::utils;

static void showUsage(const char *name)
{
    printf(" \n");
    printf("# \n");
    printf("# %s PARAMETERS [OPTIONAL PARAMETERS]\n", name);
    printf("# OPTIONS:\n");
    printf("#  --model       |-m Path to the model directory.\n");
    printf("#  [--log-level  |-l Logging level to enable. [0: DEBUG 1:INFO 2:WARN 3:ERROR]. Default is 2.\n");
    printf("#  [--help       |-h]\n");
    printf("# \n");
    printf("# \n");
    printf("# (c) Texas Instruments 2022\n");
    printf("# \n");
    printf("# \n");
    exit(0);
}

static void ParseCmdlineArgs(int32_t    argc,
                             char      *argv[],
                             string    &modelBasePath,
                             string    &configFile)
{
    int32_t longIndex;
    int32_t opt;
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h' },
        {"model",   required_argument, 0, 'm' },
        {0,         0,                 0,  0  }
    };
    LogLevel            logLevel{WARN};

    while ((opt = getopt_long(argc, argv,"hm:l:", 
                   long_options, &longIndex )) != -1)
    {
        switch (opt)
        {
            case 'm' :
                modelBasePath = optarg;
                break;

            case 'l' :
                logLevel = static_cast<LogLevel>(strtol(optarg, NULL, 0));
                break;

            case 'h' :
            default:
                showUsage(argv[0]);
                exit(-1);

        } // switch (opt)

    } // while ((opt = getopt_long(argc, argv

    // Validate the parameters
    if (modelBasePath.empty())
    {
        showUsage(argv[0]);
        exit(-1);
    }

    configFile = modelBasePath + "/param.yaml";

    logSetLevel(logLevel);

    return;

} // End of ParseCmdLineArgs()

int main(int argc, char * argv[])
{
    InfererConfig   infConfig;
    string          modelBasePath;
    string          configFile;
    int32_t         status;

    // Parse the command line options
    ParseCmdlineArgs(argc, argv, modelBasePath, configFile);

    // Parse the input configuration file
    YAML::Node yaml = YAML::LoadFile(configFile);

    // Populate infConfig from yamlCfg
    status = infConfig.getInfererConfig(yaml, modelBasePath);

    if (status < 0)
    {
        printf("[%s:%d] ti::utils::getInfererConfig() failed.\n",
               __FUNCTION__, __LINE__);
    }
    else
    {
        DLInferer  *inferer;

        inferer = DLInferer::makeInferer(infConfig);

        if (inferer == nullptr)
        {
            printf("[%s:%d] ti::DLInferer::makeInferer() failed.\n",
                   __FUNCTION__, __LINE__);
        }
        else
        {
            inferer->dumpInfo();
        }

        delete inferer;
    }

    return status;
}
