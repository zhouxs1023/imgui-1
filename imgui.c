#ifndef STB_TRUETYPE_IMPLEMENTATION // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#include "stb_truetype.h"

typedef int(ButtonFunc)(IMContext *, void *);
typedef void(IM_actionFunc)(IMContext *);

inline void IMPushData(IMContext *im, void *data, size_t size)
{
    memset(im->hotData, 0, sizeof(im->hotData));
    memcpy(im->hotData, data, size);
}

inline void IMNewFrame(IMContext *im, int windowWidth, int windowHeight)
{
    im->top = 0;
    im->hot = NULL;
    im->zoomScrollTop = 0;
    im->zoom = NULL;
    im->scroll = NULL;

    im->windowWidth = (float)windowWidth;
    im->windowHeight = (float)windowHeight;

    im->scissorTop = 0;
    im->scissorBuffer[0].xmin = 0.f;
    im->scissorBuffer[0].xmax = im->windowWidth;
    im->scissorBuffer[0].ymin = 0.f;
    im->scissorBuffer[0].ymax = im->windowHeight;
}

inline void IMPushScissor(IMContext *im, IMV4 scissor)
{
    IMV4 *scissorLast = &im->scissorBuffer[im->scissorTop];
    im->scissorTop++;
    im->scissorBuffer[im->scissorTop].xmin = IM_MAX(scissor.xmin, scissorLast->xmin);
    im->scissorBuffer[im->scissorTop].xmax = IM_MIN(scissor.xmax, scissorLast->xmax);
    im->scissorBuffer[im->scissorTop].ymin = IM_MAX(scissor.ymin, scissorLast->ymin);
    im->scissorBuffer[im->scissorTop].ymax = IM_MIN(scissor.ymax, scissorLast->ymax);
}

inline void IMPopScissor(IMContext *im)
{
    if (im->scissorTop > 0)
    {
        im->scissorTop--;
    }
}

inline void IMProcessKeyEvents(IMKeyState *state, int32_t pressStarted, int32_t pressEnded)
{
    state->pressStarted = 0;
    state->pressEnded = 0;
    if (pressStarted)
    {
        state->pressStarted = 1;
        state->down = 1;
    }
    if (pressEnded)
    {
        state->pressEnded = 1;
        state->down = 0;
    }
}

inline void IMUpdateInput(IMContext *im,
                          int32_t mouseX,
                          int32_t mouseY,
                          int32_t mouseDeltaX,
                          int32_t mouseDeltaY,
                          int32_t scrollX,
                          int32_t scrollY,
                          int32_t esc_Started,
                          int32_t esc_Ended,
                          int32_t enter_Started,
                          int32_t enter_Ended,
                          int32_t m1_Started,
                          int32_t m1_Ended,
                          int32_t m2_Started,
                          int32_t m2_Ended,
                          int32_t m3_Started,
                          int32_t m3_Ended)
{
    im->input.mx = mouseX;
    im->input.my = mouseY;
    im->input.mdx = mouseDeltaX;
    im->input.mdy = mouseDeltaY;
    im->input.scrollx = scrollX;
    im->input.scrolly = scrollY;

    IMProcessKeyEvents(&im->input.esc, esc_Started, esc_Ended);
    IMProcessKeyEvents(&im->input.enter, enter_Started, enter_Ended);
    IMProcessKeyEvents(&im->input.m1, m1_Started, m1_Ended);
    IMProcessKeyEvents(&im->input.m2, m2_Started, m2_Ended);
    IMProcessKeyEvents(&im->input.m3, m3_Started, m3_Ended);

    im->isMouseActive = 1;
}

inline IMPanel IMMakePanel(IMAnchoring anchoring, int growToFit, uint32_t depth, float xmin, float ymin, float xmax, float ymax)
{
    IMPanel panel = {0};
    panel.anchoring = anchoring;
    panel.growToFit = growToFit;
    panel.bounds.xmin = xmin;
    panel.bounds.ymin = ymin;
    panel.bounds.xmax = xmax;
    panel.bounds.ymax = ymax;
    memcpy(panel.freeBounds.e, panel.bounds.e, sizeof(float) * 4);
    panel.depth = depth;
    panel.top = depth;
    panel.scale = 1.f;
    return panel;
}

inline IMPanel IMEmptyPanel()
{
    IMPanel panel;
    memset(&panel, 0, sizeof(panel));
    panel.depth = 1;
    panel.top = 1;
    panel.scale = 1.f;
    return panel;
}

inline void IMInitSubPanel(IMPanel *parent, IMPanel *child, float w)
{
    float scale = parent->scale;
    if (parent->zoom != NULL)
    {
        scale *= *parent->zoom;
    }
    child->scale = scale;
    switch (child->anchoring)
    {
    case IM_left:
    {
        child->bounds.xmin = parent->freeBounds.xmin;
        child->bounds.xmax = parent->freeBounds.xmin + w * scale;
        child->bounds.ymin = parent->freeBounds.ymin;
        child->bounds.ymax = parent->freeBounds.ymax;
        parent->freeBounds.xmin = child->bounds.xmax;
    }
    break;
    case IM_right:
    {
        child->bounds.xmin = parent->freeBounds.xmax - w * scale;
        child->bounds.xmax = parent->freeBounds.xmax;
        child->bounds.ymin = parent->freeBounds.ymin;
        child->bounds.ymax = parent->freeBounds.ymax;
        parent->freeBounds.xmax = child->bounds.xmin;
    }
    break;
    case IM_top:
    default:
    {
        child->bounds.xmin = parent->freeBounds.xmin;
        child->bounds.xmax = parent->freeBounds.xmax;
        child->bounds.ymin = parent->freeBounds.ymin;
        child->bounds.ymax = parent->freeBounds.ymin + w * scale;
        parent->freeBounds.ymin = child->bounds.ymax;
    }
    break;
    }
    if (parent->growToFit)
    {
        if (parent->bounds.ymax < child->bounds.ymax)
        {
            parent->bounds.ymax = child->bounds.ymax;
        }
        if (parent->bounds.xmax < child->bounds.xmax)
        {
            parent->bounds.xmax = child->bounds.xmax;
        }
        if (parent->bounds.ymin > child->bounds.ymin)
        {
            parent->bounds.ymin = child->bounds.ymin;
        }
        if (parent->bounds.xmin > child->bounds.xmin)
        {
            parent->bounds.xmin = child->bounds.xmin;
        }
    }
    memcpy(child->freeBounds.e, child->bounds.e, sizeof(float) * 4);
    child->depth = parent->depth + 1;
    if (parent->top < child->depth)
    {
        parent->top = child->depth;
    }
    child->parent = parent;
}

