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
#include <ti_post_process_utils.h>

namespace ti::post_process
{

void getColor(YUVColor* color, uint8_t R, uint8_t G, uint8_t B)
{
    color->Y = RGB2Y(R,G,B);
    color->U = RGB2U(R,G,B);
    color->V = RGB2V(R,G,B);
}

void drawPixel(Image*       img,
               int32_t      drawX,
               int32_t      drawY,
               YUVColor*    color)
{   
    uint8_t* YRowPtr = img->yRowAddr + (drawY * img->width);
    uint8_t* UVRowPtr = img->uvRowAddr + (drawY/2 * img->width);
    *(YRowPtr + drawX )  = color->Y;
    *((uint16_t*)(UVRowPtr) + (drawX>>1)) = (color->V << 8) | color->U;
}

void fillRegion(Image*      img,
                int32_t     startX,
                int32_t     startY,
                int32_t     width,
                int32_t     height,
                YUVColor*   color)
{
    if (startX >= img->width)
    {
        return;
    }
    if (startY >= img->height)
    {
        return;
    }
    if (startX < 0)
    {
        startX = 0;
    }
    if (startY < 0)
    {
        startY = 0;
    }
    if ((startX + width) > img->width)
    {
        width = img->width - startX;
    }
    if ((startY + height) > img->height)
    {
        height = img->height - startY;
    }

    uint8_t* YRowPtr;
    uint8_t* UVRowPtr;

    int startXUV = startX >> 1;
    int endXUV = startXUV + (width >> 1);
    
    for(int row = startY; row < startY + height; row++)
    {
        YRowPtr = img->yRowAddr + (row * img->width);
        UVRowPtr = img->uvRowAddr + ((row >> 1) * img->width);
        memset(YRowPtr + startX, color->Y, width);
        for(int col = startXUV; col < endXUV; col++)
        {
            *((uint16_t*)(UVRowPtr) + col) = (color->V << 8) | color->U;
        }
    }
}

void drawLine(Image*    img,
              int32_t   drawX1,
              int32_t   drawY1,
              int32_t   drawX2,
              int32_t   drawY2,
              YUVColor* color,
              int32_t   thickness)
{
    // Breshnam's line drawing algorithm
    if (drawX1 >= img->width)
    {
        drawX1 = img->width - 1;
    }
    if (drawX2 >= img->width)
    {
        drawX2 = img->width - 1;
    }
    if (drawY1 >= img->height)
    {
        drawY1 = img->height - 1;
    }
    if (drawY2 >= img->height)
    {
        drawY2 = img->height - 1;
    }

    int32_t i,k,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;
    
    dx      = drawX2 - drawX1;      /* the horizontal distance of the line */
    dy      = drawY2 - drawY1;      /* the vertical distance of the line */
    dxabs   = ABSOLUTE(dx);
    dyabs   = ABSOLUTE(dy);
    sdx     = SIGN(dx);
    sdy     = SIGN(dy);
    x       = dyabs >> 1;
    y       = dxabs >> 1;
    px      = drawX1;
    py      = drawY1;

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        for(k = (-thickness/2); k < (thickness/2); k++)
        {
            drawPixel(img, px, py+k, color);
        }
        for(i = 0; i < dxabs; i++)
        {
            y += dyabs;
            if (y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            for(k = (-thickness/2); k < (thickness/2); k++)
            {
                drawPixel(img, px, py+k, color);
            }
        }
    }
    
    else /* the line is more vertical than horizontal */
    {
        for(k = (-thickness/2); k < (thickness/2); k++)
        {
            drawPixel(img, px+k, py, color);
        }
        for(i = 0;i < dyabs; i++)
        {
            x += dxabs;
            if (x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            for(k = (-thickness/2); k < (thickness/2); k++)
            {
               drawPixel(img, px+k, py, color);
            }
        }
    }
}

void drawHorizontalLine(Image*      img,
                        int32_t     startX,
                        int32_t     startY,
                        int32_t     width,
                        YUVColor*   color,
                        int32_t     thickness)
{
    fillRegion(img,startX,startY,width,thickness,color);
}

void drawVerticalLine(Image*    img,
                      int32_t   startX,
                      int32_t   startY,
                      int32_t   height,
                      YUVColor* color,
                      int32_t   thickness)
{
    fillRegion(img,startX,startY,thickness,height,color);
}

void drawRect(Image*    img,
              int32_t   startX,
              int32_t   startY,
              int32_t   width,
              int32_t   height,
              YUVColor* color,
              int32_t   thickness)
{
    if (thickness <= 0)
    {
        fillRegion(img, startX, startY, width, height, color);
    }
    else
    {
        drawHorizontalLine(img, startX, startY, width, color, thickness);
        drawHorizontalLine(img, startX, startY+height, width+thickness, color, thickness);
        drawVerticalLine(img, startX, startY, height, color, thickness);
        drawVerticalLine(img, startX+width, startY, height, color, thickness);
    }
}

void drawCircle(Image*      img,
                int32_t     xc,
                int32_t     yc,
                int32_t     radius,
                YUVColor*   color,
                int32_t     thickness)
{
    // Mid-Point Circle Drawing algorithm
    int32_t outerRadius,innerRadius;

    if (thickness <= 0)
    {
        outerRadius = radius;
        innerRadius = 0;
    }

    else
    {
        outerRadius = radius + (thickness >> 1);
        innerRadius = radius - (thickness >> 1);
    }

    int32_t xo = outerRadius;
    int32_t xi = innerRadius;
    int32_t y = 0;

    int32_t erro = 1 - xo;
    int32_t erri = 1 - xi;
    int32_t wd;

    while(xo >= y)
    {
        wd = xo-xi+1;
        drawHorizontalLine(img, xc + xi, yc + y, wd, color, 1);
        drawVerticalLine(img, xc + y, yc + xi, wd, color, 1);
        drawHorizontalLine(img, xc - xo, yc + y, wd, color, 1);
        drawVerticalLine(img, xc - y, yc + xi, wd, color, 1);
        drawHorizontalLine(img, xc - xo, yc - y, wd, color, 1);
        drawVerticalLine(img, xc - y, yc - xo, wd, color, 1);
        drawHorizontalLine(img, xc + xi, yc - y, wd, color, 1);
        drawVerticalLine(img, xc + y, yc - xo, wd, color, 1);
        y++;

        if (erro < 0)
        {
            erro += 2 * y + 1;
        } else
        {
            xo--;
            erro += 2 * (y - xo + 1);
        }

        if (y > innerRadius)
        {
            xi = y;
        }
        else
        {
            if (erri < 0)
            {
                erri += 2 * y + 1;
            }
            else
            {
                xi--;
                erri += 2 * (y - xi + 1);
            }
        }
    }
}

void drawText(Image*        img,
              const char*   text,
              int32_t       topX,
              int32_t       topY,
              FontProperty* fontProp,
              YUVColor*     color)
{   
    if (topX >= img->width)
    {
        return;
    }
    if (topY >= img->height)
    {
        return;
    }
    if (topX < 0)
    {
        topX = 0;
        text = text + int(floor(-topX/fontProp->width) + 1);
    }
    if (topY < 0)
    {
        topY = 0;
    }
    else if (topY + fontProp->height >= img->height)
    {
        topY = img->height - fontProp->height - 1;
    }

    int x = topX;
    int i,j;
    int numChar = floor((img->width - topX) / fontProp->width);
    int totalChar = strlen(text);
    int stopIndex = totalChar;
    int textCount = 0;
    int offset = ceil((float)fontProp->width*(float)fontProp->height/32.0);

    if (numChar < totalChar)
    {
        stopIndex = numChar;
    }
    while(textCount++ < stopIndex)
    {
        int ascii = *(text++) - 33;
        if (ascii >= 0) //Skip Character below ascii 33
        {
            uint32_t *fontAddr = fontProp->addr + (ascii*offset);
            uint32_t fontHexVal = *fontAddr;
            uint8_t  bitCount = 0;
            int      count = 0;
            for (i = 0; i < fontProp->height; i++)
            {
                for (j = 0; j < fontProp->width; j++ )
                {
                    if (bitCount == 32)
                    {
                        fontHexVal = *(++fontAddr);
                        bitCount = 0;
                    }
                    if (fontHexVal >> (bitCount++) & 1)
                    {
                        drawPixel(img, x+j, topY+i, color);
                    }
                    count++;
                }
            }
        }
        x = x + fontProp->width;
    }
}

void blendImage(Image*  imgSrc,
                Image*  imgDest,
                float   alpha,
                float   beta,
                float   gamma
                )
{   
    int imgWidth = imgSrc->width;
    int imgHeight = imgSrc->height*1.5;
    int extraWidth = imgWidth % 8;

    int i,j,k;
    uint8_t* srcRowPtr;
    uint8_t* destRowPtr;
    
    for (i = 0; i < imgHeight; i++)
    {
        srcRowPtr  = imgSrc->yRowAddr + (i * imgSrc->width); 
        destRowPtr = imgDest->yRowAddr + (i * imgSrc->width);
        
        for (j = 0; j < imgWidth; j+=8)
        {
            // Auto Vectorized Hopefully
            srcRowPtr[j]   = alpha * srcRowPtr[j]   + beta * destRowPtr[j]   + gamma;
            srcRowPtr[j+1] = alpha * srcRowPtr[j+1] + beta * destRowPtr[j+1] + gamma;
            srcRowPtr[j+2] = alpha * srcRowPtr[j+2] + beta * destRowPtr[j+2] + gamma;
            srcRowPtr[j+3] = alpha * srcRowPtr[j+3] + beta * destRowPtr[j+3] + gamma;
            srcRowPtr[j+4] = alpha * srcRowPtr[j+4] + beta * destRowPtr[j+4] + gamma;
            srcRowPtr[j+5] = alpha * srcRowPtr[j+5] + beta * destRowPtr[j+5] + gamma;
            srcRowPtr[j+6] = alpha * srcRowPtr[j+6] + beta * destRowPtr[j+6] + gamma;
            srcRowPtr[j+7] = alpha * srcRowPtr[j+7] + beta * destRowPtr[j+7] + gamma;
        }

        for (k = 0; k < extraWidth; k++)
        {
            srcRowPtr[j] = alpha * srcRowPtr[j] + beta * destRowPtr[j] + gamma;
            j++;
        }
    }
}

void BarGraph::initGraph(Image*        img,
                         int32_t       topX,
                         int32_t       topY,
                         int32_t       width,
                         int32_t       height,
                         int32_t       maxValue,
                         const char*   title,
                         const char*   valueUnit,
                         FontProperty* titleFontProp,
                         FontProperty* valueFontProp,
                         YUVColor*     textColor,
                         YUVColor*     fillColor,
                         YUVColor*     bgColor
                        )
{
    if (width & 1)
    {
        width = width - 1;
    }
    if (width <= 0 || height <= 0)
    {
        return;
    }
    if (topX >= img->width)
    {
        return;
    }
    if (topY >= img->height)
    {
        return;
    }
    if (topX < 0)
    {
       return;
    }
    if (topY < 0)
    {
        return;
    }
    m_img = img;
    m_topX = topX;
    m_topY = topY;
    m_graphTopX = topX;
    m_graphTopY = topY + valueFontProp->height + 1;
    m_valueFontProp = valueFontProp;
    m_textColor = textColor;
    m_fillColor = fillColor;
    m_width = width;
    m_height = height;
    m_maxValue = maxValue;
    m_heightPerUnit = (float)height/(float)maxValue;
    m_valueUnit = valueUnit;

    drawRect(m_img, m_graphTopX, m_graphTopY, m_width, m_height, bgColor, -1);

    int totalFontWidth = 0;
    uint8_t counter = 0;
    while (*(title+counter))
    {
        totalFontWidth += titleFontProp->width;
        counter++;
    }
    int titleX = (m_graphTopX + (m_width/2)) - (totalFontWidth/2);
    int titleY = m_graphTopY + m_height + 1;
    drawText(m_img,title,titleX,titleY,titleFontProp,m_textColor);
}

void BarGraph::update(int32_t value)
{
    char buffer[20];
    sprintf(buffer,"%d%s",value,m_valueUnit);

    int totalFontWidth = 0;
    uint8_t counter = 0;
    while (*(buffer+counter))
    {
        totalFontWidth += m_valueFontProp->width;
        counter++;
    }

    int bufferX = (m_graphTopX + (m_width/2)) - (totalFontWidth/2);
    drawText(m_img,buffer,bufferX,m_topY,m_valueFontProp,m_textColor);

    if (value <= 0)
    {
        return;
    }
    if (value > m_maxValue)
    {
        value = m_maxValue;
    }

    int fillHeight = value * m_heightPerUnit;
    int fillY = m_graphTopY + m_height - fillHeight;

    drawRect(m_img, m_graphTopX, fillY, m_width, fillHeight, m_fillColor, -1);
}

} // namespace ti::post_process