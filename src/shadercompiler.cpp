#include "shadercompiler.h"
#include "helpers.h"

#include <stdexcept>
#include <vector>
#include <iostream>

namespace {
	std::string processLine(const std::string& line, bool addLine) {
		auto includePos = line.find("#include");
		if (includePos != std::string::npos) {
			std::string includeFileName;

			bool foundStart = false;
			for (std::size_t i = includePos; i < line.size(); i++) {
				if (line[i] == '"') {
					if (!foundStart) {
						foundStart = true;
					} else {
						break;
					}
				} else if (foundStart) {
					includeFileName += line[i];
				}
			}

			return Helpers::readFileAsText(includeFileName);
		}

		return line + (addLine ? "\n" : "");
	}
}

std::string ShaderCompiler::preProcessShader(const std::string& content) {
	std::string outputContent;

	auto workingContent = content;
	std::string delimiter = "\n";

	size_t pos = 0;
	std::string token;
	while ((pos = workingContent.find(delimiter)) != std::string::npos) {
		token = workingContent.substr(0, pos);
		outputContent += processLine(token, true);
		workingContent.erase(0, pos + delimiter.length());
	}
	outputContent += processLine(workingContent, false);

	return outputContent;
}

GLuint ShaderCompiler::loadAndCompileShader(const std::string& content, GLenum shaderType) {
	auto processedContent = preProcessShader(content);

	auto contentPtr = processedContent.c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &contentPtr, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != 1) {
		const std::size_t MAX_LENGTH = 1024;
		char buffer[MAX_LENGTH];
		GLsizei length;
		glGetShaderInfoLog(shader, MAX_LENGTH, &length, buffer);
		std::string errorMessage(buffer, length);

		throw std::runtime_error("Failed to compile shader due to: " + errorMessage);
	}

	return shader;
}

GLuint ShaderCompiler::linkShaders(GLuint vertexShader, GLuint fragmentShader) {
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outputColor");
	glLinkProgram(shaderProgram);

	return shaderProgram;
}