inline IMPanel IMMakeSubPanel(IMPanel *parent, IMAnchoring anchoring, float w)
{
    IMPanel child = IMEmptyPanel();
    child.anchoring = anchoring;
    IMInitSubPanel(parent, &child, w);
    return child;
}

inline void IMGrowPanel(IMPanel *panel, IMV4 padding)
{
    panel->bounds.xmin -= padding.x * panel->scale;
    panel->bounds.ymin -= padding.y * panel->scale;
    panel->bounds.xmax += padding.z * panel->scale;
    panel->bounds.ymax += padding.w * panel->scale;

    panel->freeBounds.xmin -= padding.x * panel->scale;
    panel->freeBounds.ymin -= padding.y * panel->scale;
    panel->freeBounds.xmax += padding.z * panel->scale;
    panel->freeBounds.ymax += padding.w * panel->scale;
}

inline void IMShiftPanel(IMPanel *panel, IMV2 shift)
{
    panel->bounds.xmin += shift.x * panel->scale;
    panel->bounds.ymin += shift.y * panel->scale;
    panel->bounds.xmax += shift.x * panel->scale;
    panel->bounds.ymax += shift.y * panel->scale;

    panel->freeBounds.xmin += shift.x * panel->scale;
    panel->freeBounds.ymin += shift.y * panel->scale;
    panel->freeBounds.xmax += shift.x * panel->scale;
    panel->freeBounds.ymax += shift.y * panel->scale;
}

inline void IMSubdividePanel(IMPanel *parent, int x, int y, IMPanel *panels)
{
    assert(x > 0);
    assert(y > 0);
    float parentWidth = parent->bounds.xmax - parent->bounds.xmin;
    float parentHeight = parent->bounds.ymax - parent->bounds.ymin;
    float xmin = parent->bounds.xmin;
    float ymin = parent->bounds.ymin;
    float childWidth = parentWidth / x;
    float childHeight = parentHeight / y;
    for (int iy = 0; iy < y; iy++)
    {
        for (int ix = 0; ix < x; ix++)
        {
            IMPanel *c = panels + iy * x + ix;
            memcpy(c, parent, sizeof(IMPanel));
            c->bounds.xmin = xmin + childWidth * ix;
            c->bounds.ymin = ymin + childHeight * iy;
            c->bounds.xmax = c->bounds.xmin + childWidth;
            c->bounds.ymax = c->bounds.ymin + childHeight;
            memcpy(&c->freeBounds, &c->bounds, sizeof(IMBounds));
        }
    }
}

inline int IM_CheckPointOverlap(IMBounds *bounds, IMBounds *pbounds, float mx, float my)
{
    int overlap = (mx >= bounds->xmin &&
                   mx <= bounds->xmax &&
                   my >= bounds->ymin &&
                   my <= bounds->ymax);
    if (pbounds != NULL)
    {
        int poverlap = (mx >= pbounds->xmin &&
                        mx <= pbounds->xmax &&
                        my >= pbounds->ymin &&
                        my <= pbounds->ymax);
        overlap = (overlap && poverlap);
    }
    return overlap;
}

inline void IMDrawLabel(IMContext *im, IMPanel *panel, char *text, void *target)
{
    if (text != NULL)
    {
        IMPanel *parent = panel->parent;
        if (parent != NULL)
        {
            IMPushScissor(im, parent->bounds);
        }
        IMRGBA color = MakeIMRGBA(240, 150, 160, 255);
        IMV3 stringPos = {(panel->freeBounds.xmin + panel->freeBounds.xmax) * 0.5f, (panel->freeBounds.ymin + panel->freeBounds.ymax) * 0.5f, (float)panel->depth + 1.f};
        PushStr(im,
                stringPos,
                Font_center, Font_center,
                0, MakeIMRGBA(240, 150, 160, 255), 16.f * panel->scale, text);
        if (parent != NULL)
        {
            IMPopScissor(im);
        }
    }
}

inline void IMDrawContainer(IMContext *im, IMPanel *panel, void *target, int centerStyle, int borderStyle)
{
    float width = panel->bounds.xmax - panel->bounds.xmin;
    float height = panel->bounds.ymax - panel->bounds.ymin;
    IMRGBA *centerColor = &im->style.baseColors[centerStyle];
    IMRGBA *borderColor = &im->style.baseColors[borderStyle];
    if (target != NULL)
    {
        if (im->active != NULL)
        {
            if (im->active == target)
            {
                centerColor = &im->style.baseColors[centerStyle];
                borderColor = &im->style.hotColors[borderStyle];
            }
        }
        else if (target == im->hot)
        {
            centerColor = &im->style.baseColors[centerStyle];
            borderColor = &im->style.hotColors[borderStyle];
        }
    }
    //IMV2 pos = {panel->bounds.xmin, panel->bounds.ymin};
    //IMV2 dim = {width, height};
    IMV2 thickness = {2.f * panel->scale, 2.f * panel->scale};
    PushRect(im, panel->bounds, thickness, (float)panel->depth, *centerColor, *borderColor);
}

