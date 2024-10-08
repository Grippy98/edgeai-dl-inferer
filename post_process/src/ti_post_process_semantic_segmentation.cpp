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
#include <ti_post_process_semantic_segmentation.h>
uint8_t RGB_COLOR_MAP[26][3] = {{255,0,0},{0,255,0},{73,102,92},
                                {255,255,0},{0,255,255},{0,99,245},
                                {255,127,0},{0,255,100},{235,117,117},
                                {242,15,109},{118,194,167},{255,0,255},
                                {231,200,255},{255,186,239},{0,110,0},
                                {0,0,255},{110,0,0},{110,110,0},
                                {100,0,50},{90,60,0},{255,255,255} ,
                                {170,0,255},{204,255,0},{78,69,128},
                                {133,133,74},{0,0,110}};

namespace ti::post_process
{
#define INVOKE_BLEND_LOGIC(T)                           \
    blendSegMask(reinterpret_cast<uint8_t*>(frameData), \
                 reinterpret_cast<T*>(buff->data),      \
                 m_config.inDataWidth,                  \
                 m_config.inDataHeight,                 \
                 m_config.outDataWidth,                 \
                 m_config.outDataHeight,                \
                 m_config.alpha,                        \
                 mYUVColorMap,                          \
                 mMaxColorClass)

PostprocessSemanticSegmentation::PostprocessSemanticSegmentation(const PostprocessImageConfig   &config):
    PostprocessImage(config)
{
    /* Generate YUV Color map from RGB Color Map. */
    mMaxColorClass = sizeof(RGB_COLOR_MAP)/sizeof(RGB_COLOR_MAP[0]);
    mYUVColorMap = new uint8_t*[mMaxColorClass];
    for(int i = 0; i < mMaxColorClass; ++i)
    {
        mYUVColorMap[i] = new uint8_t[3];
        uint8_t R = RGB_COLOR_MAP[i][0];
        uint8_t G = RGB_COLOR_MAP[i][1];
        uint8_t B = RGB_COLOR_MAP[i][2];
        mYUVColorMap[i][0] = RGB2Y(R,G,B);
        mYUVColorMap[i][1] = RGB2U(R,G,B);
        mYUVColorMap[i][2] = RGB2V(R,G,B);
    }
}

/**
 * For every pixel in input frame, this will find the scaled co-ordinates for a
 * downscaled result and use the color associated with detected class ID.
 *
 * @param frame Original NV12 data buffer, where the in-place updates will happen
 * @param classes Reference to a vector of vector of floats representing the output
 *          from an inference API. It should contain 1 vector describing the class ID
 *          detected for that pixel.
 * @returns original frame with some in-place post processing done
 */
template <typename T1, typename T2>
static T1 *blendSegMask(T1         *frame,
                        T2         *classes,
                        int32_t     inDataWidth,
                        int32_t     inDataHeight,
                        int32_t     outDataWidth,
                        int32_t     outDataHeight,
                        float       alpha,
                        uint8_t    **colorMap,
                        uint8_t     maxClass)
{
    uint8_t     a;
    uint8_t     sa;
    uint8_t*    uvPtr;
    uint8_t     y_m;
    uint8_t     u_m;
    uint8_t     v_m;
    int32_t     w;
    int32_t     h;
    int32_t     sw;
    int32_t     sh;
    int32_t     class_id;
    int32_t     rowOffset;

    a  = alpha * 256;
    sa = (1 - alpha ) * 256;
    int     uvOffset = outDataHeight*outDataWidth;
    // Here, (w, h) iterate over frame and (sw, sh) iterate over classes
    for (h = 0; h < outDataHeight/2; h++)
    {
        sh = (int32_t)((h << 1) * inDataHeight / outDataHeight);
        uvPtr = frame + uvOffset + (h * outDataWidth);
        rowOffset = sh*inDataWidth;

        for (w = 0; w < outDataWidth; w+=2)
        {
            int32_t index;

            sw = (int32_t)(w * inDataWidth / outDataWidth);
            index = (int32_t)(rowOffset + sw);
            class_id =  classes[index];
            if (class_id < maxClass)
            {
                u_m = colorMap[class_id][1];
                v_m = colorMap[class_id][2];
            }
            else
            {
                u_m = 128;
                v_m = 128;
            }
            u_m = ((*(uvPtr) * a) + (u_m * sa)) >> 8;
            v_m = ((*(uvPtr+1) * a) + (v_m * sa)) >> 8;
            *((uint16_t*)uvPtr) = (v_m << 8) | u_m;
            uvPtr += 2;
        }
    }
    return frame;
}

void *PostprocessSemanticSegmentation::operator()(void             *frameData,
                                                  VecDlTensorPtr   &results)
{
    /* Even though a vector of variants is passed only the first
     * entry is valid.
     */
    auto *buff = results[0];
    void *ret  = frameData;

    if (buff->type == DlInferType_Int8)
    {
        ret = INVOKE_BLEND_LOGIC(int8_t);
    }
    else if (buff->type == DlInferType_UInt8)
    {
        ret = INVOKE_BLEND_LOGIC(uint8_t);
    }
    else if (buff->type == DlInferType_Int16)
    {
        ret = INVOKE_BLEND_LOGIC(int16_t);
    }
    else if (buff->type == DlInferType_UInt16)
    {
        ret = INVOKE_BLEND_LOGIC(uint16_t);
    }
    else if (buff->type == DlInferType_Int32)
    {
        ret = INVOKE_BLEND_LOGIC(int32_t);
    }
    else if (buff->type == DlInferType_UInt32)
    {
        ret = INVOKE_BLEND_LOGIC(uint32_t);
    }
    else if (buff->type == DlInferType_Int64)
    {
        ret = INVOKE_BLEND_LOGIC(int64_t);
    }
    else if (buff->type == DlInferType_Float32)
    {
        ret = INVOKE_BLEND_LOGIC(float);
    }

    return ret;
}

PostprocessSemanticSegmentation::~PostprocessSemanticSegmentation()
{
    for(int i = 0; i < mMaxColorClass; ++i)
    {
        delete mYUVColorMap[i];
    }
    delete mYUVColorMap;
}

} // namespace ti::post_process
