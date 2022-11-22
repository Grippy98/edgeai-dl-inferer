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
#if !defined(_TI_DL_INFERER_)
#define _TI_DL_INFERER_

/* Standard headers. */
#include <string>
#include <cstdarg>
#include <vector>
#include <mutex>
#include <stdexcept>

/* Third-party headers. */
#include <yaml-cpp/yaml.h>

/**
 * \defgroup group_dl_inferer Deep Learning Inference engine
 *
 * \brief Unified interface for running different DL run time APIs. The goal of
 *        this class to provide a common interface to the application for
 *        different runtime API (ex:- DLR, TFLITE,..). These underlying
 *        runtime libraries provide different capabilities and when designing
 *        the applications to use differne models, it is important that the
 *        application has a common API that it can use for DL inference and
 *        this class provides just that.
 */

/**
 * \brief Constant for DLR invalid device Id.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVID_INVALID                   (-1)

/**
 * \brief Constant for TFLITE RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_TFLITE              "tflitert"

/**
 * \brief Constant for DLR RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_DLR                 "tvmdlr"

/**
 * \brief Constant for ONNX RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_ONNX                "onnxrt"

/**
 * \brief Constant for Default Pre Proc Width
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_PREPROC_DEFAULT_WIDTH    320

/**
 * \brief Constant for Default Pre Proc Height
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_PREPROC_DEFAULT_HEIGHT   240

/**
 * \brief Constant for Default Post Proc Width
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_POSTPROC_DEFAULT_WIDTH   1280

/**
 * \brief Constant for Default Post Proc Height
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_POSTPROC_DEFAULT_HEIGHT  720

/**
 * \brief Constant for Default Post Proc Disp Width
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_DEFAULT_DISP_WIDTH       1920

/**
 * \brief Constant for Default Post Proc Disp Height
 * \ingroup group_dl_inferer
 */
#define DL_INFERER_DEFAULT_DISP_HEIGHT      1080

namespace ti::dl_inferer
{
    /**
     * \brief Enumeration for the different data types used for identifying
     *        the data types at the interface.
     *
     * \ingroup group_dl_inferer
     */
    typedef enum
    {
        /**  Invalid type. */
        DlInferType_Invalid = 0,

        /** Data type signed 8 bit integer. */
        DlInferType_Int8    = 2,

        /** Data type unsigned 8 bit integer. */
        DlInferType_UInt8   = 3,

        /** Data type signed 16 bit integer. */
        DlInferType_Int16   = 4,

        /** Data type unsigned 16 bit integer. */
        DlInferType_UInt16  = 5,

        /** Data type signed 32 bit integer. */
        DlInferType_Int32   = 6,

        /** Data type unsigned 32 bit integer. */
        DlInferType_UInt32  = 7,

        /** Data type signed 64 bit integer. */
        DlInferType_Int64   = 8,

        /** Data type 16 bit floating point. */
        DlInferType_Float16 = 9,

        /** Data type 32 bit floating point. */
        DlInferType_Float32 = 10,

     } DlInferType;

    /**
     * \brief Configuration for the DL inferer.
     *
     * \ingroup group_dl_inferer
     */
    struct InfererConfig
    {
        /** Path to the model directory or a file.
         *  - file for TFLITE and ONNX
         *  - directory for DLR
         **/
        std::string modelFile{};

        /** Path to the directory containing the model artifacts. This is only
         *  valid for TFLITE models and is not looked at for the other ones.
         */
        std::string artifactsPath{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        std::string rtType{};

        /** Type of the device. This field is specific to the DLR API and
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        std::string devType{};

        /** Id of the device. This field is specific to the DLR API and
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        int32_t     devId{DLR_DEVID_INVALID};

        /** Should TIDL be enabled. This field is specific to the DLR API and 
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        bool        enableTidl{};

        /** Layout of the data. Allowed values. */
        std::string dataLayout{"NCHW"};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse inference configuration. */
        int32_t getInfererConfig(const std::string  &modelBasePath,
                                 const bool          enableTidlDelegate);
    };