IMPanel IMStartContainer(IMContext *im, IMPanel *parent, IMAnchoring anchoring, IMV2 *pos, IMV2 *dim, IMV2 *scroll, float *zoom)
{
    IMPanel panel = IMEmptyPanel();
    if (parent != NULL)
    {
        panel.depth = parent->depth + 1;
        if (parent->top < panel.depth)
        {
            parent->top = panel.depth;
        }
        panel.scale *= parent->scale;
        if (parent->zoom != NULL)
        {
            panel.scale *= *parent->zoom;
        }
    }
    panel.anchoring = anchoring;
    panel.pos = pos;
    panel.dim = dim;
    panel.scroll = scroll;
    panel.zoom = zoom;
    panel.parent = parent;
    if (parent != NULL &&
        anchoring == IM_full)
    {
        panel.bounds = parent->freeBounds;
    }
    else if (parent != NULL &&
             anchoring != IM_free)
    {
        float width = dim->x;
        if (anchoring == IM_top || anchoring == IM_bottom)
        {
            width = dim->y;
        }
        IMInitSubPanel(parent, &panel, width);
    }
    else
    {
        panel.bounds.xmin = pos->x * panel.scale;
        panel.bounds.ymin = pos->y * panel.scale;
        panel.bounds.xmax = pos->x * panel.scale + dim->x * panel.scale;
        panel.bounds.ymax = pos->y * panel.scale + dim->y * panel.scale;

        if (parent != NULL)
        {
            if (parent->scroll != NULL)
            {
                panel.bounds.xmin += parent->scroll->x; // panel.scale;
                panel.bounds.xmax += parent->scroll->x; // panel.scale;
                panel.bounds.ymin += parent->scroll->y; // panel.scale;
                panel.bounds.ymax += parent->scroll->y; // panel.scale;
            }
        }
    }
    IMPushScissor(im, panel.bounds);
    panel.freeBounds = panel.bounds;
    if (scroll != NULL)
    {
        panel.freeBounds.xmin += scroll->x + 2.f;
        panel.freeBounds.xmax += scroll->x - 2.f;
        panel.freeBounds.ymin += scroll->y + 2.f;
        panel.freeBounds.ymax += scroll->y - 2.f;
    }

    return panel;
}

int IMFinishContainer(IMContext *im, IMPanel *panel, int type, void *target, void *data, size_t dataSize)
{
    IMPopScissor(im);
    int overlap = 0;
    if (im->isMouseActive)
    {
        IMBounds *pbounds = NULL;
        if (panel->parent != NULL)
        {
            pbounds = &panel->parent->bounds;
        }
        overlap = IM_CheckPointOverlap(&panel->bounds, pbounds, im->input.mx, im->input.my);
        if (overlap)
        {
            if (panel->depth > im->top && target != NULL)
            {
                im->top = panel->depth;
                im->hotType = type;
                im->hot = target;
                IMPushData(im, data, dataSize);
                im->hotDataSize = dataSize;
            }

            if (panel->depth > im->zoomScrollTop)
            {
                if (panel->zoom || panel->scroll)
                {
                    im->pos.x = panel->bounds.xmin;
                    im->pos.y = panel->bounds.ymin;
                    im->zoomScrollTop = panel->depth;
                    im->zoom = panel->zoom;
                    im->scroll = panel->scroll;
                }
            }
        }
    }
    if (panel->parent != NULL)
    {
        IMPanel *parent = panel->parent;
        if (panel->parent->top < panel->top)
        {
            panel->parent->top = panel->top;
        }
        if (panel->growToFit && parent->growToFit)
        {
            if (parent->bounds.ymax < panel->bounds.ymax)
            {
                parent->bounds.ymax = panel->bounds.ymax;
            }
            if (parent->bounds.xmax < panel->bounds.xmax)
            {
                parent->bounds.xmax = panel->bounds.xmax;
            }
            if (parent->bounds.ymin > panel->bounds.ymin)
            {
                parent->bounds.ymin = panel->bounds.ymin;
            }
            if (parent->bounds.xmin > panel->bounds.xmin)
            {
                parent->bounds.xmin = panel->bounds.xmin;
            }
        }
    }
    return overlap;
}

inline void IMDoLabel(IMContext *im, IMPanel *parent, IMAnchoring anchoring, float size, char *text)
{

    IMPanel lPanel = IMMakeSubPanel(parent, anchoring, size);
    IMDrawLabel(im, &lPanel, text, NULL);
}

inline void IMDoIntField(IMContext *im, IMPanel *parent, int *target, IMV2i *minmax)
{
    IMV2 dim = {20.f, 20.f};
    IMPanel intPanel = IMStartContainer(im, parent, IM_top, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &intPanel, IM_intField, target, minmax, sizeof(IMV2i));
    IMDrawContainer(im, &intPanel, target, IMStyle_panel, IMStyle_border);
    if (parent != NULL)
    {
        //nvgScissor(vg,parent->bounds.bounds[0],parent->bounds.bounds[1],parent->bounds.bounds[2] - parent->bounds.bounds[0],parent->bounds.bounds[3] - parent->bounds.bounds[1]);
    }
    float width = intPanel.bounds.xmax - intPanel.bounds.xmin;
    float height = intPanel.bounds.ymax - intPanel.bounds.ymin;
    char text[16];
    // TODO stbsp_snprintf(text, 16, "%i", *target);
    IMDrawLabel(im, &intPanel, text, NULL);
}

