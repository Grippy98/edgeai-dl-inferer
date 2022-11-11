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

#ifndef _POST_PROCESS_IMAGE_SEGMENTATION_H_
#define _POST_PROCESS_IMAGE_SEGMENTATION_H_

/* Module headers. */
#include <test_cpp/include/app_dl_inferer_post_process.h>

/**
 * \defgroup group_dl_inferer_cpp_test_segmentation Semantic Segmentation post-processing
 *
 * \brief Class implementing the image based semantic segmentation post-processing
 *        logic.
 *
 * \ingroup group_dl_inferer_cpp_test_post_proc
 */

namespace ti::app_dl_inferer::common
{
    /** Post-processing for semantic segmentation.
     *
     * \ingroup group_dl_inferer_cpp_test_segmentation
     */
    class PostprocessImageSemanticSeg : public PostprocessImage
    {
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessImageSemanticSeg(const PostprocessImageConfig    &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             *
             * @param frameData Input data frame on which results are overlaid
             * @param results Segmentation output results from the inference
             */
            void *operator()(void              *frameData,
                             VecDlTensorPtr    &results);

            /** Destructor. */
            ~PostprocessImageSemanticSeg();

        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessImageSemanticSeg &
                operator=(const PostprocessImageSemanticSeg& rhs) = delete;
    };
} // namespace ti::app_dl_inferer::common

#endif /* _POST_PROCESS_IMAGE_SEGMENTATION_H_ */