    /**
     * \brief Configuration for the Pre Process.
     *
     * \ingroup group_dl_inferer
     */
    struct PreprocessImageConfig
    {
        /** Name of the model. */
        std::string         modelName{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        std::string         rtType{};

        /** Task type.
         *  - detection
         *  - segmentation
         *  - classification
         */
        std::string         taskType{};

        /** Width of the input data. */
        int32_t             inDataWidth{DL_INFERER_PREPROC_DEFAULT_WIDTH};

        /** Height of the input data. */
        int32_t             inDataHeight{DL_INFERER_PREPROC_DEFAULT_HEIGHT};

        /** Out width. */
        int32_t             outDataWidth{DL_INFERER_PREPROC_DEFAULT_WIDTH};

        /** Out height. */
        int32_t             outDataHeight{DL_INFERER_PREPROC_DEFAULT_HEIGHT};

        /** Mean values to apply during normalization. */
        std::vector<float>  mean;

        /** Scale values to apply during normalization. */
        std::vector<float>  scale;

        /** Resize width. */
        int32_t             resizeWidth{DL_INFERER_PREPROC_DEFAULT_WIDTH};

        /** Resize height. */
        int32_t             resizeHeight{DL_INFERER_PREPROC_DEFAULT_HEIGHT};

        /** Layout of the data. Allowed values. */
        std::string         dataLayout{"NCHW"};

        int32_t             numChans{0};

        /** If preprocess is reverse channel. */
        bool                reverseChannel{false};

        /** Data type of Input tensor. */
        DlInferType         inputTensorType{DlInferType_Invalid};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse pre process configuration. */
        int32_t getPreprocessImageConfig(const std::string      &modelBasePath);
    };

    /**
     * \brief Configuration for the Post Process.
     *
     * \ingroup group_dl_inferer
     */

    struct PostprocessImageConfig
    {

    public:
        /** Name of the model. */
        std::string                             modelName{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        std::string                             rtType{};

        /** Task type.
         *  - detection
         *  - segmentation
         *  - classification
         */
        std::string                             taskType{};

        /** Layout of the data. Allowed values. */
        std::string                             dataLayout{"NCHW"};

        /** Optional offset to be applied when detecting the output
         * class. This is applicable for image classification and
         * detection cases only.
         * Classification - a single scalar value
         * Detection      - a map
         */
        std::map<int32_t,int32_t>               labelOffsetMap{{0,0}};

        /** Order of results for detection use case
         * default is assumed to be [0 1 2 3 4 5] which means
         * [x1y1 x2y2 label score]
         */
        std::vector<int32_t>                    formatter{0, 1, 2, 3, 4, 5};

        /** Ignore given index*/
        int32_t                                 ignoreIndex{-1};

        /** If detections are normalized to 0-1 */
        bool                                    normDetect{false};

        /** Order of tensors for detection results */
        std::vector<int32_t>                    resultIndices{0, 1, 2, 3};

        /** Multiplicative factor to be applied to Y co-ordinates. This is used
         * for visualization of the bounding boxes for object detection post-
         * processing only.
         */
        float                                   vizThreshold{0.50f};

        /** Alpha value for blending. This is used for semantic segmentation
         *  post-processing only.
         */
        float                                   alpha{0.5f};

        /** Number of classification results to pick from the top of the model output. */
        int32_t                                 topN{5};

        /** Width of the output to display after adding tile. */
        int32_t                                 dispWidth{DL_INFERER_DEFAULT_DISP_WIDTH};

        /** Height of the output to display after adding tile. */
        int32_t                                 dispHeight{DL_INFERER_DEFAULT_DISP_HEIGHT};

        /** Width of the input data. */
        int32_t                                 inDataWidth{DL_INFERER_POSTPROC_DEFAULT_WIDTH};

        /** Height of the input data. */
        int32_t                                 inDataHeight{DL_INFERER_POSTPROC_DEFAULT_HEIGHT};

        /** Width of the output data. */
        int32_t                                 outDataWidth{DL_INFERER_POSTPROC_DEFAULT_WIDTH};

        /** Height of the output data. */
        int32_t                                 outDataHeight{DL_INFERER_POSTPROC_DEFAULT_HEIGHT};

        /** Name of the dataset. */
        std::string                             dataset{};

        /** An array of strings for object class names. */
        std::map<int32_t, std::string>          classnames{};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse post process configuration. */
        int32_t getPostprocessImageConfig(const std::string &modelBasePath);

    private:
        /** Helper function to parse dataset.yaml and get classname from it. */
        void    getClassNames(const std::string &modelBasePath);
    };

    /* Forward declaration. */
    class DLInferer;

