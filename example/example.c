#include "stdio.h"
#include "math.h"
#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "glad/glad.c"

#include "imgui.h"
#include "imgui_drawBuffer.c"
#include "imgui.c"
#include "imgui_gl3.c"

typedef struct
{
	int pressStarted;
	int pressEnded;
	int down;
} KeyState;

typedef struct
{
	int32_t mouseX;
	int32_t mouseY;
	int32_t mouseDeltaX;
	int32_t mouseDeltaY;
	int32_t scrollX;
	int32_t scrollY;

	KeyState m1;
	KeyState m2;
	KeyState m3;
	KeyState esc;
	KeyState enter;

	int backspaceCount;
	int deleteCount;
	char textBuffer[512];
} InputState;

typedef struct
{
	SDL_Window *window;
	int windowWidth;
	int windowHeight;
	SDL_GLContext context;
} WindowState;

inline float
randf(uint32_t *state)
{
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	x = 0x3f800000 | (x & 0x007fffff);
	float result;
	memcpy(&result, &x, sizeof(float));
	return result - 1.f;
}

uint32_t InitRandfState()
{
	static uint32_t internalState = 79019823;
	internalState ^= internalState << 13;
	internalState ^= internalState >> 15;
	internalState ^= internalState << 3;
	return internalState;
}

void GetInput(InputState *input, WindowState *window, int *continueRunning, float dt)
{
	input->mouseDeltaX = 0;
	input->mouseDeltaY = 0;

	input->m1.pressStarted = 0;
	input->m1.pressEnded = 0;
	input->m2.pressStarted = 0;
	input->m2.pressEnded = 0;
	input->m3.pressStarted = 0;
	input->m3.pressEnded = 0;
	input->esc.pressStarted = 0;
	input->esc.pressEnded = 0;
	input->enter.pressStarted = 0;
	input->enter.pressEnded = 0;

	input->backspaceCount = 0;
	input->deleteCount = 0;
	memset(input->textBuffer, 0, sizeof(input->textBuffer));

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			*continueRunning = 0;
		}
		if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				input->esc.pressStarted = 1;
				input->esc.down = 1;
			}
			if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
			{
				input->enter.pressStarted = 1;
				input->enter.down = 1;
			}
			else if (event.key.keysym.sym == SDLK_BACKSPACE)
			{
				input->backspaceCount++;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				input->esc.pressEnded = 1;
				input->esc.down = 0;
			}
			if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
			{
				input->enter.pressEnded = 1;
				input->enter.down = 0;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				input->m1.pressStarted = 1;
				input->m1.down = 1;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT)
			{
				input->m2.pressStarted = 1;
				input->m2.down = 1;
			}
			else if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				input->m3.pressStarted = 1;
				input->m3.down = 1;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				input->m1.pressEnded = 1;
				input->m1.down = 0;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT)
			{
				input->m2.pressEnded = 1;
				input->m2.down = 0;
			}
			else if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				input->m3.pressEnded = 1;
				input->m3.down = 0;
			}
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
			input->scrollX = event.wheel.x;
			input->scrollY = event.wheel.y;
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			input->mouseX = event.motion.x;
			input->mouseY = event.motion.y;
			input->mouseDeltaX = event.motion.xrel;
			input->mouseDeltaY = event.motion.yrel;
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				window->windowWidth = event.window.data1;
				window->windowHeight = event.window.data2;
				SDL_SetWindowSize(window->window, event.window.data1, event.window.data2);
				glViewport(0, 0, window->windowWidth, window->windowHeight);
			}
		}
		else if (event.type == SDL_TEXTINPUT)
		{
			strcat(input->textBuffer, event.text.text);
		}
	}
}

