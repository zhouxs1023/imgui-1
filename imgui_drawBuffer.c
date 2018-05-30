#include "imgui.h"

inline void StartTriangleStrip(IMContext *im, IMV3 pos, float thickness, IMRGBA color)
{
    if (im->drawBuffer.triStripVertCount >= MAX_TRISTRIP_VERT_COUNT - 4)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.triStripVerts[im->drawBuffer.triStripVertCount];
    im->drawBuffer.triStripVertCount += 4;

    // duplicate vert to make a degenerate triangle so there can be a split in the triangle strip
    memcpy(vert, vert - 1, sizeof(PosScissorColor));
    vert++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    vert->x = (int16_t)pos.x;
    vert->y = (int16_t)(pos.y + thickness * 0.5f);
    vert->z = (int16_t)pos.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;

    // degenerate tri
    memcpy(vert, vert - 1, sizeof(PosScissorColor));
    vert++;

    vert->x = (int16_t)pos.x;
    vert->y = (int16_t)(pos.y - thickness * 0.5f);
    vert->z = (int16_t)pos.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;
}

inline void PushTriangleStrip(IMContext *im, IMV3 pos, float thickness, IMRGBA color)
{
    if (im->drawBuffer.triStripVertCount >= MAX_TRISTRIP_VERT_COUNT - 2)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.triStripVerts[im->drawBuffer.triStripVertCount];
    im->drawBuffer.triStripVertCount += 2;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    vert->x = (int16_t)pos.x;
    vert->y = (int16_t)(pos.y + thickness * 0.5f);
    vert->z = (int16_t)pos.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;

    vert->x = (int16_t)pos.x;
    vert->y = (int16_t)(pos.y - thickness * 0.5f);
    vert->z = (int16_t)pos.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;
}

//inline void PushTriangle(IMContext *im, IMV3 pos0, IMV3 pos1, IMV3 pos2, IMRGBA color)
//{
//}

inline void PushTriangle(IMContext *im, IMV3 pos0, IMV3 pos1, IMV3 pos2, IMRGBA color)
{
    if (im->drawBuffer.triCount >= MAX_TRIANGLE_COUNT)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.triVerts[im->drawBuffer.triCount * 3];
    im->drawBuffer.triCount++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    vert->x = (int16_t)pos0.x;
    vert->y = (int16_t)pos0.y;
    vert->z = (int16_t)pos0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;

    vert->x = (int16_t)pos1.x;
    vert->y = (int16_t)pos1.y;
    vert->z = (int16_t)pos1.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
    vert++;

    vert->x = (int16_t)pos2.x;
    vert->y = (int16_t)pos2.y;
    vert->z = (int16_t)pos2.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = color.packedColor;
}

inline void PushRect_SimpleTris(IMContext *im, IMV2 pos, IMV2 dim, IMV2 thickness, float depth, IMRGBA centerColor, IMRGBA edgeColor)
{
    if (im->drawBuffer.triCount >= MAX_TRIANGLE_COUNT + 4)
    {
        return;
    }

    /* 
    verts:
    0       1 <- outer
      0   1   <- inner

      2   3
    2       3

    */

    IMV3 outer0;
    outer0.x = pos.x;
    outer0.y = pos.y;
    outer0.z = depth;

    IMV3 outer1;
    outer1.x = (pos.x + dim.x);
    outer1.y = pos.y;
    outer1.z = depth;

    IMV3 outer2;
    outer2.x = pos.x;
    outer2.y = (pos.y + dim.y);
    outer2.z = depth;

    IMV3 outer3;
    outer3.x = outer1.x;
    outer3.y = outer2.y;
    outer3.z = depth;

    IMV3 inner0;
    inner0.x = pos.x + thickness.x;
    inner0.y = pos.y + thickness.y;
    inner0.z = depth;

    IMV3 inner1;
    inner1.x = (pos.x + dim.x) - thickness.x;
    inner1.y = pos.y + thickness.y;
    inner1.z = depth;

    IMV3 inner2;
    inner2.x = pos.x + thickness.x;
    inner2.y = (pos.y + dim.y) - thickness.y;
    inner2.z = depth;

    IMV3 inner3;
    inner3.x = inner1.x;
    inner3.y = inner2.y;
    inner3.z = depth;

    // TODO: use linestrips
    // top edge quad
    PushTriangle(im, outer0, outer1, inner0, edgeColor);
    PushTriangle(im, outer1, inner0, inner1, edgeColor);
    // left edge quad
    PushTriangle(im, outer0, inner0, outer2, edgeColor);
    PushTriangle(im, inner0, inner2, outer2, edgeColor);
    // right edge quad
    PushTriangle(im, inner1, outer1, outer3, edgeColor);
    PushTriangle(im, inner1, outer3, inner3, edgeColor);
    // bottom edge quad
    PushTriangle(im, inner2, inner3, outer3, edgeColor);
    PushTriangle(im, inner2, outer3, outer2, edgeColor);
    // center quad
    PushTriangle(im, inner0, inner1, inner2, centerColor);
    PushTriangle(im, inner1, inner3, inner2, centerColor);

    //TODO instanced rectangles
}