inline void IMDoFloatField(IMContext *im, IMPanel *parent, float *target, IMFloatInfo *fi)
{
    IMV2 dim = {20.f, 20.f};
    IMPanel floatPanel = IMStartContainer(im, parent, IM_top, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &floatPanel, IM_floatField, target, fi, sizeof(IMFloatInfo));
    IMDrawContainer(im, &floatPanel, target, IMStyle_panel, IMStyle_border);
    if (parent != NULL)
    {
        //nvgScissor(vg,parent->bounds.bounds[0],parent->bounds.bounds[1],parent->bounds.bounds[2] - parent->bounds.bounds[0],parent->bounds.bounds[3] - parent->bounds.bounds[1]);
    }
    float width = floatPanel.bounds.xmax - floatPanel.bounds.xmin;
    float height = floatPanel.bounds.ymax - floatPanel.bounds.ymin;
    IMDrawLabel(im, &floatPanel, fi->string, NULL);
}

inline void IMDoStringField(IMContext *im, IMPanel *parent, char *string, int size)
{
    IMV2 dim = {20.f, 20.f};
    IMStringInfo stringInfo = {string, size};
    IMPanel stringPanel = IMStartContainer(im, parent, IM_top, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &stringPanel, IM_stringField, string, &stringInfo, sizeof(stringInfo));
    IMDrawContainer(im, &stringPanel, string, IMStyle_panel, IMStyle_border);
    IMDrawLabel(im, &stringPanel, string, NULL);
}

inline int IMDoButton(IMContext *im, IMPanel *parent, IMAnchoring anchoring, float size, char *text, void *func, void *data, IMDefaultType buttonType)
{
    IMV2 dim = {size, size};
    IMFunctionInfo funcInfo = {func, data};
    uint64_t id = (uint64_t)func + (uint64_t)data;
    IMPanel buttonPanel = IMStartContainer(im, parent, anchoring, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &buttonPanel, buttonType, (void *)id, &funcInfo, sizeof(funcInfo));
    IMDrawContainer(im, &buttonPanel, (void *)id, IMStyle_panel, IMStyle_border);
    IMDrawLabel(im, &buttonPanel, text, NULL);
    return 0;
}

inline int IMDoButtonCustom(IMContext *im, IMPanel *parent, IMAnchoring anchoring, float size, char *text,
                            void *id, void *func, void *data, size_t dataSize,
                            IMDefaultType buttonType)
{
    IMV2 dim = {size, size};
    IMFunctionCustom funcData = {func};
    memcpy(funcData.data, data, dataSize);
    IMPanel buttonPanel = IMStartContainer(im, parent, anchoring, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &buttonPanel, buttonType, id, &funcData, sizeof(funcData));
    IMDrawContainer(im, &buttonPanel, (void *)id, IMStyle_panel, IMStyle_border);
    IMDrawLabel(im, &buttonPanel, text, NULL);
    return 0;
}

inline void IMDoFloatSlider(IMContext *im, IMPanel *parent, float *target, IMFloatInfo *fi)
{
    IMV2 dim = {10.f, 10.f};
    if (fi->string == NULL)
    {
        //int sigs = (*target > 10) + (*target > 100) + (*target > 1000) + (*target > 10000) + 7;
        //stbsp_snprintf(fi->string, 16, "%'.*g", sigs, *target);
    }
    IMPanel floatPanel = IMStartContainer(im, parent, IM_top, NULL, &dim, NULL, NULL);
    IMFinishContainer(im, &floatPanel, IM_floatField, target, fi, sizeof(IMFloatInfo));
    IMDrawContainer(im, &floatPanel, target, IMStyle_panel, IMStyle_border);
    float width = (*target - fi->fmin) / (fi->fmax - fi->fmin) * (floatPanel.bounds.xmax - floatPanel.bounds.xmin);
    float height = (floatPanel.bounds.ymax - floatPanel.bounds.ymin);
    //IMV2 rpos = {floatPanel.bounds.xmin, floatPanel.bounds.ymin + height / 2.f};
    //IMV2 rdim = {width, height};
    IMV4 sliderRect = {floatPanel.bounds.xmin, floatPanel.bounds.xmin + width, floatPanel.bounds.ymin + height * 0.4f, floatPanel.bounds.ymin + height * 0.6f};
    IMV2 thickness = {0.f, 0.f};
    //IMV2 thickness = {2.f * floatPanel.scale, 2.f * floatPanel.scale};
    PushRect(im, sliderRect, thickness, (float)floatPanel.depth, im->style.hotColors[IMStyle_border], im->style.hotColors[IMStyle_border]);
}

inline void IMDrawToggle(IMContext *im, IMPanel *panel, IMPanel *parent, char *text, int *target)
{
    float width = panel->bounds.xmax - panel->bounds.xmin;
    float height = panel->bounds.ymax - panel->bounds.ymin;

    if (target != NULL && *target)
    {
        IMDrawContainer(im, panel, target, IMStyle_panelBright, IMStyle_border);
    }
    else
    {
        IMDrawContainer(im, panel, target, IMStyle_panel, IMStyle_border);
    }
    if (text != NULL)
    {
        IMDrawLabel(im, panel, text, NULL);
    }
}

