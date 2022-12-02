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

#ifndef _TI_POST_PROCESS_HUMAN_POSE_ESTIMATION_
#define _TI_POST_PROCESS_HUMAN_POSE_ESTIMATION_

/* Module headers. */
#include <ti_post_process.h>

/**
 * \defgroup group_post_process_human_pose_estimation Human Pose Estimation post-processing
 *
 * \brief Class implementing the human pose estimation post-processing logic.
 *
 * \ingroup group_post_process
 */

namespace ti::post_process
{
    /** Post-processing for human pose estimation.
     *
     * \ingroup group_post_process_human_pose_estimation.
     */
    class PostprocessHumanPoseEstimation : public PostprocessImage
    {
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessHumanPoseEstimation(const PostprocessImageConfig  &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             *
             * @param frameData  Input data frame on which overlay is done
             * @param results Detection output results from the inference
             */
            void *operator()(void              *frameData,
                             VecDlTensorPtr    &results);

            /** Destructor. */
            ~PostprocessHumanPoseEstimation();

        private:
            /** Multiplicative factor to be applied to X co-ordinates. */
            float                   m_scaleX{1.0f};

            /** Multiplicative factor to be applied to Y co-ordinates. */
            float                   m_scaleY{1.0f};

            /** Structure to hold information about NV12 Image. */
            Image                   m_imageHolder;

            /** Vector of YUV colors. */
            std::vector<YUVColor>   m_yuvColorMap;

            /** Vector of YUV colors for limb. */
            std::vector<YUVColor>   m_yuvPoseLimbColor;

            /** Vector of YUV colors for key-points. */
            std::vector<YUVColor>   m_yuvPoseKpt;

            /** Font for text. */
            FontProperty            m_textFont;


        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessHumanPoseEstimation &
                operator=(const PostprocessHumanPoseEstimation& rhs) = delete;
    };

} // namespace ti::post_process

#endif /* _TI_POST_PROCESS_HUMAN_POSE_ESTIMATION_ */

