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

#ifndef _TI_POST_PROCESS_
#define _TI_POST_PROCESS_

/* Standard headers. */
#include <string>
#include <algorithm>

/* Third Party headers. */
#include <iomanip>

/* Module Headers. */
#include <ti_dl_inferer.h>
#include <ti_post_process_utils.h>

/**
 * \defgroup group_post_process Post Process in NV12
 *
 * \brief Unified interface for running different Post Process APIs.
 *        The goal of this class to provide a common interface to the 
 *        application for doing post process in NV12 buffers.
 */

namespace ti::post_process
{
    using namespace ti::dl_inferer;

    class PostprocessImage
    {
        /** Base class for images based post-processing. This class forms as a base
        * class for different concrete post-processing algorithms. This does not
        * provide polymorphic operations since the language does not allow virtual
        * functions that are abstract and templated.
        *
        * The design is that this class holds common data across different objects
        * and provides helpe functions for parsing and storing configuration data.
        * Any configuration specific data needed beyoond this basic capability will
        * be handled by the sub-classes as needed.
        *
        * \ingroup group_post_process
        */
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessImage(const PostprocessImageConfig   &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             */
            virtual void *operator()(void              *frameData,
                                     VecDlTensorPtr    &results) = 0;

            /** Destructor. */
            virtual ~PostprocessImage();

            /** Factory method for making a specifc post-process object based on the
             * configuration passed.
             *
             * @param config   Configuration information not present in YAML
             * @returns A valid post-process object if success. A nullptr otherwise.
             */
            static PostprocessImage* makePostprocessImageObj(
                                        const PostprocessImageConfig   &config);

            /** Return the task type string. */
            const std::string &getTaskType();

            /** Title. */
            std::string                     m_title;

        protected:
            /** Configuration information. */
            const PostprocessImageConfig    m_config{};

        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessImage & operator=(const PostprocessImage& rhs) = delete;
    };

} // namespace ti::post_process

#endif /* _TI_POST_PROCESS_ */
