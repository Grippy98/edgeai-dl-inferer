/*
 *  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
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
#include <test_cpp/include/app_dl_inferer_utils.h>

namespace ti::app_dl_inferer::common
{
    using namespace std;
    using namespace ti::dl_inferer;

    void preProcessImage(cv::Mat               &testImage,
                        cv::Mat                &preProcImage,
                        PreprocessImageConfig  &preProcCfg)
    {
        cv::Mat resizedImage;

        /* Resize. */
        cv::resize(testImage, resizedImage, cv::Size(preProcCfg.resizeWidth, preProcCfg.resizeHeight), cv::INTER_LINEAR);

        if (!preProcCfg.reverseChannel)
        {
            cv::cvtColor(resizedImage, resizedImage, cv::COLOR_BGR2RGB);
        }

        /* Center Crop. */
        if (preProcCfg.resizeHeight != preProcCfg.outDataHeight && preProcCfg.resizeWidth != preProcCfg.outDataWidth)
        {

            int32_t x_boundary = (preProcCfg.resizeWidth - preProcCfg.outDataWidth)/2;
            int32_t y_boundary = (preProcCfg.resizeHeight - preProcCfg.outDataHeight)/2;

            int32_t xMin = x_boundary;
            int32_t xMax = preProcCfg.resizeWidth-x_boundary;

            int32_t yMin = y_boundary;
            int32_t yMax = preProcCfg.resizeHeight-y_boundary;
            
            resizedImage(cv::Rect(xMin,yMin,xMax-xMin,yMax-yMin)).copyTo(preProcImage);
        }
        else
        {
            resizedImage.copyTo(preProcImage);
        }

    }

} // namespace ti::app_dl_inferer::common