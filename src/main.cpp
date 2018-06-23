#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "rendering/shadercompiler.h"
#include "helpers.h"
#include "rendering/glhelpers.h"
#include "rendering/font.h"
#include "rendering/renderstyle.h"
#include "rendering/textrender.h"
#include "rendering/framebuffer.h"
#include "rendering/textview.h"
#include "rendering/renderviewport.h"
#include "text/text.h"
#include "windowstate.h"
#include "inputmanager.h"
#include "rendering/texturerender.h"
#include "text/textformatter.h"

static WindowState windowState;

int main(int argc, char* argv[]) {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	auto window = glfwCreateWindow(windowState.width, windowState.height, "TextEditor", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowSizeCallback(window, [&](GLFWwindow* window, int width, int height) {
		windowState.width = width;
		windowState.height = height;
		windowState.changed = true;
		glViewport(0, 0, width, height);
	});

	glfwSetScrollCallback(window, [&](GLFWwindow* window,double offsetX, double offsetY) {
		windowState.scrollY = offsetY;
		windowState.scrolled = true;
	});

	// Compile and link shaders
	auto textVertexShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsText("shaders/textVertex.glsl"), GL_VERTEX_SHADER);
	auto textFragmentShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsText("shaders/text.glsl"), GL_FRAGMENT_SHADER);
	auto textProgram = ShaderCompiler::linkShaders(textVertexShader, textFragmentShader);

	glUseProgram(textProgram);
	glUniform1i(glGetUniformLocation(textProgram, "inputTexture"), 0);
	auto projection = glm::ortho(0.0f, (float)windowState.width, -(float)windowState.height, 0.0f + 0);
	glUniformMatrix4fv(glGetUniformLocation(textProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

	//auto fontName = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
	auto fontName = "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf";

//	Font font(fontName, 28);
	Font font(fontName, 16);
//	Font font(fontName, 8);
	TextRender textRender(textProgram);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto wololo = "4	\t44\""; 5.0f;

//  std::string text = "hello	world\nmy friend";
//	std::string inlineText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse eu enim vitae erat viverra dignissim. Cras congue hendrerit eleifend. Curabitur augue mauris, rutrum laoreet mollis in, laoreet in lacus. Phasellus laoreet lectus quis magna convallis elementum. Donec tempus, nibh vitae ultricies molestie, ex enim porttitor dolor, vel dignissim mauris massa vitae leo.\n\nUt varius semper eros, et gravida nulla maximus sed. Phasellus viverra libero at dignissim mollis. Fusce consectetur porta volutpat. Pellentesque aliquam pharetra convallis. Nunc at elit quam. Proin urna lorem, bibendum vitae consectetur nec, rhoncus at magna. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus odio sapien, scelerisque quis facilisis quis, suscipit eget ligula. Morbi vitae elementum leo. Nullam mollis est quis consectetur finibus. Suspendisse vehicula purus vel libero auctor ultrices. Ut scelerisque quam ante, vitae tincidunt velit laoreet et. Quisque tellus urna, pellentesque nec tincidunt in, semper et nisl. Proin id euismod lorem. Nam nec lorem ornare, blandit arcu nec, pulvinar justo. Curabitur nec rutrum risus. Aenean maximus turpis sit amet risus blandit vehicula. Vestibulum tincidunt odio lectus, non viverra nibh mattis non. Mauris vitae ex posuere diam tincidunt accumsan.\n\nQuisque pretium tortor vitae diam suscipit pellentesque at ac erat. In hac habitasse platea dictumst. Proin sodales dapibus pretium. Duis id eros nulla. Ut hendrerit vehicula lobortis. Etiam sagittis porta dui, vel porttitor tellus scelerisque vel. Quisque cursus rhoncus velit, quis volutpat velit volutpat sit amet. Sed quis molestie libero, eu sollicitudin felis.\n\nNunc tempor eu leo non fermentum. Donec bibendum et lectus vel malesuada. Vivamus malesuada eros nibh, id faucibus eros elementum at. Integer ac sem lorem. Curabitur luctus feugiat justo. Nulla at tortor sit amet orci cursus ornare non id massa. Vivamus vel lacus feugiat, vehicula urna dignissim, blandit lectus. Morbi non urna dui. Morbi ac libero a ligula varius lobortis. Proin at purus eget velit mattis viverra vel sed quam. Duis sollicitudin massa magna, viverra fringilla arcu ultricies eget. Suspendisse potenti. Morbi turpis felis, pellentesque id pretium vel, euismod non eros. Nulla mauris ipsum, interdum at leo aliquam, porttitor condimentum nunc. Cras semper feugiat hendrerit.";

//	Text text(Helpers::readFileAsText("data/lorem.txt"));
	Text text(Helpers::readFileAsText("data/gc.cpp"));
//	Text text(Helpers::readFileAsText("data/test.cpp"));
//	Text text(Helpers::readFileAsText("src/main.cpp"));
//	Text text(Helpers::readFileAsText("/home/antjans/curve.py"));

//	std::chrono::time_point<std::chrono::system_clock> startTime;
	auto startTime = std::chrono::system_clock::now();
	int numFrames = 0;
	float fps = 0.0f;

	auto frameBuffer = std::make_unique<FrameBuffer>(windowState.width, windowState.height);

	auto passthroughVertexShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsText("shaders/vertex.glsl"), GL_VERTEX_SHADER);
	auto passthroughFragmentShader = ShaderCompiler::loadAndCompileShader(Helpers::readFileAsText("shaders/passthrough.glsl"), GL_FRAGMENT_SHADER);
	auto passthroughProgram = ShaderCompiler::linkShaders(passthroughVertexShader, passthroughFragmentShader);

	InputManager inputManager(window);

	RenderStyle renderStyle;
	RenderViewPort renderViewPort { glm::vec2(0, 0), windowState.width, windowState.height };

	TextView codeTextView(font, FormatMode::Code, renderViewPort, renderStyle);
	TextureRender frameBufferRender(passthroughProgram);

	while (!glfwWindowShouldClose(window)) {
		if (windowState.changed) {
			renderViewPort = RenderViewPort { glm::vec2(), windowState.width, windowState.height };

			projection = glm::ortho(0.0f, (float)windowState.width, -(float)windowState.height, 0.0f);
			glUseProgram(textProgram);
			glUniformMatrix4fv(glGetUniformLocation(textProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

//			frameBuffer = std::make_unique<FrameBuffer>(windowState.width, windowState.height);

			windowState.changed = false;
		}

		inputManager.update(windowState);

		// Draw text to frame buffer
//		frameBuffer->bind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(renderStyle.backgroundColor.r, renderStyle.backgroundColor.g, renderStyle.backgroundColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		codeTextView.render(textRender, text, inputManager.cursorPosition());

		// Draw frame buffer to screen
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		frameBufferRender.render(frameBuffer->textureColorBuffer());

		// Swap
		glfwSwapBuffers(window);
		glfwPollEvents();

		numFrames++;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
		if (duration >= 1000) {
			fps = numFrames / (duration * 0.001f);
			startTime = std::chrono::system_clock::now();
			numFrames = 0;
		}

//		std::cout << "\rFPS: " << fps << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
	}

	glfwTerminate();
}