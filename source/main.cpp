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

		auto last_frame = std::chrono::high_resolution_clock::now();

		float r = 0.0f;
		bool dir = true;
		float speed = 0.5f;
		// Main loop
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

			if (dir) {
				r += delta * speed;
				if (r >= 1.0f) {
					r = 1.0f;
					dir = false;
				}
			} else {
				r -= delta * speed;
				if (r <= 0.0f) {
					r = 0.0f;
					dir = true;
				}
			}

			glClearColor(r, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			eglSwapBuffers(egl_display, egl_surface);
		}

		CleanupEGL();
	}

#ifdef DEBUG
	socketExit();
#endif
	return 0;
}