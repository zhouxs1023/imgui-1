#ifndef IMGUI_H
#define IMGUI_H

#include "stdint.h"
#include "intrin.h"
#include "assert.h"
#include "stb_truetype.h"

// define IM_DATA_SIZE to another size before including this header if necessary
#ifndef IM_DATA_SIZE
#define IM_DATA_SIZE 512
#endif

#define MAX_TRIANGLE_COUNT 16 * 1024
#define MAX_TRISTRIP_VERT_COUNT 128 * 1024
#define MAX_GLYPH_INSTANCES 16 * 1024
#define MAX_RECT_INSTANCES 4096
#define MAX_DIAGRAM_LINES 1024
#define NUM_FONT_SIZES 25
#define NUM_FONT_GLYPHS 96
#define MAX_FONT_COUNT 8
#define IM_SCISSOR_STACK_COUNT 1024

#define PRIM_RESTART 65535

#define IM_CLAMP(X, A, B) ((X) <= (A) ? (A) : (X) >= (B) ? (B) : (X))
#define IM_MIN(A, B) ((A) < (B) ? (A) : (B))
#define IM_MAX(A, B) ((A) > (B) ? (A) : (B))

///////////////////////////////////////////////////////////////////
// Basic types
/* 
typedef union {
    struct
    {
        float xmin, ymin, xmax, ymax;
    };
    float bounds[4];
} IMBounds;
*/

#define IMBounds IMV4

typedef union {
    struct
    {
        float x, y;
    };
    struct
    {
        float w, h;
    };
    float e[2];
} IMV2;

typedef union {
    struct
    {
        float x, y, z;
    };
    struct
    {
        float w, h, d;
    };
    float e[3];
} IMV3;

typedef union {
    struct
    {
        int x, y;
    };
    struct
    {
        int w, h;
    };
    int e[2];
} IMV2i;

typedef union {
    struct
    {
        float x, y, z, w;
    };
    struct
    {
        float xmin, xmax, ymin, ymax;
    };
    struct
    {
        float r, g, b, a;
    };
    float e[4];
} IMV4;

typedef union {
    struct
    {
        uint8_t r, g, b, a;
    };
    uint32_t packedColor; // ABGR
} IMRGBA;

///////////////////////////////////////////////////////////////
// types used to execute actions

typedef struct
{
    char *string;
    int size;
} IMStringInfo;

typedef struct
{
    float fmin;
    float fmax;
    char *string;
    char buff[16];
} IMFloatInfo;

typedef struct
{
    void *funcAddr;
    void *funcArg;
} IMFunctionInfo;

typedef struct
{
    void *funcAddr;
    uint8_t data[IM_DATA_SIZE - sizeof(void *)];
} IMFunctionCustom;

typedef enum
{
    IM_container,
    IM_toggle,
    IM_intField,
    IM_floatField,
    IM_stringField,
    IM_funcOnPress,
    IM_funcOnPressCustom,
    IM_funcOnRelease,
    IM_funcOnReleaseCustom,
    IM_window,
    IM_resizeHandle,
    IM_count,
} IMDefaultType;

typedef struct
{
    int pressStarted;
    int pressEnded;
    int down;
} IMKeyState;

typedef struct
{
    IMKeyState esc;
    IMKeyState enter;

    IMKeyState m1;
    IMKeyState m2;
    IMKeyState m3;
    int32_t mx, my;
    int32_t mdx, mdy;
    int32_t scrollx, scrolly;

    int32_t numBackspace;
    int32_t numDelete;
    char text[512];
} IMInputState;

//////////////////////////////////////////////////////////////
// types used for layouting

typedef enum
{
    IM_free,
    IM_top,
    IM_bottom,
    IM_left,
    IM_right,
    IM_full,
} IMAnchoring;

typedef struct IMPanel
{
    IMBounds bounds;
    IMBounds freeBounds;
    uint32_t depth;
    uint32_t top;
    float scale;
    IMAnchoring anchoring;
    int growToFit;
    IMV2 *pos;
    IMV2 *dim;
    IMV2 *scroll;
    float *zoom;
    struct IMPanel *parent;
} IMPanel;

typedef struct
{
    float x, y;
    float w, h;
    float w_min, h_min, w_max, h_max;
    IMV2 scroll;
    float x_scrollMax, y_scrollMax;
    float zoom;
    IMAnchoring anchoring;
    int flags;
    int isExpanded;
    char *title;
} IMWindowState;

typedef enum
{
    IMFlag_draggable = 1 << 1,
    IMFlag_resizable = 1 << 2,
    IMFlag_full = 1 << 3,
    IMFlag_anchored = 1 << 4,
    IMFlag_zoomable = 1 << 5,
    IMFlag_scrollable = 1 << 6,
} IMFlag;

