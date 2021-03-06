#include "fix_vscode.h"

// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>

// Include the main libnx system header, for Switch development
#include <switch.h>


#include <EGL/egl.h> // EGL Library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h> // OpenGL loader

#include "ttt/solver.hpp"
#include "gl/tile_renderer.hpp"
#include <cmath>
#include "Base_png.h"
#include "Cross_png.h"
#include "Circle_png.h"

constexpr glm::vec4 selectionColor(1.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 crossColor(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 circleColor(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 emptyColor(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 errorColor(0.0f, 0.0f, 0.0f, 1.0f);

EGLDisplay egl_display;
EGLContext egl_context;
EGLSurface egl_surface;

int InitEGL(NWindow* win) {
	egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!egl_display) {
		printf("Error: Failed to open EGL display: %d", eglGetError());
		return 0;
	}

	eglInitialize(egl_display, NULL, NULL);

	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
		printf("Error: Failed to bind OpenGL api: %d", eglGetError());
		eglTerminate(egl_display);
		egl_display = NULL;
		return 0;
	}

	EGLConfig config;
	EGLint numConfigs;
	EGLint frameBufferAtrribList[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};

	eglChooseConfig(egl_display, frameBufferAtrribList, &config, 1, &numConfigs);
	if (numConfigs == 0) {
		printf("Error: No EGL config found: %d", eglGetError());
		eglTerminate(egl_display);
		egl_display = NULL;
		return 0;
	}

	egl_surface = eglCreateWindowSurface(egl_display, config, win, NULL);
	if (!egl_surface) {
		printf("Error: Failed to create EGL surface: %d", eglGetError());
		eglTerminate(egl_display);
		egl_display = NULL;
		return 0;
	}

	EGLint contextAttribsList[] = {
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
		EGL_CONTEXT_MAJOR_VERSION, 4,
		EGL_CONTEXT_MINOR_VERSION, 3,
		EGL_NONE
	};
	egl_context = eglCreateContext(egl_display, config, NULL, contextAttribsList);
	if (!egl_context) {
		printf("Error: Failed to create EGL context: %d", eglGetError());
		eglDestroySurface(egl_display, egl_surface);
		egl_surface = NULL;
		eglTerminate(egl_display);
		egl_display = NULL;
		return 0;
	}

	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
	return 1;
}

void glDebugCB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string src = "[SRC_UNKNOWN]";
	switch(source) {
		case GL_DEBUG_SOURCE_API:
			src = "[SRC_API]";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			src = "[SRC_APP]";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			src = "[SRC_OTHER]";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			src = "[SRC_SHADER]";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			src = "[SRC_3RD]";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			src = "[SRC_WINDOW]";
			break;
	}

	std::string tp = "[TYPE_UNKNOWN]";
	switch(type) {
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			tp = "[TYPE_DEPRECATED";
			break;
		case GL_DEBUG_TYPE_ERROR:
			tp = "[TYPE_ERROR]";
			break;
		case GL_DEBUG_TYPE_MARKER:
			tp = "[TYPE_MARKER]";
			break;
		case GL_DEBUG_TYPE_OTHER:
			tp = "[TYPE_OTHER]";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			tp = "[TYPE_PERF]";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			tp = "[TYPE_POP]";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			tp = "[TYPE_PORT]";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			tp = "[TYPE_PUSH]";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			tp = "[TYPE_UB]";
			break;
	}

	std::string sev = "[SEV_UNKNOWN]";
	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			sev = "[SEV_HIGH]";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			sev = "[SEV_LOW]";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			sev = "[SEV_MEDIUM]";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			sev = "[SEV_NOTIF]";
			break;
	}

	std::cout << "[GLD]" << sev << tp << src << ": " << std::string(message, length) << std::endl;
}

void CleanupEGL() {
	if (egl_display) {
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (egl_context) {
			eglDestroyContext(egl_display, egl_context);
			egl_context = NULL;
		}
		if (egl_surface) {
			eglDestroySurface(egl_display, egl_surface);
			egl_surface = NULL;
		}
		eglTerminate(egl_display);
		egl_display = NULL;
	}
}

void setMesaConfig() {
#ifdef DEBUG
	setenv("MESA_NO_ERROR", "0", 1);

	setenv("EGL_LOG_LEVEL", "debug", 1);
	setenv("MESA_VERBOSE", "all", 1);
	setenv("MESA_DEBUG", "1", 1);
	setenv("NOUVEAU_MESA_DEBUG", "1", 1);

    setenv("NV50_PROG_OPTIMIZE", "0", 1);
    setenv("NV50_PROG_DEBUG", "1", 1);
    setenv("NV50_PROG_CHIPSET", "0x120", 1);
#else
	setenv("MESA_NO_ERROR", "1", 1);

	unsetenv("EGL_LOG_LEVEL");
	unsetenv("MESA_DEBUG");
	unsetenv("MESA_VERBOSE");
	unsetenv("NOUVEAU_MESA_DEBUG");

	unsetenv("NV50_PROG_OPTIMIZE");
    unsetenv("NV50_PROG_OPTIMIZE");
    unsetenv("NV50_PROG_DEBUG");
    unsetenv("NV50_PROG_CHIPSET");
#endif
}