int main(int argc, char *argv[])
{
	WindowState window = {NULL};
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_Rect dispRect;
	int DispResult = SDL_GetDisplayUsableBounds(0, &dispRect);
	window.windowWidth = dispRect.w - 50;
	window.windowHeight = dispRect.h - 100;
	window.window = SDL_CreateWindow("imgui example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window.windowWidth, window.windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	window.context = SDL_GL_CreateContext(window.window);
	SDL_GL_SetSwapInterval(1); // 0 = immediate, 1 = v-sync
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	IMContext im;
	IM_InitContext(&im);
	GL_State gl_state;
	InitGlState(&im, &gl_state);

	InputState input;
	memset(&input, 0, sizeof(input));

	IMWindowState testWindowState = {0};
	testWindowState.x = 100.f;
	testWindowState.y = 100.f;
	testWindowState.w = 800.f;
	testWindowState.h = 800.f;
	testWindowState.w_min = 200.f;
	testWindowState.h_min = 200.f;
	testWindowState.w_max = 99999.f;
	testWindowState.h_max = 99999.f;
	testWindowState.zoom = 1.f;
	testWindowState.anchoring = IM_free;
	testWindowState.flags = (IMFlag_draggable | IMFlag_resizable);
	testWindowState.title = "Test Window!";

	int spline1 = 1;
	int spline2 = 0;
	int spline3 = 0;

#define SPLINE_POINTS 20
	uint32_t randState = 92534;
	float points[2 * SPLINE_POINTS] = {0.f};
	float pmax_x = 0.f;
	float pmax_y = 0.f;
	for (int i = 0; i < SPLINE_POINTS; i++)
	{
		points[i * 2] = i + randf(&randState);
		points[i * 2 + 1] = randf(&randState);
		pmax_x = IM_MAX(pmax_x, points[i * 2]);
		pmax_y = IM_MAX(pmax_y, points[i * 2 + 1]);
	}

	uint64_t perfFreq = SDL_GetPerformanceFrequency();
	uint64_t lastPerfCount = SDL_GetPerformanceCounter();
	float frameTime = 0.f;
	float frame = 1.f;
	int isRunning = 1;
	while (isRunning)
	{
		uint64_t currentPerfCount = SDL_GetPerformanceCounter();
		float currentFrameTime = (float)(currentPerfCount - lastPerfCount) * 1000.f / perfFreq;
		lastPerfCount = currentPerfCount;
		frameTime = 0.9 * frameTime + 0.1 * currentFrameTime;
		char timeString[64];
		snprintf(timeString, 64, "%f ms", frameTime);
		timeString[63] = 0;

		GetInput(&input, &window, &isRunning, 0.f);
		IMNewFrame(&im, window.windowWidth, window.windowHeight);
		IMUpdateInput(&im,
					  input.mouseX,
					  input.mouseY,
					  input.mouseDeltaX,
					  input.mouseDeltaY,
					  input.scrollX,
					  input.scrollY,
					  input.esc.pressStarted,
					  input.esc.pressEnded,
					  input.enter.pressStarted,
					  input.enter.pressEnded,
					  input.m1.pressStarted,
					  input.m1.pressEnded,
					  input.m2.pressStarted,
					  input.m2.pressEnded,
					  input.m3.pressStarted,
					  input.m3.pressEnded);

#if 0
		IMRGBA spiralColor = {240, 10, 20, 255};
		float spiralX = 300.f;
		float spiralY = 500.f;
		IMV3 spiralPoint = {spiralX + 50.f, spiralY, 10.f};
		StartTriangleStrip(&im, spiralPoint, 6.f, spiralColor);
		for (float i = 0.f, j = 2.f; i < 300.f; i++, j *= (j * 1.01f))
		{
			spiralPoint.x = spiralX + (50.f + i) * cosf(i * 0.1);
			spiralPoint.y = spiralY + (50.f + i) * sinf(i * 0.1);
			PushTriangleStrip(&im, spiralPoint, 6.f, spiralColor);
		}

		spiralX = 800.f;
		spiralPoint.x = spiralX + 50.f;
		spiralPoint.y = spiralY;
		StartTriangleStrip(&im, spiralPoint, 6.f, spiralColor);
		for (float i = 0.f, j = 2.f; i < 300.f; i++, j *= (j * 1.01f))
		{
			spiralPoint.x = spiralX + (50.f + i) * cosf(i * 0.1);
			spiralPoint.y = spiralY + (50.f + i) * sinf(i * 0.1);
			PushTriangleStrip(&im, spiralPoint, 6.f, spiralColor);
		}
#endif

		IMPanel root = IMMakePanel(IM_top, 0, 0, 0.f, 0.f, window.windowWidth, window.windowHeight);
		IMPanel topPanel = IMMakeSubPanel(&root, IM_top, 20.f);
		IMDrawContainer(&im, &topPanel, NULL, 0, 1);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, timeString);

#if 1
		//IMPushScissor(&im, {(float)im.input.mx - 100.f, (float)im.input.mx + 100.f, (float)im.input.my - 100.f, (float)im.input.my + 100.f});
		IMPanel leftPanel = IMMakeSubPanel(&root, IM_left, 100.f);
		IMDrawContainer(&im, &leftPanel, NULL, 0, 1);
		IMDoLabel(&im, &leftPanel, IM_top, 20.f, "Float Sliders:");
		IMFloatInfo floatInfo = {0.f, 1.f, NULL};
		for (int i = 0; i < SPLINE_POINTS; i++)
		{
			IMDoFloatSlider(&im, &leftPanel, &points[i * 2 + 1], &floatInfo);
		}

		IMPanel testWindowPanel = IMStartWindowPanel(&im, &root, &testWindowState);
		IMDoLabel(&im, &testWindowPanel, IM_top, 20.f, "Toggles:");
		IMDoToggle(&im, &testWindowPanel, IM_top, 20.f, "Smoothstep", &spline1);
		IMDoToggle(&im, &testWindowPanel, IM_top, 20.f, "Cubic XY", &spline2);
		IMDoToggle(&im, &testWindowPanel, IM_top, 20.f, "Paniq", &spline3);
		IMDoLabel(&im, &testWindowPanel, IM_top, 20.f, "Spline:");

#if 0
		float points[4] = {0.f, 0.f, 1.f, 1.f};
		float pmax_x = 1.f;
		float pmax_y = 1.f;
		IMV4 splineRange = {0.f, pmax_x, 0.f, pmax_y};
		IMV4 splineRect = {50.f, 1000.f, 50.f, 800.f};
		IMRGBA splineColor = {220, 80, 20, 255};
		PushSpline(&im, testWindowPanel.freeBounds, testWindowPanel.depth + 1.f, splineColor, splineRange, 2, points);
#else
		pmax_x = 0.f;
		pmax_y = 0.f;
		for (int i = 0; i < 20; i++)
		{
			pmax_x = IM_MAX(pmax_x, points[i * 2]);
			//pmax_y = IM_MAX(pmax_y, points[i * 2 + 1]);
		}
		IMV4 splineRange = {0.f, pmax_x, 0.f, 1.f};
		IMV4 splineRect = {50.f, 1000.f, 50.f, 800.f};
		IMRGBA splineColor = {220, 80, 20, 255};
		if (spline1)
		{
			PushSmoothstepSpline(&im, testWindowPanel.freeBounds, testWindowPanel.depth + 1.f, splineColor, splineRange, 20, points);
		}
		else if (spline2)
		{

			PushCubicSpline1(&im, testWindowPanel.freeBounds, testWindowPanel.depth + 1.f, splineColor, splineRange, 20, points);
		}
		else if (spline3)
		{
			PushCubicSpline2(&im, testWindowPanel.freeBounds, testWindowPanel.depth + 1.f, splineColor, splineRange, 20, points);
		}
#endif

		/* 		IMRGBA c1 = MakeIMRGBA(200, 100, 20, 0);
		IMRGBA c2 = MakeIMRGBA(20, 100, 200, 255);
		IMV2 splineThickness = {2.f, 2.f};
		PushRect(&im, splineRect, splineThickness, 500.f, c1, c2); */

		IMFinishWindowPanel(&im, &testWindowPanel, &testWindowState);

		IMProcessInput(&im);
#endif

#if 0
		IMRGBA c1 = MakeIMRGBA(200, 100, 20, 255);
		IMRGBA c2 = MakeIMRGBA(20, 100, 200, 255);
		int rectCount = 0;
		IMV2 rectPos = {30.f, 30.f};
		IMV2 rectDim = {10.f, 10.f};
		IMV2 rectThickness = {2.f, 2.f};
		for (float i = 0.f; i < 1800.f; i += 18.f)
		{
			rectPos.y = 30.f;
			for (float j = 0.f; j < 1000.f; j += 20.f)
			{
				PushRect(&im, rectPos, rectDim, rectThickness, 500.f, c1, c2);
				rectPos.y += 20.f;
				rectCount++;
			}
			rectPos.x += 18.f;
		}
		char rectCountString[64];
		snprintf(rectCountString, 64, "Rects: %i", rectCount);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, rectCountString);
#endif

		char triCountString[64];
		snprintf(triCountString, 64, "General Tris: %i", im.drawBuffer.triCount);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, triCountString);
		snprintf(triCountString, 64, "Strip Tris: %i", im.drawBuffer.triStripVertCount);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, triCountString);
		snprintf(triCountString, 64, "PSC Indexes: %i", im.drawBuffer.PSCElementCount);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, triCountString);
		snprintf(triCountString, 64, "PSC Verts: %i", im.drawBuffer.PSCVertCount);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, triCountString);
		snprintf(triCountString, 64, "Font Tris: %i", im.drawBuffer.glyphCount * 2);
		IMDoLabel(&im, &topPanel, IM_left, 200.f, triCountString);

#if 0
		for (int i = 0; i < 40; i++)
		{
			for (int j = 0; j < 30; j++)
			{
				PushStr(&im, {50.f * (float)i, 30.f + 30.f * (float)j, 1000.f}, Font_top, Font_left, 0, {20, 10, 20, 255}, 100.f / (float)(i + 1), "Test");
			}
		}
#endif

		glClearColor(0.1, 0.5f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawContext(&im, &gl_state);
		SDL_GL_SwapWindow(window.window);

		frame += 0.01f;
	}

	return 0;
}