typedef enum
{
    IMStyle_background,
    IMStyle_panel,
    IMStyle_panelDark,
    IMStyle_panelBright,
    IMStyle_border,
    IMStyle_test,
    IMStyle_count,
} IMStyleType;

typedef struct
{
    IMRGBA baseColors[IMStyle_count];
    IMRGBA hotColors[IMStyle_count];
} IMStyle;

#define IM_FUNCTION_TABLE_SIZE 1024
typedef struct
{
    struct
    {
        uint32_t key;
        void *func;
    } table[IM_FUNCTION_TABLE_SIZE];
} FunctionTable;

/////////////////////////////////////////////////////////
// Drawing types

typedef struct
{
    float x, y, z;
    uint32_t abgr;
} PosColorVertex;

typedef struct
{
    int16_t x, y, z, w;
    int16_t x_min, x_max, y_min, y_max;
    uint32_t abgr;
} PosScissorColor;

typedef struct
{
    float x, y, z;
    float u, v;
} PosTexcoordVertex;

typedef struct
{
    float x, y, z;
    int16_t x_min, x_max, y_min, y_max;
    float u, v;
    uint32_t abgr;
} GlyphVert;

typedef struct
{
    int triCount;
    PosScissorColor triVerts[MAX_TRIANGLE_COUNT * 3];

    int triStripVertCount;
    PosScissorColor triStripVerts[MAX_TRISTRIP_VERT_COUNT];

#define MAX_PSC_VERT_COUNT 64 * 1024
#define MAX_PSC_ELEMENT_COUNT 128 * 1024
    int PSCVertCount;
    PosScissorColor PSCVerts[MAX_PSC_VERT_COUNT];
    int PSCElementCount;
    uint16_t PSCElements[MAX_PSC_ELEMENT_COUNT];

    //int rectCount;
    //RectInstance rects[MAX_RECT_INSTANCES];

    int diagramLineCount;
    PosColorVertex diagramVerts[MAX_DIAGRAM_LINES * 8];

    int glyphCount;
    GlyphVert glyphVerts[MAX_GLYPH_INSTANCES * 6];

} IMDrawBuffer;

///////////////////////////////////////////////////////////
// Font types

typedef struct
{
    float size;
    stbtt_packedchar fontData[NUM_FONT_GLYPHS];
} FontSize;

typedef struct
{
    char name[512];
    uint32_t nameHash;
    int fontSizeCount;
    FontSize sizes[NUM_FONT_SIZES];
} FontStore;

typedef enum
{
    Font_top,
    Font_bottom,
    Font_left,
    Font_right,
    Font_center,
} FontAlign;

typedef struct
{
    int activeType;
    void *active;
    uint8_t activeData[IM_DATA_SIZE];

    int hotType;
    void *hot;
    uint8_t hotData[IM_DATA_SIZE];
    size_t hotDataSize;

    uint32_t top;

    uint32_t zoomScrollTop;
    IMV2 pos;
    IMV2 *scroll;
    float *zoom;
    float scale;

    int isMouseActive;
    float windowWidth, windowHeight;
    IMInputState input;

    IMStyle style;
    IMAnchoring anchoring;

    FunctionTable activeFuncTable;
    FunctionTable hotFuncTable;

    // Drawing related state
    int scissorTop;
    IMV4 scissorBuffer[IM_SCISSOR_STACK_COUNT];
    IMDrawBuffer drawBuffer;
    FontStore fonts[NUM_FONT_SIZES];
    uint8_t *fontBitmap; // can be freed after sending to GPU

} IMContext;

inline IMV4 IMV4Make(float a, float b, float c, float d)
{
    IMV4 result = {a, b, c, d};
    return result;
}

inline IMV3 MakeIMV3(float a, float b, float c)
{
    IMV3 result = {a, b, c};
    return result;
}

inline IMV2 MakeIMV2(float a, float b)
{
    IMV2 result = {a, b};
    return result;
}

inline IMRGBA MakeIMRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    IMRGBA result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;
    return result;
}

// Fast math functions using SSE intrinsics from https://www.sebastiansylvan.com/post/scalarsseintrinsics/
__forceinline float rcp(float x)
{
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
}

__forceinline float rsqrt_fast(float x)
{
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
__forceinline float rsqrt(float x)
{
    float xrsqrt_est = rsqrt_fast(x);
    return xrsqrt_est * (1.5f - x * 0.5f * xrsqrt_est * xrsqrt_est); // NR iteration
}

#endif