#if 0
// simple degenerate tri strip method
inline void PushRect(IMContext *im, IMV2 pos, IMV2 dim, IMV2 thickness, float depth, IMRGBA centerColor, IMRGBA edgeColor)
{
    /* 
    verts:
    0       1 <- outer
      0   1   <- inner

      2   3
    2       3
    */

    IMV3 outer0;
    outer0.x = pos.x;
    outer0.y = pos.y;
    outer0.z = depth;

    IMV3 outer1;
    outer1.x = (pos.x + dim.x);
    outer1.y = pos.y;
    outer1.z = depth;

    IMV3 outer2;
    outer2.x = pos.x;
    outer2.y = (pos.y + dim.y);
    outer2.z = depth;

    IMV3 outer3;
    outer3.x = outer1.x;
    outer3.y = outer2.y;
    outer3.z = depth;

    IMV3 inner0;
    inner0.x = pos.x + thickness.x;
    inner0.y = pos.y + thickness.y;
    inner0.z = depth;

    IMV3 inner1;
    inner1.x = (pos.x + dim.x) - thickness.x;
    inner1.y = pos.y + thickness.y;
    inner1.z = depth;

    IMV3 inner2;
    inner2.x = pos.x + thickness.x;
    inner2.y = (pos.y + dim.y) - thickness.y;
    inner2.z = depth;

    IMV3 inner3;
    inner3.x = inner1.x;
    inner3.y = inner2.y;
    inner3.z = depth;

    // triangle strip for the border,  2 degenerate verts + 10 outline verts (first two first are duplicated to finish the loop) + 2 more degenerate verts + 4 center verts
    if (im->drawBuffer.triStripVertCount >= MAX_TRISTRIP_VERT_COUNT - 18)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.triStripVerts[im->drawBuffer.triStripVertCount];
    im->drawBuffer.triStripVertCount += 18;

    // make first tri degenerate
    memcpy(vert, vert - 1, sizeof(PosScissorColor));
    vert++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    vert->x = (int16_t)outer0.x;
    vert->y = (int16_t)outer0.y;
    vert->z = (int16_t)outer0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // make second tri degenerate, now we're doing the strip starting with outer0
    memcpy(vert, vert - 1, sizeof(PosScissorColor));
    vert++;

    vert->x = (int16_t)inner0.x;
    vert->y = (int16_t)inner0.y;
    vert->z = (int16_t)inner0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)outer1.x;
    vert->y = (int16_t)outer1.y;
    vert->z = (int16_t)outer1.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)inner1.x;
    vert->y = (int16_t)inner1.y;
    vert->z = (int16_t)inner1.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)outer3.x;
    vert->y = (int16_t)outer3.y;
    vert->z = (int16_t)outer3.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)inner3.x;
    vert->y = (int16_t)inner3.y;
    vert->z = (int16_t)inner3.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)outer2.x;
    vert->y = (int16_t)outer2.y;
    vert->z = (int16_t)outer2.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)inner2.x;
    vert->y = (int16_t)inner2.y;
    vert->z = (int16_t)inner2.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)outer0.x;
    vert->y = (int16_t)outer0.y;
    vert->z = (int16_t)outer0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    vert->x = (int16_t)inner0.x;
    vert->y = (int16_t)inner0.y;
    vert->z = (int16_t)inner0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // two degenerate verts to seperate center quad
    memcpy(vert, vert - 1, sizeof(*vert));
    vert++;

    vert->x = (int16_t)inner0.x;
    vert->y = (int16_t)inner0.y;
    vert->z = (int16_t)inner0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;
    // end of degenerate verts

    vert->x = (int16_t)inner0.x;
    vert->y = (int16_t)inner0.y;
    vert->z = (int16_t)inner0.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    vert->x = (int16_t)inner1.x;
    vert->y = (int16_t)inner1.y;
    vert->z = (int16_t)inner1.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    vert->x = (int16_t)inner2.x;
    vert->y = (int16_t)inner2.y;
    vert->z = (int16_t)inner2.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    vert->x = (int16_t)inner3.x;
    vert->y = (int16_t)inner3.y;
    vert->z = (int16_t)inner3.z;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    // center quad
    //PushTriangle(im, inner0, inner1, inner2, centerColor);
    //PushTriangle(im, inner1, inner3, inner2, centerColor);

    //TODO instanced rectangles
}
#else
// Indexed triangle strip method
inline void PushRect(IMContext *im, IMV4 rect, IMV2 thickness, float depth, IMRGBA centerColor, IMRGBA edgeColor)
{
    // 8 verts for edge, 4 verts for the center that require a seperate color, 1 primitive restart index + 10 Outer strip indexes + 1 primitive restart index + 4 inner quad indexes = 16 indexes
    if (im->drawBuffer.PSCVertCount >= MAX_PSC_VERT_COUNT - 12 ||
        im->drawBuffer.PSCElementCount >= MAX_PSC_ELEMENT_COUNT - 16)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    uint16_t *index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];
    uint16_t e0 = im->drawBuffer.PSCVertCount;
    im->drawBuffer.PSCVertCount += 12;
    im->drawBuffer.PSCElementCount += 16;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    /* 
    vert:
    0            2
        1      3
          8  9
          10 11
        7       5
    6              4
    */

    index[0] = PRIM_RESTART;
    index[1] = e0;
    index[2] = e0 + 1;
    index[3] = e0 + 2;
    index[4] = e0 + 3;
    index[5] = e0 + 4;
    index[6] = e0 + 5;
    index[7] = e0 + 6;
    index[8] = e0 + 7;
    index[9] = e0 + 0;
    index[10] = e0 + 1;
    index[11] = PRIM_RESTART;
    index[12] = e0 + 8;
    index[13] = e0 + 9;
    index[14] = e0 + 10;
    index[15] = e0 + 11;

    // top left
    vert->x = (int16_t)rect.xmin;
    vert->y = (int16_t)rect.ymin;
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // top left inner
    vert->x = (int16_t)(rect.xmin + thickness.x);
    vert->y = (int16_t)(rect.ymin + thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // top right
    vert->x = (int16_t)(rect.xmax);
    vert->y = (int16_t)(rect.ymin);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // top right inner
    vert->x = (int16_t)(rect.xmax - thickness.x);
    vert->y = (int16_t)(rect.ymin + thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // bottom right
    vert->x = (int16_t)(rect.xmax);
    vert->y = (int16_t)(rect.ymax);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // bottom right inner
    vert->x = (int16_t)(rect.xmax - thickness.x);
    vert->y = (int16_t)(rect.ymax - thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // bottom left
    vert->x = (int16_t)(rect.xmin);
    vert->y = (int16_t)(rect.ymax);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // bottom left inner
    vert->x = (int16_t)(rect.xmin + thickness.x);
    vert->y = (int16_t)(rect.ymax - thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = edgeColor.packedColor;
    vert++;

    // Center colored verts
    // top left
    vert->x = (int16_t)(rect.xmin + thickness.x);
    vert->y = (int16_t)(rect.ymin + thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    // top right
    vert->x = (int16_t)(rect.xmax - thickness.x);
    vert->y = (int16_t)(rect.ymin + thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    // bottom left
    vert->x = (int16_t)(rect.xmin + thickness.x);
    vert->y = (int16_t)(rect.ymax - thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;

    // bottom right
    vert->x = (int16_t)(rect.xmax - thickness.x);
    vert->y = (int16_t)(rect.ymax - thickness.y);
    vert->z = (int16_t)depth;
    vert->w = 0;
    vert->x_min = x_min;
    vert->x_max = x_max;
    vert->y_min = y_min;
    vert->y_max = y_max;
    vert->abgr = centerColor.packedColor;
    vert++;
}
#endif

inline IMV2 IMV2UnitNormal(IMV2 v)
{
    float rmag = rsqrt_fast(v.x * v.x + v.y * v.y);
    IMV2 result = {v.y * rmag, -v.x * rmag};
    return result;
}
inline float IMV2Mag(IMV2 v)
{
    float result = sqrtf(v.x * v.x + v.y * v.y);
    return result;
}
inline IMV2 IMV2Unit(IMV2 v)
{
    float rmag = rsqrt_fast(v.x * v.x + v.y * v.y);
    IMV2 result = {v.x * rmag, v.y * rmag};
    return result;
}

void PushSmoothstepSpline(IMContext *im, IMV4 rect, float depth, IMRGBA color, IMV4 range, int pointCount, float *points)
{
    // 2 verts for each pixel in the rect width, 1 primitive restart index, then 2 indexes for vert
    if (pointCount < 3)
    {
        //TODO: should call the linear spline function
        return;
    }
    int vertCount = (int)(2.f * (rect.xmax - rect.xmin));
    int indexCount = 1 + vertCount;
    if (im->drawBuffer.PSCVertCount + vertCount >= MAX_PSC_VERT_COUNT ||
        im->drawBuffer.PSCElementCount + indexCount >= MAX_PSC_ELEMENT_COUNT)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    uint16_t *index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];

    *index = PRIM_RESTART;
    index++;
    im->drawBuffer.PSCElementCount++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    float rangeDimX = range.xmax - range.xmin;
    float rangeDimY = range.ymax - range.ymin;
    float rectDimX = rect.xmax - rect.xmin;
    float rectDimY = rect.ymax - rect.ymin;

    float scaleX = rectDimX / rangeDimX;
    float scaleY = -rectDimY / rangeDimY;

    int16_t depth16 = (int16_t)depth;
    float thickness = 20.f;

    float *p = &points[6];

    IMV2 p0;
    IMV2 p1 = {rect.xmin + (points[0] - range.xmin) * scaleX, rect.ymax + (points[1] - range.ymin) * scaleY};
    IMV2 p2 = {rect.xmin + (points[2] - range.xmin) * scaleX, rect.ymax + (points[3] - range.ymin) * scaleY};
    IMV2 p3 = {rect.xmin + (points[4] - range.xmin) * scaleX, rect.ymax + (points[5] - range.ymin) * scaleY};
    p0.x = p1.x - (p2.x - p1.x);
    p0.y = p1.y - (p2.y - p1.y);

    for (int i = 3; i <= pointCount; i++)
    {
        //TODO: interval should be approx. the xy distance, not just the x distance
        float interval = (p2.x - p1.x);
        float rinterval = 1.f / interval;

        IMV2 leftSlope = IMV2Unit(MakeIMV2(p2.x - p0.x, p2.y - p0.y));
        IMV2 rightSlope = IMV2Unit(MakeIMV2(p3.x - p1.x, p3.y - p1.y));

#if 1
        float dy0 = leftSlope.y * (interval / leftSlope.x);
        float dy1 = rightSlope.y * (interval / rightSlope.x);
#else
        float dy0 = leftSlope.y * IMV2Mag(MakeIMV2(p2.x - p1.x, p2.y - p1.y));
        float dy1 = rightSlope.y * IMV2Mag(MakeIMV2(p2.x - p1.x, p2.y - p1.y));
#endif

#if 1
        PushTriangle(im,
                     MakeIMV3(p1.x, p1.y, 1000.f),
                     MakeIMV3(p1.x, p1.y + 3.f, 1000.f),
                     MakeIMV3(p1.x + interval, p1.y + dy0, 1000.f),
                     color);
        PushTriangle(im,
                     MakeIMV3(p2.x, p2.y, 1000.f),
                     MakeIMV3(p2.x, p2.y + 3.f, 1000.f),
                     MakeIMV3(p2.x - interval, p2.y - dy1, 1000.f),
                     color);
#endif

        IMV2 pnext;
        IMV2 pcurrent = {p1.x, p1.y};
        float t_elapsed = 0.f;
        float t_remaining = 1.f;
        while (t_elapsed < 1.f)
        {
            t_elapsed += rinterval;
            t_remaining -= rinterval;

            float smoothStep = 3.f * t_elapsed * t_elapsed - 2.f * t_elapsed * t_elapsed * t_elapsed;
            pnext.x = p1.x + t_elapsed * interval;
            float lin0 = p1.y + t_elapsed * dy0;
            float lin1 = p2.y - t_remaining * dy1;
            pnext.y = (1.f - smoothStep) * lin0 + (smoothStep)*lin1;

            IMV2 dir;
            dir.x = pnext.x - pcurrent.x;
            dir.y = pnext.y - pcurrent.y;

            IMV2 normal = IMV2UnitNormal(dir);

            pcurrent = pnext;

            float lineWidth = 2.f;
            vert->x = (int16_t)(pcurrent.x + normal.x * lineWidth);
            vert->y = (int16_t)(pcurrent.y + normal.y * lineWidth);
            vert->z = depth16;
            vert->w = 0; // thickness
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;

            vert->x = (int16_t)(pcurrent.x - normal.x * lineWidth);
            vert->y = (int16_t)(pcurrent.y - normal.y * lineWidth);
            vert->z = depth16;
            vert->w = 0; // -thickness
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;
        }

        p0 = p1;
        p1 = p2;
        p2 = p3;
        if (i < pointCount)
        {
            p3.x = rect.xmin + (p[0] - range.xmin) * scaleX;
            p3.y = rect.ymax + (p[1] - range.ymin) * scaleY;
        }
        else
        {
            p3.x = p2.x + (p2.x - p1.x);
            p3.y = p2.y + (p2.y - p1.y);
        }

        p += 2;
    }
}

void PushCubicSpline1(IMContext *im, IMV4 rect, float depth, IMRGBA color, IMV4 range, int pointCount, float *points)
{
    // 2 verts for each pixel in the rect width, 1 primitive restart index, then 2 indexes for vert
    if (pointCount < 3)
    {
        //TODO: should call the linear spline function
        return;
    }
    int vertCount = (int)(2.f * (rect.xmax - rect.xmin));
    int indexCount = 1 + vertCount;
    if (im->drawBuffer.PSCVertCount + vertCount >= MAX_PSC_VERT_COUNT ||
        im->drawBuffer.PSCElementCount + indexCount >= MAX_PSC_ELEMENT_COUNT)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    uint16_t *index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];

    *index = PRIM_RESTART;
    index++;
    im->drawBuffer.PSCElementCount++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    float rangeDimX = range.xmax - range.xmin;
    float rangeDimY = range.ymax - range.ymin;
    float rectDimX = rect.xmax - rect.xmin;
    float rectDimY = rect.ymax - rect.ymin;

    float scaleX = rectDimX / rangeDimX;
    float scaleY = -rectDimY / rangeDimY;

    int16_t depth16 = (int16_t)depth;
    float thickness = 20.f;

    float *p = &points[6];

    IMV2 p0;
    IMV2 p1 = {rect.xmin + (points[0] - range.xmin) * scaleX, rect.ymax + (points[1] - range.ymin) * scaleY};
    IMV2 p2 = {rect.xmin + (points[2] - range.xmin) * scaleX, rect.ymax + (points[3] - range.ymin) * scaleY};
    IMV2 p3 = {rect.xmin + (points[4] - range.xmin) * scaleX, rect.ymax + (points[5] - range.ymin) * scaleY};
    p0.x = p1.x - (p2.x - p1.x);
    p0.y = p1.y - (p2.y - p1.y);

    //PushRect(IMContext *im, IMV4 rect, IMV2 thickness, float depth, IMRGBA centerColor, IMRGBA edgeColor)
    IMV4 pointRect = {p1.x - 5.f, p1.x + 5.f, p1.y - 5.f, p1.y + 5.f};
    IMV2 pointThickness = {1.f, 1.f};
    PushRect(im, pointRect, pointThickness, depth + 100.f, MakeIMRGBA(50, 90, 180, 255), color);
    vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];

    *index++ = PRIM_RESTART;
    im->drawBuffer.PSCElementCount++;

    IMV2 pcurrent = {p0.x, p0.y};
    for (int i = 3; i < pointCount; i++)
    {
        float interval = sqrtf((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
        float rinterval = 2.f / interval;
        float t_elapsed = 0.f;
        float t_remaining = 1.f;
        while (t_elapsed < 1.f)
        {
#if 0
            //pcurrent.x = 0.5f * ((2 * p1.x) +
            //                     (-p0.x + p2.x) * t_elapsed +
            //                     (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t_elapsed * t_elapsed +
            //                     (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t_elapsed * t_elapsed * t_elapsed);
            pcurrent.x = p1.x + t_elapsed * interval;
            pcurrent.y = 0.5f * ((2 * p1.y) +
                                 (-p0.y + p2.y) * t_elapsed +
                                 (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t_elapsed * t_elapsed +
                                 (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t_elapsed * t_elapsed * t_elapsed);

            t_elapsed += rinterval;
            t_remaining -= rinterval;
            IMV2 dir;
            dir.x = -pcurrent.x + p1.x + t_elapsed * interval;
            dir.y = -pcurrent.y + 0.5f * ((2 * p1.y) +
                                          (-p0.y + p2.y) * t_elapsed +
                                          (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t_elapsed * t_elapsed +
                                          (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t_elapsed * t_elapsed * t_elapsed);
#else
            pcurrent.x = 0.5f * ((2 * p1.x) +
                                 (-p0.x + p2.x) * t_elapsed +
                                 (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t_elapsed * t_elapsed +
                                 (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t_elapsed * t_elapsed * t_elapsed);
            pcurrent.y = 0.5f * ((2 * p1.y) +
                                 (-p0.y + p2.y) * t_elapsed +
                                 (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t_elapsed * t_elapsed +
                                 (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t_elapsed * t_elapsed * t_elapsed);

            t_elapsed += rinterval;
            t_remaining -= rinterval;
            IMV2 dir;
            dir.x = -pcurrent.x + 0.5f * ((2 * p1.x) +
                                          (-p0.x + p2.x) * t_elapsed +
                                          (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t_elapsed * t_elapsed +
                                          (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t_elapsed * t_elapsed * t_elapsed);
            dir.y = -pcurrent.y + 0.5f * ((2 * p1.y) +
                                          (-p0.y + p2.y) * t_elapsed +
                                          (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t_elapsed * t_elapsed +
                                          (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t_elapsed * t_elapsed * t_elapsed);
#endif
            IMV2 normal = IMV2UnitNormal(dir);

            vert->x = (int16_t)(pcurrent.x + normal.x * 3.f);
            vert->y = (int16_t)(pcurrent.y + normal.y * 3.f);
            vert->z = depth16;
            vert->w = 0;
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;

            vert->x = (int16_t)(pcurrent.x - normal.x * 3.f);
            vert->y = (int16_t)(pcurrent.y - normal.y * 3.f);
            vert->z = depth16;
            vert->w = 0;
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;

            pcurrent.x += 1.f;
        }

        p0 = p1;
        p1 = p2;
        p2 = p3;
        p3.x = rect.xmin + (p[0] - range.xmin) * scaleX;
        p3.y = rect.ymax + (p[1] - range.ymin) * scaleY;
        /* 
        pointRect.xmin = p1.x - 5.f;
        pointRect.xmax = p1.x + 5.f;
        pointRect.ymin = p1.y - 5.f;
        pointRect.ymax = p1.y + 5.f;
        PushRect(im, pointRect, pointThickness, depth + 100.f, MakeIMRGBA(50, 90, 180, 255), color);
        vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
        index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];
 */
        p += 2;
    }
}

void PushCubicSpline2(IMContext *im, IMV4 rect, float depth, IMRGBA color, IMV4 range, int pointCount, float *points)
{
    // 2 verts for each pixel in the rect width, 1 primitive restart index, then 2 indexes for vert
    if (pointCount < 3)
    {
        //TODO: should call the linear spline function
        return;
    }
    int vertCount = (int)(2.f * (rect.xmax - rect.xmin));
    int indexCount = 1 + vertCount;
    if (im->drawBuffer.PSCVertCount + vertCount >= MAX_PSC_VERT_COUNT ||
        im->drawBuffer.PSCElementCount + indexCount >= MAX_PSC_ELEMENT_COUNT)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    uint16_t *index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];

    *index = PRIM_RESTART;
    index++;
    im->drawBuffer.PSCElementCount++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    float rangeDimX = range.xmax - range.xmin;
    float rangeDimY = range.ymax - range.ymin;
    float rectDimX = rect.xmax - rect.xmin;
    float rectDimY = rect.ymax - rect.ymin;

    float scaleX = rectDimX / rangeDimX;
    float scaleY = -rectDimY / rangeDimY;

    int16_t depth16 = (int16_t)depth;
    float thickness = 20.f;

    float *p = &points[6];

    IMV2 p0;
    IMV2 p1 = {rect.xmin + (points[0] - range.xmin) * scaleX, rect.ymax + (points[1] - range.ymin) * scaleY};
    IMV2 p2 = {rect.xmin + (points[2] - range.xmin) * scaleX, rect.ymax + (points[3] - range.ymin) * scaleY};
    IMV2 p3 = {rect.xmin + (points[4] - range.xmin) * scaleX, rect.ymax + (points[5] - range.ymin) * scaleY};
    p0.x = p1.x - (p2.x - p1.x);
    p0.y = p1.y - (p2.y - p1.y);

    for (int i = 3; i <= pointCount; i++)
    {
        /* By @paniq - https://www.shadertoy.com/view/MdcBzB
    m[0] = p[1] + (p[2] - p[0]) / 6.0;
    m[1] = p[2] - (p[3] - p[1]) / 6.0;
    
    float rx = (1.0 - x);
    float rxx = rx*rx;
    float xx = x*x;
    
    float t0 = rxx*rx;
    float t1 = 3.0*x*rxx;
    float t2 = 3.0*xx*rx;
    float t3 = xx*x;
    
    return t0 * p[1] + t1 * m[0] + t2 * m[1] + t3 * p[2];
 */
        float interval = (p2.x - p1.x);
        float rinterval = 1.f / interval;

        IMV2 m0 = {p1.x + (p2.x - p0.x) / 6.f, p1.y + (p2.y - p0.y) / 6.f};
        IMV2 m1 = {p2.x - (p3.x - p1.x) / 6.f, p2.y - (p3.y - p1.y) / 6.f};

#if 0
        PushTriangle(im,
                     MakeIMV3(p1.x, p1.y, 1000.f),
                     MakeIMV3(p1.x, p1.y + 3.f, 1000.f),
                     MakeIMV3(p1.x + interval, p1.y + dy0, 1000.f),
                     color);
        PushTriangle(im,
                     MakeIMV3(p2.x, p2.y, 1000.f),
                     MakeIMV3(p2.x, p2.y + 3.f, 1000.f),
                     MakeIMV3(p2.x - interval, p2.y - dy1, 1000.f),
                     color);
#endif

        IMV2 pnext;
        IMV2 pcurrent = {p1.x, p1.y};
        float t_elapsed = 0.f;
        float t_remaining = 1.f;
        while (t_elapsed < 1.f)
        {
            t_elapsed += rinterval;
            t_remaining -= rinterval;

            float t_remaining_2 = t_remaining * t_remaining;
            float t_elapsed_2 = t_elapsed * t_elapsed;

            float t0 = t_remaining_2 * t_remaining;
            float t1 = 3.f * t_elapsed * t_remaining_2;
            float t2 = 3.f * t_elapsed_2 * t_remaining;
            float t3 = t_elapsed_2 * t_elapsed;

            //pnext.x = p1.x + t_elapsed * interval;
            pnext.x = t0 * p1.x + t1 * m0.x + t2 * m1.x + t3 * p2.x;
            pnext.y = t0 * p1.y + t1 * m0.y + t2 * m1.y + t3 * p2.y;

            IMV2 dir;
            dir.x = pnext.x - pcurrent.x;
            dir.y = pnext.y - pcurrent.y;

            IMV2 normal = IMV2UnitNormal(dir);

            pcurrent = pnext;

            float lineWidth = 2.f;
            vert->x = (int16_t)(pcurrent.x + normal.x * lineWidth);
            vert->y = (int16_t)(pcurrent.y + normal.y * lineWidth);
            vert->z = depth16;
            vert->w = 0; // thickness
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;

            vert->x = (int16_t)(pcurrent.x - normal.x * lineWidth);
            vert->y = (int16_t)(pcurrent.y - normal.y * lineWidth);
            vert->z = depth16;
            vert->w = 0; // -thickness
            vert->x_min = x_min;
            vert->x_max = x_max;
            vert->y_min = y_min;
            vert->y_max = y_max;
            vert->abgr = color.packedColor;
            vert++;
            *index++ = im->drawBuffer.PSCVertCount++;
            im->drawBuffer.PSCElementCount++;
        }

        p0 = p1;
        p1 = p2;
        p2 = p3;
        if (i < pointCount)
        {
            p3.x = rect.xmin + (p[0] - range.xmin) * scaleX;
            p3.y = rect.ymax + (p[1] - range.ymin) * scaleY;
        }
        else
        {
            p3.x = p2.x + (p2.x - p1.x);
            p3.y = p2.y + (p2.y - p1.y);
        }

        p += 2;
    }
}

void PushSpline(IMContext *im, IMV4 rect, float depth, IMRGBA color, IMV4 range, int pointCount, float *points)
{
    // 2 verts for each point, 1 primitive restart index, then 2 indexes for each point
    int vertCount = 2 * pointCount;
    int indexCount = 1 + 2 * pointCount;
    if (im->drawBuffer.PSCVertCount + vertCount >= MAX_PSC_VERT_COUNT ||
        im->drawBuffer.PSCElementCount + indexCount >= MAX_PSC_ELEMENT_COUNT)
    {
        return;
    }

    PosScissorColor *vert = &im->drawBuffer.PSCVerts[im->drawBuffer.PSCVertCount];
    uint16_t *index = &im->drawBuffer.PSCElements[im->drawBuffer.PSCElementCount];
    uint16_t e0 = im->drawBuffer.PSCVertCount;
    im->drawBuffer.PSCVertCount += vertCount;
    im->drawBuffer.PSCElementCount += indexCount;

    *index = PRIM_RESTART;
    index++;

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    float rangeDimX = range.xmax - range.xmin;
    float rangeDimY = range.ymax - range.ymin;
    float rectDimX = rect.xmax - rect.xmin;
    float rectDimY = rect.ymax - rect.ymin;

    float scaleX = rectDimX / rangeDimX;
    float scaleY = -rectDimY / rangeDimY;

    int16_t depth16 = (int16_t)depth;
    float thickness = 20.f;

    float *p = points;
    for (int i = 0; i < pointCount; i++)
    {
        IMV2 dir;
        if (i + 1 < pointCount)
        {
            dir.x = (p[2] - p[0]) * scaleX;
            dir.y = (p[3] - p[1]) * scaleY;
        }
        else
        {
            dir.x = (p[0] - *(p - 2)) * scaleX;
            dir.y = (p[1] - *(p - 1)) * scaleY;
        }

        IMV2 normal = IMV2UnitNormal(dir);

        float screenX = rect.xmin + (p[0] - range.xmin) * scaleX;
        float screenY = rect.ymax + (p[1] - range.ymin) * scaleY;

        vert->x = (int16_t)(screenX);
        vert->y = (int16_t)(screenY);
        vert->z = depth16;
        vert->w = 0;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->abgr = color.packedColor;
        vert++;

        vert->x = (int16_t)(screenX + normal.x * thickness);
        vert->y = (int16_t)(screenY + normal.y * thickness);
        vert->z = depth16;
        vert->w = 0;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->abgr = color.packedColor;
        vert++;

        *index++ = e0++;
        *index++ = e0++;

        p += 2;
    }
}

void PushStr(IMContext *im, IMV3 pos, FontAlign alignV, FontAlign alignH, int fontIndex, IMRGBA color, float size, char *s)
{
    assert(size > 0.f);
    assert(fontIndex < MAX_FONT_COUNT);

    FontStore *font = &im->fonts[fontIndex];
    int sizeIndex = IM_CLAMP((int)size, 1, font->fontSizeCount) - 1;
    stbtt_packedchar *fontData = font->sizes[sizeIndex].fontData;
    float actualSize = (float)(sizeIndex + 1);

    int16_t x_min = (int16_t)im->scissorBuffer[im->scissorTop].xmin;
    int16_t x_max = (int16_t)im->scissorBuffer[im->scissorTop].xmax;
    int16_t y_min = (int16_t)im->scissorBuffer[im->scissorTop].ymin;
    int16_t y_max = (int16_t)im->scissorBuffer[im->scissorTop].ymax;

    uint32_t packedColor = color.packedColor;

    GlyphVert *vert = &im->drawBuffer.glyphVerts[im->drawBuffer.glyphCount * 6];
    float stringWidth = 0.f;
    int glyphCount = 0;
    float uvScale = 1.f / 512.f;
    while (*s > 31 && *s < 128)
    {
        stbtt_packedchar *pack = fontData + (*s - 32);
        // top left
        vert->x = pos.x + pack->xoff;
        vert->y = pos.y + pack->yoff;
        vert->z = pos.z;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->u = (float)pack->x0 * uvScale;
        vert->v = (float)pack->y0 * uvScale;
        vert->abgr = packedColor;
        vert++;
        // top right
        vert->x = pos.x + pack->xoff2;
        vert->y = pos.y + pack->yoff;
        vert->z = pos.z;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->u = (float)pack->x1 * uvScale;
        vert->v = (float)pack->y0 * uvScale;
        vert->abgr = packedColor;
        vert++;
        // bottom left
        vert->x = pos.x + pack->xoff;
        vert->y = pos.y + pack->yoff2;
        vert->z = pos.z;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->u = (float)pack->x0 * uvScale;
        vert->v = (float)pack->y1 * uvScale;
        vert->abgr = packedColor;
        vert++;
        // bottom left again
        memcpy(vert, vert - 1, sizeof(GlyphVert));
        vert++;
        // top right again
        memcpy(vert, vert - 3, sizeof(GlyphVert));
        vert++;
        // bottom right
        vert->x = pos.x + pack->xoff2;
        vert->y = pos.y + pack->yoff2;
        vert->z = pos.z;
        vert->x_min = x_min;
        vert->x_max = x_max;
        vert->y_min = y_min;
        vert->y_max = y_max;
        vert->u = (float)pack->x1 * uvScale;
        vert->v = (float)pack->y1 * uvScale;
        vert->abgr = packedColor;
        vert++;

        pos.x += pack->xadvance;
        stringWidth += pack->xadvance;
        s++;
        glyphCount++;
    }

    float scaleV = 1.f;
    float shiftV = 1.f;
    float scaleH = 1.f;
    float shiftH = 1.f;

    if (alignV == Font_center)
    {
        shiftV += actualSize * scaleV * 0.3f;
    }
    else if (alignV == Font_top)
    {
        shiftV += actualSize * scaleV * 0.9f;
    }

    if (alignH == Font_center)
    {
        shiftH -= stringWidth * scaleH * 0.5f;
    }
    else if (alignH == Font_right)
    {
        shiftH -= stringWidth * scaleH;
    }

    vert = &im->drawBuffer.glyphVerts[im->drawBuffer.glyphCount * 6];
    for (int i = 0; i < glyphCount * 6; i++)
    {
        vert->x = vert->x * scaleH + shiftH;
        vert->y = vert->y * scaleV + shiftV;
        vert++;
    }

    im->drawBuffer.glyphCount += glyphCount;
}

void ResetDrawBuffer(IMDrawBuffer *drawBuffer)
{
    drawBuffer->triCount = 0;
    // triStrip buffer needs to start out degenerate
    drawBuffer->triStripVertCount = 1;
    PosScissorColor *vert = &drawBuffer->triStripVerts[0];
    vert->x = 0;
    vert->y = 0;
    vert->z = 0;
    vert++;
    //vert->x = 0;
    //vert->y = 0;
    //vert->z = 0;
    //vert++;
    drawBuffer->PSCElementCount = 0;
    drawBuffer->PSCVertCount = 0;
    drawBuffer->glyphCount = 0;
}