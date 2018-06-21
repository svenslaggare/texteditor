#pragma once
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

/**
 * Represents a shader compiler
 */
namespace ShaderCompiler {
	/**
	 * Pre-process the given shader
	 * @param content The content of the shader
	 * @return The output of the pre-processor
	 */
	std::string preProcessShader(const std::string& content);

	/**
	 * Loads and compiles the given shader
	 * @param content The content of the shader
	 * @param shaderType The type of the shader
	 * @return The compiled shader
	 */
	GLuint loadAndCompileShader(const std::string& content, GLenum shaderType);

	/**
	 * Links the given vertex and fragment shader into a shader program
	 * @param vertexShader The vertex shader
	 * @param fragmentShader The fragment shader
	 * @return The shader program
	 */
	GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader);
}