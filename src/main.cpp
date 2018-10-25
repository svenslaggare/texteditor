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

#include "rendering/renderviewport.h"
#include "windowstate.h"
#include "rendering/common/shaderprogram.h"
#include "text/text.h"
#include "helpers.h"
#include "rendering/textrender.h"
#include "rendering/texturerender.h"
#include "interface/textview.h"
#include "rendering/common/framebuffer.h"
#include "text/formatters/cpp.h"
#include "rendering/renderstyle.h"
#include "rendering/font.h"
#include "text/textloader.h"

RenderViewPort getViewPort(const WindowState& windowState) {
	return RenderViewPort { glm::vec2(0, 0), (float)windowState.width(), (float)windowState.height() };
}

void setProjection(ShaderProgram& shaderProgram, const WindowState& windowState) {
	auto projection = windowState.projection();
	glUseProgram(shaderProgram.id());
	shaderProgram.setParameters({ ShaderParameter::float4x4MatrixParameter("projection", projection) });
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: ./texteditor <filename>" << std::endl;
		std::exit(1);
	}

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	WindowState windowState;

	auto window = glfwCreateWindow(windowState.width(), windowState.height(), "TextEditor", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, &windowState),

	glewExperimental = GL_TRUE;
	glewInit();

	windowState.initialize(window);

	// Compile and link shaders
	ShaderProgram textProgram(
		Helpers::readFileAsUTF8Text("shaders/textVertex.glsl"),
		Helpers::readFileAsUTF8Text("shaders/text.glsl"));

	glUseProgram(textProgram.id());
	textProgram.setParameters({ ShaderParameter::textureParameter("inputTexture", 0) });
	setProjection(textProgram, windowState);

	auto fontName = "fonts/NotoMono-Regular.ttf";
	Font font(fontName, 16);

	TextRender textRender(textProgram.id());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto wololo = "4	\t44\""; 5.0f;

//  std::string text = "hello	world\nmy friend";
//  std::string inlineText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse eu enim vitae erat viverra dignissim. Cras congue hendrerit eleifend. Curabitur augue mauris, rutrum laoreet mollis in, laoreet in lacus. Phasellus laoreet lectus quis magna convallis elementum. Donec tempus, nibh vitae ultricies molestie, ex enim porttitor dolor, vel dignissim mauris massa vitae leo.\n\nUt varius semper eros, et gravida nulla maximus sed. Phasellus viverra libero at dignissim mollis. Fusce consectetur porta volutpat. Pellentesque aliquam pharetra convallis. Nunc at elit quam. Proin urna lorem, bibendum vitae consectetur nec, rhoncus at magna. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus odio sapien, scelerisque quis facilisis quis, suscipit eget ligula. Morbi vitae elementum leo. Nullam mollis est quis consectetur finibus. Suspendisse vehicula purus vel libero auctor ultrices. Ut scelerisque quam ante, vitae tincidunt velit laoreet et. Quisque tellus urna, pellentesque nec tincidunt in, semper et nisl. Proin id euismod lorem. Nam nec lorem ornare, blandit arcu nec, pulvinar justo. Curabitur nec rutrum risus. Aenean maximus turpis sit amet risus blandit vehicula. Vestibulum tincidunt odio lectus, non viverra nibh mattis non. Mauris vitae ex posuere diam tincidunt accumsan.\n\nQuisque pretium tortor vitae diam suscipit pellentesque at ac erat. In hac habitasse platea dictumst. Proin sodales dapibus pretium. Duis id eros nulla. Ut hendrerit vehicula lobortis. Etiam sagittis porta dui, vel porttitor tellus scelerisque vel. Quisque cursus rhoncus velit, quis volutpat velit volutpat sit amet. Sed quis molestie libero, eu sollicitudin felis.\n\nNunc tempor eu leo non fermentum. Donec bibendum et lectus vel malesuada. Vivamus malesuada eros nibh, id faucibus eros elementum at. Integer ac sem lorem. Curabitur luctus feugiat justo. Nulla at tortor sit amet orci cursus ornare non id massa. Vivamus vel lacus feugiat, vehicula urna dignissim, blandit lectus. Morbi non urna dui. Morbi ac libero a ligula varius lobortis. Proin at purus eget velit mattis viverra vel sed quam. Duis sollicitudin massa magna, viverra fringilla arcu ultricies eget. Suspendisse potenti. Morbi turpis felis, pellentesque id pretium vel, euismod non eros. Nulla mauris ipsum, interdum at leo aliquam, porttitor condimentum nunc. Cras semper feugiat hendrerit.";

	TextLoader textLoader;
//	auto loadedText = textLoader.load("data/gc.cpp");
//	auto loadedText = textLoader.load("src/main.cpp");
//	auto loadedText = textLoader.load("data/circle.py");

	auto loadedText = textLoader.load(argv[1]);

	auto startTime = Helpers::timeNow();
	int numFrames = 0;
	float fps = 0.0f;

	auto frameBuffer = std::make_unique<FrameBuffer>(windowState.width(), windowState.height());

	ShaderProgram passthroughProgram(
		Helpers::readFileAsUTF8Text("shaders/vertex.glsl"),
		Helpers::readFileAsUTF8Text("shaders/passthrough.glsl"));

	RenderStyle renderStyle;
	auto renderViewPort = getViewPort(windowState);

	TextView codeTextView(
		window,
		font,
		std::move(loadedText.rules),
		renderViewPort,
		renderStyle,
		loadedText.text);

	TextureRender frameBufferRender(passthroughProgram.id());

	while (!glfwWindowShouldClose(window)) {
		windowState.update();

		if (windowState.hasChangedWindowSize()) {
			renderViewPort = getViewPort(windowState);
			setProjection(textProgram, windowState);
			frameBuffer = std::make_unique<FrameBuffer>(windowState.width(), windowState.height());
		}

		// Draw text to frame buffer
//		frameBuffer->bind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(renderStyle.backgroundColor.r, renderStyle.backgroundColor.g, renderStyle.backgroundColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		codeTextView.update(windowState);
		codeTextView.render(windowState, textRender);

		// Draw frame buffer to screen
		/*
		 * glBindFramebuffer(GL_FRAMEBUFFER, 0);
		 *
		 * frameBufferRender.render(frameBuffer->textureColorBuffer());
		 * */

		// Swap
		glfwSwapBuffers(window);
		glfwPollEvents();

		numFrames++;
		auto duration = (float)Helpers::durationMilliseconds(Helpers::timeNow(), startTime);
		if (duration >= 1000) {
			fps = numFrames / (duration * 0.001f);
			startTime = Helpers::timeNow();
			numFrames = 0;
		}

		// std::cout << "\rFPS: " << fps << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
	}

	glfwTerminate();
}