inline int IMDoToggle(IMContext *im, IMPanel *parent, IMAnchoring anchoring, float size, char *text, int *target)
{
    // TODO: have this use a default panel if no parent panel is assigned
    IMPanel panel = IMEmptyPanel();
    panel.anchoring = anchoring;
    IMInitSubPanel(parent, &panel, size);
    int overlap = 0;
    if (im->isMouseActive && panel.depth > im->top)
    {
        int overlap = IM_CheckPointOverlap(&panel.bounds, &parent->bounds, im->input.mx, im->input.my);
        if (overlap)
        {
            im->top = panel.depth;
            im->hot = target;
            im->hotType = IM_toggle;
        }
    }
    IMDrawToggle(im, &panel, parent, text, target);
    return overlap;
}

inline void IMDoResizeHandle(IMContext *im, int flags, IMPanel *parent, IMWindowState *windowState)
{
    IMBounds bounds;
    bounds.xmax = parent->bounds.xmax;
    bounds.ymax = parent->bounds.ymax;
    bounds.xmin = bounds.xmax - 10.f;
    bounds.ymin = bounds.ymax - 10.f;
    uint32_t depth = ++parent->top;

    if (im->isMouseActive && depth >= im->top)
    {
        int overlap = IM_CheckPointOverlap(&bounds, NULL, im->input.mx, im->input.my);
        if (overlap)
        {
            im->top = parent->depth;
            im->hot = windowState;
            im->hotType = IM_resizeHandle;
        }
    }

    float width = bounds.xmax - bounds.xmin;
    float height = bounds.ymax - bounds.ymin;

    IMPanel rPanel = IMEmptyPanel();
    rPanel.bounds = bounds;
    rPanel.depth = depth;
    rPanel.parent = parent;
    IMDrawContainer(im, &rPanel, NULL, IMStyle_panel, IMStyle_border);
}

inline void IMDrawWindow(IMContext *im, IMPanel *panel, IMPanel *parent, IMWindowState *windowState)
{
    IMDrawContainer(im, panel, windowState, IMStyle_background, IMStyle_border);
}

inline IMPanel IMStartWindowPanel(IMContext *im, IMPanel *parent, IMWindowState *windowState)
{
    IMPanel panel = IMEmptyPanel();
    panel.zoom = &windowState->zoom;
    panel.scroll = &windowState->scroll;
    panel.parent = parent;
    if (parent != NULL &&
        (windowState->flags & IMFlag_full))
    {
        panel.bounds = parent->freeBounds;
        if (windowState->title != NULL ||
            (windowState->flags & IMFlag_draggable))
        {
            panel.bounds.ymin += 20.f; // space saved for window label
        }
    }
    else if (parent != NULL &&
             (windowState->flags & IMFlag_anchored))
    {
        panel.anchoring = windowState->anchoring;
        IMInitSubPanel(parent, &panel, windowState->w);
    }
    else
    {
        if (windowState->w_max > 0.f)
        {
            windowState->w = IM_CLAMP(windowState->w, windowState->w_min, windowState->w_max);
        }
        if (windowState->h_max > 0.f)
        {
            windowState->h = IM_CLAMP(windowState->h, windowState->h_min, windowState->h_max);
        }

        panel.bounds.xmin = windowState->x; // space saved for window label
        panel.bounds.ymin = windowState->y + 20.f;
        panel.bounds.xmax = windowState->x + windowState->w;
        panel.bounds.ymax = windowState->y + windowState->h;
    }
    panel.freeBounds = panel.bounds;
    panel.freeBounds.xmin += windowState->scroll.x + 2.f;
    panel.freeBounds.xmax += windowState->scroll.x - 2.f;
    panel.freeBounds.ymin += windowState->scroll.y + 2.f;
    panel.freeBounds.ymax += windowState->scroll.y - 2.f;

    if (parent != NULL)
    {
        panel.scale *= parent->scale;
        if (parent->zoom != NULL)
        {
            panel.scale *= *parent->zoom;
        }
        if (parent->scroll != NULL)
        {
            panel.bounds.xmin += parent->scroll->x * panel.scale;
            panel.bounds.xmax += parent->scroll->x * panel.scale;
            panel.bounds.ymin += parent->scroll->y * panel.scale;
            panel.bounds.ymax += parent->scroll->y * panel.scale;
        }
        panel.depth = parent->depth + 1;
    }

    IMPushScissor(im, panel.bounds);

    return panel;
}

