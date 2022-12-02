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

/* Module headers. */
#include <ti_post_process_human_pose_estimation.h>

namespace ti::post_process
{
using namespace std;

vector<vector<int>> CLASS_COLOR_MAP = {{0, 0, 255}, {255, 0, 0},
                                       {0, 255, 0}, {255, 0, 255},
                                       {0, 255, 255}, {255, 255, 0}};

vector<vector<int>> palette = {{255, 128, 0}, {255, 153, 51},
                               {255, 178, 102}, {230, 230, 0},
                               {255, 153, 255}, {153, 204, 255},
                               {255, 102, 255}, {255, 51, 255},
                               {102, 178, 255}, {51, 153, 255},
                               {255, 153, 153}, {255, 102, 102},
                               {255, 51, 51}, {153, 255, 153},
                               {102, 255, 102}, {51, 255, 51},
                               {0, 255, 0}, {0, 0, 255},
                               {255, 0, 0}, {255, 255, 255}};

vector<vector<int>> skeleton = {{16, 14}, {14, 12}, {17, 15}, {15, 13},
                                {12, 13}, {6, 12}, {7, 13}, {6, 7}, {6, 8},
                                {7, 9}, {8, 10}, {9, 11}, {2, 3}, {1, 2},
                                {1, 3}, {2, 4}, {3, 5}, {4, 6}, {5, 7}};

vector<vector<int>> pose_limb_color = {palette[9], palette[9], palette[9],
                                       palette[9], palette[7], palette[7],
                                       palette[7], palette[0], palette[0],
                                       palette[0], palette[0], palette[0],
                                       palette[16], palette[16], palette[16],
                                       palette[16], palette[16], palette[16],
                                       palette[16]};

vector<vector<int>> pose_kpt_color = {palette[16], palette[16], palette[16],
                                      palette[16], palette[16], palette[0],
                                      palette[0], palette[0], palette[0],
                                      palette[0], palette[0], palette[9],
                                      palette[9], palette[9], palette[9],
                                      palette[9], palette[9]};

int kptSize = 10;

PostprocessHumanPoseEstimation::PostprocessHumanPoseEstimation(const PostprocessImageConfig   &config):
    PostprocessImage(config)
{
    m_scaleX = static_cast<float>(m_config.outDataWidth)/m_config.inDataWidth;
    m_scaleY = static_cast<float>(m_config.outDataHeight)/m_config.inDataHeight;
    m_imageHolder.width = m_config.outDataWidth;
    m_imageHolder.height = m_config.outDataHeight;
    
    /* Convert RGB Color map to YUV Color map*/
    YUVColor color;
    for (auto &c : CLASS_COLOR_MAP)
    {
        getColor(&color,c[0],c[1],c[2]);
        m_yuvColorMap.push_back(color);
    }

    for (auto &c : pose_limb_color)
    {
        getColor(&color,c[0],c[1],c[2]);
        m_yuvPoseLimbColor.push_back(color);
    }

    for (auto &c : pose_kpt_color)
    {
        getColor(&color,c[0],c[1],c[2]);
        m_yuvPoseKpt.push_back(color);
    }

    getFont(&m_textFont,16);
}

/**
 *
 * @param frameData Original Data buffer where in-place updates will happen.
 * @param results
 * @returns Original frame where some in-place post processing done.
 */

void *PostprocessHumanPoseEstimation::operator()(void           *frameData,
                                                 VecDlTensorPtr &results)
{
    void *ret = frameData;
    auto *result = results[0];
    float* data = (float*)result->data;
    m_imageHolder.yRowAddr = (uint8_t *)frameData;
    m_imageHolder.uvRowAddr = (uint8_t *)frameData + (m_imageHolder.width*m_imageHolder.height);

    for(int i = 0; i < result->shape[2] ; i++)
    {
        vector<int> det_bbox;
        float det_score;
        int det_label;
        vector<float> kpt;

        det_score = data[i * 57 + 4];
        det_label = int(data[i * 57 + 5]);

        if(det_score > m_config.vizThreshold) {
            YUVColor color_map = m_yuvColorMap[det_label];

            for(int j = 6; j < 57; j++)
            {
                kpt.push_back(data[i * 57 + j]);
            }

            det_bbox.push_back(data[i * 57 + 0] * m_scaleX);
            det_bbox.push_back(data[i * 57 + 1] * m_scaleY);
            det_bbox.push_back(data[i * 57 + 2] * m_scaleX);
            det_bbox.push_back(data[i * 57 + 3] * m_scaleY);

            drawRect(&m_imageHolder,
                     det_bbox[0],
                     det_bbox[1],
                     det_bbox[2] - det_bbox[0],
                     det_bbox[3] - det_bbox[1],
                     &color_map,
                     2);

            string id = "Id : " + to_string(det_label);

            drawText(&m_imageHolder,
                     id.c_str(),
                     det_bbox[0] + 5,
                     det_bbox[1] + 15,
                     &m_textFont,
                     &color_map);

            stringstream ss;
            ss << fixed << setprecision(1) << det_score;
            string score = "Score : " + ss.str();

            drawText(&m_imageHolder,
                     score.c_str(),
                     det_bbox[0] + 5,
                     det_bbox[1] + 15 + m_textFont.height,
                     &m_textFont,
                     &color_map);

            int steps = 3;
            int num_kpts = kpt.size()/steps;
            for(int kid = 0; kid < num_kpts; kid++)
            {
                YUVColor kpt_color_map = m_yuvPoseKpt[kid];

                int x_coord = kpt[steps * kid] * m_scaleX;
                int y_coord = kpt[steps * kid + 1] * m_scaleY;
                float conf = kpt[steps * kid + 2];

                if(conf > 0.5)
                {
                    drawRect(&m_imageHolder,
                             x_coord - kptSize/2,
                             y_coord - kptSize/2,
                             kptSize,
                             kptSize,
                             &kpt_color_map,
                             -1);

                }
            }

            for(uint64_t sk_id = 0; sk_id < skeleton.size(); sk_id++)
            {
                YUVColor limb_color_map = m_yuvPoseLimbColor[sk_id];

                int p11 = kpt[(skeleton[sk_id][0] - 1) * steps] * m_scaleX;
                int p12 = kpt[(skeleton[sk_id][0] - 1) * steps + 1] * m_scaleY;

                int p21 = kpt[(skeleton[sk_id][1] - 1) * steps] * m_scaleX;
                int p22 = kpt[(skeleton[sk_id][1] - 1) * steps + 1] * m_scaleY;

                float conf1 = kpt[(skeleton[sk_id][0] - 1) * steps + 2];
                float conf2 = kpt[(skeleton[sk_id][1] - 1) * steps + 2];

                if(conf1 > 0.5 && conf2 > 0.5)
                {
                    drawLine(&m_imageHolder,
                             p11,
                             p12,
                             p21,
                             p22,
                             &limb_color_map,
                             2);
                }
            }
        }
    }
    return ret;
}

PostprocessHumanPoseEstimation::~PostprocessHumanPoseEstimation()
{
}

} // namespace ti::post_process
