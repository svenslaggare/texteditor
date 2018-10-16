#include "shaderprogram.h"
#include "shadercompiler.h"

ShaderParameter ShaderParameter::textureParameter(std::string name, GLuint textureId) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Texture2D;
	parameter.value.glUintValue = textureId;
	return parameter;
}

ShaderParameter ShaderParameter::floatParameter(std::string name, float value) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Float;
	parameter.value.floatValue = value;
	return parameter;
}

ShaderParameter ShaderParameter::float2Parameter(std::string name, float x, float y) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Float2;
	parameter.value.float2Value[0] = x;
	parameter.value.float2Value[1] = y;
	return parameter;
}

ShaderParameter ShaderParameter::float3Parameter(std::string name, float x, float y, float z) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Float3;
	parameter.value.float2Value[0] = x;
	parameter.value.float2Value[1] = y;
	parameter.value.float2Value[2] = z;
	return parameter;
}

ShaderParameter ShaderParameter::float4Parameter(std::string name, float x, float y, float z, float w) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Float4;
	parameter.value.float2Value[0] = x;
	parameter.value.float2Value[1] = y;
	parameter.value.float2Value[2] = z;
	parameter.value.float2Value[3] = w;
	return parameter;
}

ShaderParameter ShaderParameter::float4x4MatrixParameter(std::string name, const glm::mat4x4& matrix) {
	ShaderParameter parameter;
	parameter.name = std::move(name);
	parameter.type = ShaderParameterType::Float4x4Matrix;
	memcpy(parameter.value.float4x4MatrixValue, &matrix[0][0], sizeof(float) * 4 * 4);
	return parameter;
}

ShaderProgram::ShaderProgram(const std::string& vertexShaderContent, const std::string& fragmentShaderContent) {
	mVertexShader = ShaderCompiler::loadAndCompileShader(vertexShaderContent, GL_VERTEX_SHADER);
	mFragmentShader = ShaderCompiler::loadAndCompileShader(fragmentShaderContent, GL_FRAGMENT_SHADER);
	mShaderProgram = ShaderCompiler::linkShaders(mVertexShader, mFragmentShader);
}

ShaderProgram::~ShaderProgram() {
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);
	glDeleteProgram(mShaderProgram);
}

GLuint ShaderProgram::id() const {
	return mShaderProgram;
}

GLint ShaderProgram::getUniform(const std::string& name, bool throwOnNotExist) {
	auto uniformIterator = mUniforms.find(name);
	if (uniformIterator != mUniforms.end()) {
		return uniformIterator->second;
	}

	auto uniform = glGetUniformLocation(mShaderProgram, name.c_str());
	if (uniform == -1) {
		if (throwOnNotExist) {
			throw std::runtime_error("'" + name + "' is not a valid uniform.");
		}
	} else {
		return uniform;
	}

	mUniforms.insert({ name, uniform });
	return uniform;
}

void ShaderProgram::setParameters(const std::vector<ShaderParameter>& parameters) {
	int textureNumber = 0;
	for (auto& parameter : parameters) {
		auto uniform = getUniform(parameter.name);

		switch (parameter.type) {
			case ShaderParameterType::Unknown:
				throw std::runtime_error("Unknown parameter type.");
			case ShaderParameterType::Texture2D:
				glActiveTexture((GLenum) (GL_TEXTURE0 + textureNumber));
				glBindTexture(GL_TEXTURE_2D, parameter.value.glUintValue);
				glUniform1i(uniform, textureNumber);
				textureNumber++;
				break;
			case ShaderParameterType::Float:
				glUniform1f(uniform, parameter.value.floatValue);
				break;
			case ShaderParameterType::Float2:
				glUniform2f(uniform, parameter.value.float2Value[0], parameter.value.float2Value[1]);
				break;
			case ShaderParameterType::Float3:
				glUniform3f(
					uniform,
					parameter.value.float2Value[0],
					parameter.value.float2Value[1],
					parameter.value.float2Value[2]);
				break;
			case ShaderParameterType::Float4:
				glUniform4f(
					uniform,
					parameter.value.float2Value[0],
					parameter.value.float2Value[1],
					parameter.value.float2Value[2],
					parameter.value.float2Value[3]);
				break;
			case ShaderParameterType::Float4x4Matrix:
				glUniformMatrix4fv(uniform, 1, GL_FALSE, parameter.value.float4x4MatrixValue);
				break;
		}
	}
}