inline void IMFinishWindowPanel(IMContext *im, IMPanel *panel, IMWindowState *windowState)
{
    IMPanel *parent = panel->parent;
    float yFree = panel->freeBounds.ymax - panel->freeBounds.ymin;
    float xFree = panel->freeBounds.xmax - panel->freeBounds.xmin;
    if (yFree < 0)
    {
        windowState->y_scrollMax = yFree;
    }
    else
    {
        windowState->y_scrollMax = 0.f;
    }
    if (xFree < 0)
    {
        windowState->x_scrollMax = xFree;
    }
    else
    {
        windowState->x_scrollMax = 0.f;
    }

    if (!(windowState->flags & IMFlag_zoomable))
    {
        windowState->scroll.x = IM_CLAMP(windowState->scroll.x, windowState->x_scrollMax, 0);
        windowState->scroll.y = IM_CLAMP(windowState->scroll.y, windowState->y_scrollMax, 0);
    }

    if (windowState->flags & IMFlag_resizable)
    {
        IMDoResizeHandle(im, 0, panel, windowState);
    }
    panel->bounds.ymin -= 20.f;
    IMPanel labelPanel = IMEmptyPanel();
    labelPanel.depth += panel->depth;
    labelPanel.bounds = panel->bounds;
    if (windowState->title != NULL ||
        (windowState->flags & IMFlag_draggable))
    {
        labelPanel.bounds.ymax = labelPanel.bounds.ymin + 20.f;
        memcpy(labelPanel.freeBounds.e, labelPanel.bounds.e, sizeof(labelPanel.freeBounds.e));
        IMDrawLabel(im, &labelPanel, windowState->title, NULL);
    }

    int overlap = 0;
    if (im->isMouseActive)
    {
        IMBounds *pbounds = NULL;
        if (parent != NULL)
        {
            pbounds = &parent->bounds;
        }
        overlap = IM_CheckPointOverlap(&panel->bounds, pbounds, im->input.mx, im->input.my);
        if (overlap)
        {
            if (panel->depth > im->top)
            {
                im->top = panel->depth;
                im->hot = windowState;
                im->hotType = IM_window;
            }

            if (panel->depth > im->zoomScrollTop)
            {
                im->zoomScrollTop = panel->depth;
                if (windowState->flags & IMFlag_zoomable)
                {
                    im->zoom = &windowState->zoom;
                }
                else
                {
                    im->zoom = NULL;
                }
                if (windowState->flags & IMFlag_scrollable)
                {
                    im->scroll = &windowState->scroll;
                }
                else
                {
                    im->scroll = NULL;
                }
            }
        }
    }
    IMPopScissor(im);
    IMDrawWindow(im, panel, parent, windowState);
}

void IM_IntFieldAction(IMContext *im)
{
    int *val = (int *)im->active;
    IMV2i *minmax = (IMV2i *)im->activeData;
    int *isValShifted = (int *)(minmax + 1);
    int *isTextInputActive = (int *)(isValShifted + 1);
    if (im->input.m1.pressEnded)
    {
        if (im->active == im->hot && *isValShifted != 1)
        {
            *isTextInputActive = 1;
            *val = 0;
        }
        else
        {
            im->active = NULL;
        }
    }
    else if (*isTextInputActive)
    {
        char string[16];
        //TODO stbsp_snprintf(string, 16, "%i", *val);
        for (int i = 0; i < im->input.numBackspace; i++)
        {
            int len = strlen(string);
            if (len > 0)
            {
                string[len - 1] = '\0';
            }
        }
        strncat(string, im->input.text, 16);
        *val = atoi(string);
        if (im->input.enter.pressStarted || im->input.m1.pressStarted)
        {
            *val = IM_CLAMP(*val, minmax->x, minmax->y);
            im->active = NULL;
        }
    }
    else
    {
        if (im->input.mdx != 0)
        {
            *isValShifted = 1; // 1 = skip keyboard input
            *val += im->input.mdx;
            *val = IM_CLAMP(*val, minmax->x, minmax->y);
        }
    }
}

void IM_FloatFieldAction(IMContext *im)
{
    float *val = (float *)im->active;
    IMFloatInfo *fi = (IMFloatInfo *)im->activeData;
    char *string = fi->string;
    if (string == NULL)
    {
        string = fi->buff;
    }
    int *isValShifted = (int *)(fi + 1);
    int *isTextInputActive = (int *)(isValShifted + 1);
    if (im->input.m1.pressEnded)
    {
        if (im->active == im->hot && *isValShifted != 1)
        {
            *isTextInputActive = 1;
            string[0] = 0;
        }
        else
        {
            im->active = NULL;
        }
    }
    else if (*isTextInputActive)
    {
        for (int i = 0; i < im->input.numBackspace; i++)
        {
            int len = strlen(string);
            if (len > 0)
            {
                string[len - 1] = '\0';
            }
        }
        strncat(string, im->input.text, 16);
        *val = atof(string);
        if (im->input.enter.pressStarted || im->input.m1.pressStarted)
        {
            *val = IM_CLAMP(*val, fi->fmin, fi->fmax);
            im->active = NULL;
        }
    }
    else
    {
        if (im->input.mdx != 0)
        {
            *isValShifted = 1; // 1 = skip keyboard input
            float shiftScale = (fi->fmax - fi->fmin) / 500.f;
            *val += im->input.mdx * shiftScale;
            *val = IM_CLAMP(*val, fi->fmin, fi->fmax);
            int sigs = (*val > 10) + (*val > 100) + (*val > 1000) + (*val > 10000) + 7;
            //TODO stbsp_snprintf(string, 16, "%'.*g", sigs, *val);
        }
    }
}

void IM_StringFieldAction(IMContext *im)
{
    IMStringInfo *stringInfo = (IMStringInfo *)im->activeData;
    char *string = stringInfo->string;
    int *isTextInputActive = (int *)(stringInfo + 1);
    if (im->input.m1.pressEnded)
    {
        if (im->active == im->hot && *isTextInputActive != 1)
        {
            *isTextInputActive = 1;
        }
        else
        {
            im->active = NULL;
        }
    }
    else if (*isTextInputActive)
    {
        for (int i = 0; i < im->input.numBackspace; i++)
        {
            int len = strlen(string);
            if (len > 0)
            {
                string[len - 1] = '\0';
            }
        }
        strncat(string, im->input.text, stringInfo->size);
        if (im->input.enter.pressStarted || im->input.m1.pressStarted)
        {
            im->active = NULL;
        }
    }
}

void IM_ToggleAction(IMContext *im)
{
    if (im->input.m1.pressEnded)
    {
        if (im->hot == im->active)
        {
            int *target = (int *)im->active;
            *target = !*target;
        }
        im->active = NULL;
    }
}