// Main program entrypoint
int main(int argc, char* argv[])
{
	setMesaConfig();
	
	// Configure our supported input layout: a single player with standard controller styles
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);

	// Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
	PadState pad;
	padInitializeDefault(&pad);

#ifdef DEBUG
	socketInitializeDefault();
	nxlinkStdio();
#endif

	// Other initialization goes here. As a demonstration, we print hello world.
	printf("Hello World!\n");
#ifdef DEBUG
	fprintf(stderr, "Hello, Debug\n");
#endif

	if (InitEGL(nwindowGetDefault())) {

		gladLoadGL();

#ifdef DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(glDebugCB, nullptr);
#endif

		auto last_frame = std::chrono::high_resolution_clock::now();

		// Main loop

		u32 width;
		u32 height;
		nwindowGetDimensions(nwindowGetDefault(), &width, &height);

		glViewport(0, 0, width, height);
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
		float totalTime = 0;
		float waitStart = 0;
		bool waiting = false;
		int aiTurn = 0;

		ttt::Coord selectedCoord{ 1, 1 };
		ttt::Board board;


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		std::shared_ptr<gl::TileRenderer> render = std::make_shared<gl::TileRenderer>();
		std::shared_ptr<gl::Texture> cross_texture = std::make_shared<gl::Texture>();
		cross_texture->LoadPNG(Cross_png, Cross_png_size);
		std::shared_ptr<gl::Texture> circle_texture = std::make_shared<gl::Texture>();
		circle_texture->LoadPNG(Circle_png, Circle_png_size);
		std::shared_ptr<gl::Texture> empty_texture = std::make_shared<gl::Texture>();
		empty_texture->LoadPNG(Base_png, Base_png_size);
		while (appletMainLoop())
		{

			// Scan the gamepad. This should be done once for each frame
			padUpdate(&pad);

			// padGetButtonsDown returns the set of buttons that have been
			// newly pressed in this frame compared to the previous one
			u64 kDown = padGetButtonsDown(&pad);

			if (kDown & HidNpadButton_Plus)
				break; // break in order to return to hbmenu

			// Your code goes here

			auto now = std::chrono::high_resolution_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame).count() / 1000.0f;
			last_frame = now;
			totalTime += delta;
			
			float selectionOverlayAmount = glm::abs(glm::sin(totalTime * 3));

			int xMov = 0;
			int yMov = 0;
			bool applyClick = false;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			if (!waiting) {
				if (kDown & HidNpadButton_AnyRight) {
					xMov++;
				}

				if (kDown & HidNpadButton_AnyLeft) {
					xMov--;
				}

				if (kDown & HidNpadButton_AnyUp) {
					yMov++;
				}

				if (kDown & HidNpadButton_AnyDown) {
					yMov--;
				}

				if (kDown & HidNpadButton_A) {
					applyClick = true;
				}

				selectedCoord.x += xMov;
				selectedCoord.y += yMov;
				selectedCoord.Normalize();

				if (applyClick) {
					if(board.Set(selectedCoord, ttt::TileState::Circle)) {
						if (board.GetState() != ttt::BoardState::Regular) {
							waitStart = totalTime;
							waiting = true;
						}

						ttt::NextMove(board, aiTurn++);

						if (board.GetState() != ttt::BoardState::Regular) {
							waitStart = totalTime;
							waiting = true;
						}
					}
				}
			} else {
				if (totalTime - waitStart > 5) {
					board.Reset();
					waiting = false;
					aiTurn = 0;
				}
			}

			for(unsigned int x = 0; x < 3; x++) {
				for(unsigned int y = 0; y < 3; y++) {
					gl::Tile* t = render->Get(x, y);
					ttt::TileState state = board.Get(x, y);
					if (t == nullptr)
						continue;

					glm::vec4 baseColor = emptyColor;
					switch(state) {
					case ttt::TileState::Circle:
						baseColor = circleColor;
						t->texture = circle_texture;
						break;
					case ttt::TileState::Cross:
						baseColor = crossColor;
						t->texture = cross_texture;
						break;
					case ttt::TileState::Empty:
						baseColor = emptyColor;
						t->texture = empty_texture;
						break;
					case ttt::TileState::Invalid:
						baseColor = errorColor;
						t->texture = empty_texture;
						break;
					}

					if (selectedCoord.x == x && selectedCoord.y == y) {
						baseColor = glm::mix(baseColor, selectionColor, selectionOverlayAmount);
					}

					t->color = baseColor;
				}
			}

			render->Draw(glm::ivec2(width, height), 10);

			eglSwapBuffers(egl_display, egl_surface);
		}

		circle_texture = nullptr;
		cross_texture = nullptr;
		empty_texture = nullptr;
		render = nullptr;

		CleanupEGL();
	}

#ifdef DEBUG
	socketExit();
#endif
	return 0;
}