    /**
     * \brief DLR Interface context. This captures various attributes associated
     *        with the inputs and outputs of the model. The 'data' field may be
     *        undefined but all other parameters will have valid values, if the
     *        model defines them.
     *
     *        For TVM generated models, the 'name' field could be NULL for the
     *        output paramaters.
     *
     * \ingroup group_dl_inferer
     */
    class DlTensor
    {
        public:
            /** Name of the element. */
            const char             *name{nullptr};

            /** String representation of the element type. */
            const char             *typeName{nullptr};

            /** Unified type across APIs. */
            DlInferType             type;

            /** Total size in bytes of the input. This should be equal to
             *  numElem * sizeof(type).
             */
            int64_t                 size{};

            /** Total number of elements in the input. This should be equal
             *  to the product of all dimensions. The size of the type is
             *  not accounted in this.
             */
            int64_t                 numElem{};

            /** Element size in bytes. */
            int32_t                 elemSize{};

            /** Dimensions. */
            int32_t                 dim{};

            /** Shape information. */
            std::vector<int64_t>    shape;

            /** Data buffer. */
            void                   *data{nullptr};

            /**
             * Default constructor.
             */
            DlTensor();

            /**
             * Copy constructor.
             */
            DlTensor(const DlTensor& rhs);

            /**
             * Dumps the information to the screen.
             */
            void dumpInfo() const;

            /**
             * Allocate memory for the buffer.
             */
            void allocateDataBuffer(DLInferer& inferer);

            /**
             * Assignment operator.
             *
             */
            DlTensor &operator=(const DlTensor& rhs);

            /**
             * Destructor
             */
            ~DlTensor();

        private:

            /** Flag to track if memory has been allocated. */
            bool        dataAllocated{false};
    };

    /**
     * \brief Alias for a vector of DL interface info objects.
     *
     * \ingroup group_dl_inferer
     */
    using VecDlTensor = std::vector<DlTensor>;

    /** Alias for a vector of DlTensor pointers
     * \ingroup group_dl_inferer
     */
    using VecDlTensorPtr = std::vector<DlTensor *>;

    /** \brief An abstract base class for different class of RT inference API.
     *
     * \ingroup group_dl_inferer
     */
    class DLInferer
    {
        public:
            /**
             * Runs the model.
             *
             * @param inputs Input buffers to set for inference run
             * @param outputs Output buffers to set for inference run
             *
             * @returns 0 upon success. A nagative value otherwise.
             */
            virtual int32_t run(const VecDlTensorPtr &inputs,
                                VecDlTensorPtr       &outputs) = 0;

            /**
             * Dumps the model information to the screen.
             */
            virtual void dumpInfo() = 0;

            /**
             * Returns an array containing detailed information on the inputs
             * of the model.
             *
             * @returns An array of input interface parameters.
             */
            virtual const VecDlTensor *getInputInfo() = 0;

            /**
             * Returns an array containing detailed information on the outputs
             * of the model.
             *
             * @returns An array of output interface parameters.
             */
            virtual const VecDlTensor *getOutputInfo() = 0;

            /**
             * Returns an allocated pointer that can be consumed by inference
             * of the model by this framework.
             *
             * @returns An pointer to allocated memory.
             */
            virtual void *allocate(int64_t size)
            {
                return new uint8_t [size];
            }

            /** Factory method for making a specifc inferer based on the
             * configuration passed.
             *
             * @param config Configuration specifying the type of inferer and
             *               associated parameters.
             * @returns A valid inferer if success. A nullptr otherwise.
             */
            static DLInferer* makeInferer(const InfererConfig &config);

            /**
             * Creates the descriptor based on the information from the
             * inference model interface information.
             *
             * @param ifInfoList Vector of inference model interface parameters
             * @param vecVar     Vector of descriptors created by this function
             * @param allocate   Allocate memory if True
             */
            int32_t createBuffers(const VecDlTensor    *ifInfoList,
                                  VecDlTensorPtr        &vecVar,
                                  bool                  allocate);

            /**
             * Destructor.
             */
            virtual ~DLInferer(){}

        protected:
            /** Mutex for multi-thread access control. */
            std::mutex  m_mutex;
    };

#define DL_INFER_GET_EXCL_ACCESS    std::unique_lock<std::mutex> lock(this->m_mutex)
} // namespace ti::dl_inferer

#endif // _TI_DL_INFERER_