void IM_WindowAction(IMContext *im)
{
    if (im->input.m1.pressEnded)
    {
        im->active = NULL;
    }
    else
    {
        IMWindowState *win = (IMWindowState *)im->active;
        if (win->flags & IMFlag_zoomable)
        {
            win->scroll.x += im->input.mdx;
            win->scroll.y += im->input.mdy;
        }
        else
        {
            win->x += im->input.mdx;
            win->y += im->input.mdy;
        }
    }
}

void IM_ResizeHandleAction(IMContext *im)
{
    if (im->input.m1.pressEnded)
    {
        im->active = NULL;
    }
    else
    {
        IMWindowState *win = (IMWindowState *)im->active;
        win->w += im->input.mdx;
        win->h += im->input.mdy;
    }
}

inline void IM_SetHotActive(IMContext *im)
{
    im->activeType = im->hotType;
    im->active = im->hot;
    memcpy(im->activeData, im->hotData, IM_DATA_SIZE);
}

void IM_DefaultHotAction(IMContext *im)
{
    if (im->input.m1.pressStarted)
    {
        IM_SetHotActive(im);
    }
}

void IM_CallFuncSimple(IMContext *im)
{
    IMFunctionInfo *funcInfo = (IMFunctionInfo *)im->activeData;
    ((ButtonFunc *)funcInfo->funcAddr)(im, funcInfo->funcArg);
}

void IM_CallFunc(IMContext *im)
{
    IMFunctionCustom *funcInfo = (IMFunctionCustom *)im->activeData;
    ((ButtonFunc *)funcInfo->funcAddr)(im, (void *)funcInfo->data);
}

void IM_FuncAction(IMContext *im)
{
    if (im->input.m1.pressEnded)
    {
        void *oldActive = im->active;
        if (im->activeData != NULL)
        {
            IM_CallFunc(im);
        }
        if (im->active == oldActive) // if the active element has changed, assume the new active item is meant to stay active
        {
            im->active = NULL;
        }
    }
}

void IM_FuncSimpleAction(IMContext *im)
{
    if (im->input.m1.pressEnded)
    {
        void *oldActive = im->active;
        if (im->activeData != NULL)
        {
            IM_CallFuncSimple(im);
        }
        if (im->active == oldActive) // if the active element has changed, assume the new active item is meant to stay active
        {
            im->active = NULL;
        }
    }
}

void IM_FuncOnPressAction(IMContext *im)
{
    if (im->input.m1.pressStarted)
    {
        if (im->hotData != NULL)
        {
            IM_SetHotActive(im);
            IM_CallFunc(im);
        }
    }
}

void IM_FuncOnPressSimpleAction(IMContext *im)
{
    if (im->input.m1.pressStarted)
    {
        if (im->hotData != NULL)
        {
            IM_SetHotActive(im);
            IM_CallFuncSimple(im);
        }
    }
}

// Taken from Sean Barrett's STB.h
#define IM_REHASH(x) ((x) + ((x) >> 6) + ((x) >> 19))

inline uint32_t IM_HashInt32(uint32_t x)
{
    x = IM_REHASH(x);
    x += x << 16;

    // pearson's shuffle
    x ^= x << 3;
    x += x >> 5;
    x ^= x << 2;
    x += x >> 15;
    x ^= x << 10;
    return IM_REHASH(x);
}

int IM_InsertFunc(FunctionTable *table, int32_t id, void *func)
{
    uint32_t index = IM_HashInt32((uint32_t)id) % IM_FUNCTION_TABLE_SIZE;
    for (int i = 0; i < IM_FUNCTION_TABLE_SIZE; i++)
    {
        if (table->table[index].key == 0)
        {
            table->table[index].key = id;
            table->table[index].func = func;
            return 0;
        }
        index++;
        if (index >= IM_FUNCTION_TABLE_SIZE)
        {
            index = 0;
        }
    }
    return 1;
}

void *IM_GetFunc(FunctionTable *table, int32_t id)
{
    uint32_t index = IM_HashInt32((uint32_t)id) % IM_FUNCTION_TABLE_SIZE;
    for (int i = 0; i < IM_FUNCTION_TABLE_SIZE; i++)
    {
        if (table->table[index].key == id)
        {
            return table->table[index].func;
        }
        else if (table->table[index].key == 0)
        {
            return NULL;
        }
        index++;
        if (index >= IM_FUNCTION_TABLE_SIZE)
        {
            index = 0;
        }
    }
    return NULL;
}

void IMProcessInput(IMContext *im)
{
    IMInputState *input = &im->input;
    if (im->active != NULL)
    {
        void *activeFunc = IM_GetFunc(&im->activeFuncTable, im->activeType);
        if (activeFunc != NULL)
        {
            ((IM_actionFunc *)activeFunc)(im);
        }
    }
    else if (im->hot != NULL)
    {
        void *hotFunc = IM_GetFunc(&im->hotFuncTable, im->hotType);
        if (hotFunc != NULL)
        {
            ((IM_actionFunc *)hotFunc)(im);
        }
    }

    if (im->zoom != NULL)
    {
        if (input->scrolly)
        {
            float zoomCoeff = input->scrolly > 0 ? 1.25f : 0.8f;
            float z_init = *im->zoom;
            *im->zoom *= zoomCoeff;
            *im->zoom = IM_CLAMP(*im->zoom, 0.16777216f, 1.953125f); // 0.8^5 and 1.25^3, 5 levels of out, 3 levels in
            float z_final = *im->zoom;
            float mx = im->input.mx - im->pos.x - im->scroll->x;
            float my = im->input.my - im->pos.y - im->scroll->y;
            float dx = mx / z_final - mx / z_init;
            float dy = my / z_final - my / z_init;

            im->scroll->x += dx * z_final;
            im->scroll->y += dy * z_final;
        }
    }
    else
    {
        if (im->scroll != NULL)
        {
            im->scroll->x += 20.f * input->scrollx;
            im->scroll->y += 20.f * input->scrolly;
        }
    }
}

void IM_InitStyle(IMContext *im)
{
    IMRGBA *bc = im->style.baseColors;
    IMRGBA *hc = im->style.hotColors;

    bc[IMStyle_background] = MakeIMRGBA(22, 25, 28, 255);
    hc[IMStyle_background] = MakeIMRGBA(22, 25, 28, 255);

    bc[IMStyle_panel] = MakeIMRGBA(35, 38, 40, 255);
    hc[IMStyle_panel] = MakeIMRGBA(33, 41, 50, 255);

    bc[IMStyle_panelDark] = MakeIMRGBA(35, 32, 32, 255);
    hc[IMStyle_panelDark] = MakeIMRGBA(45, 45, 45, 255);

    bc[IMStyle_panelBright] = MakeIMRGBA(85, 80, 80, 255);
    hc[IMStyle_panelBright] = MakeIMRGBA(120, 100, 100, 255);

    bc[IMStyle_border] = MakeIMRGBA(60, 70, 70, 255);
    hc[IMStyle_border] = MakeIMRGBA(200, 100, 50, 255);

    bc[IMStyle_test] = MakeIMRGBA(240, 50, 50, 255);
    hc[IMStyle_test] = MakeIMRGBA(240, 70, 40, 255);
}

void IM_AddHotAction(IMContext *im, int32_t id, void *func)
{
    int success = IM_InsertFunc(&im->hotFuncTable, id, func);
    assert(success == 0);
}

void IM_AddActiveAction(IMContext *im, int32_t id, void *func)
{
    int success = IM_InsertFunc(&im->activeFuncTable, id, func);
    assert(success == 0);
}

void IM_InitFunctions(IMContext *im)
{

    IM_AddHotAction(im, IM_intField, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_floatField, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_stringField, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_toggle, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_funcOnRelease, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_funcOnReleaseCustom, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_window, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_resizeHandle, &IM_DefaultHotAction);
    IM_AddHotAction(im, IM_funcOnPress, &IM_FuncOnPressSimpleAction);
    IM_AddHotAction(im, IM_funcOnPressCustom, &IM_FuncOnPressAction);

    IM_AddActiveAction(im, IM_intField, &IM_IntFieldAction);
    IM_AddActiveAction(im, IM_floatField, &IM_FloatFieldAction);
    IM_AddActiveAction(im, IM_stringField, &IM_StringFieldAction);
    IM_AddActiveAction(im, IM_toggle, IM_ToggleAction);
    IM_AddActiveAction(im, IM_funcOnRelease, IM_FuncSimpleAction);
    IM_AddActiveAction(im, IM_funcOnReleaseCustom, IM_FuncAction);
    IM_AddActiveAction(im, IM_window, &IM_WindowAction);
    IM_AddActiveAction(im, IM_resizeHandle, &IM_ResizeHandleAction);
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void IM_MakeFontBitmap(IMContext *im)
{
    int bitmapDim = 512;
    im->fontBitmap = (uint8_t *)malloc(bitmapDim * bitmapDim);
    uint8_t *ttf_buffer = (uint8_t *)malloc(1 << 20);
    fread(ttf_buffer, 1, 1 << 20, fopen("c:/windows/fonts/LiberationMono-Regular.ttf", "rb"));

    stbtt_pack_range fontRanges[NUM_FONT_SIZES] = {0};
    float fontSize = 1.f;
    for (int i = 0; i < NUM_FONT_SIZES; i++)
    {
        im->fonts[0].sizes[i].size = fontSize;
        fontRanges[i].font_size = fontSize;
        fontSize += 1.f;
        fontRanges[i].first_unicode_codepoint_in_range = 32;
        fontRanges[i].num_chars = NUM_FONT_GLYPHS;
        fontRanges[i].chardata_for_range = im->fonts[0].sizes[i].fontData;
    }

    stbtt_pack_context context;
    int numSizes = NUM_FONT_SIZES;
    int tryPack = 1;
    while (tryPack)
    {
        stbtt_PackBegin(&context, im->fontBitmap, bitmapDim, bitmapDim, 0, 1, NULL);
        stbtt_PackSetOversampling(&context, 1, 1);
        int packResult = stbtt_PackFontRanges(&context, ttf_buffer, 0, fontRanges, numSizes);
        if (packResult == 0)
        {
            numSizes--;
        }
        else
        {
            tryPack = 0;
        }
    }
    im->fonts[0].fontSizeCount = numSizes;
    stbtt_PackEnd(&context);
    free(ttf_buffer);
    stbi_write_png("fontBitmap.png", bitmapDim, bitmapDim, 1, im->fontBitmap, 0);

    /*int triListSize = MAX_GLYPH_INSTANCES * 6 * sizeof(uint16);
    uint16 *triList = (uint16 *)malloc(triListSize);
    for (int i = 0; i < MAX_GLYPH_INSTANCES; i++)
    {
        triList[i * 6 + 0] = i * 4 + 0;
        triList[i * 6 + 1] = i * 4 + 1;
        triList[i * 6 + 2] = i * 4 + 2;
        triList[i * 6 + 3] = i * 4 + 2;
        triList[i * 6 + 4] = i * 4 + 1;
        triList[i * 6 + 5] = i * 4 + 3;
    }*/
}

void IM_InitContext(IMContext *im)
{
    IM_InitFunctions(im);
    IM_InitStyle(im);
    IM_MakeFontBitmap(im);
}