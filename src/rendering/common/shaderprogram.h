#pragma once
#include <string>
#include <unordered_map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <vector>
#include <glm/mat4x4.hpp>

/**
 * The type of the shader parameter
 */
enum class ShaderParameterType {
	Unknown,
	Texture2D,
	Float,
	Float2,
	Float3,
	Float4,
	Float4x4Matrix
};

/**
 * Represents a shader parameter
 */
struct ShaderParameter {
	std::string name;
	ShaderParameterType type;

	union {
		GLuint glUintValue;
		float floatValue;
		float float2Value[2];
		float float3Value[3];
		float float4Value[4];
		float float4x4MatrixValue[4 * 4];
	} value;

	/**
	 * Creates a new texture parameter
	 * @param name The name of the parameter
	 * @param textureId The texture id
	 */
	static ShaderParameter textureParameter(std::string name, GLuint textureId);

	/**
	 * Creates a new float parameter
	 * @param name The name of the parameter
	 * @param value The value
	 */
	static ShaderParameter floatParameter(std::string name, float value);

	/**
	 * Creates a new float2 parameter
	 * @param name The name of the parameter
	 * @param x The first component
	 * @param y The second component
	 */
	static ShaderParameter float2Parameter(std::string name, float x, float y);

	/**
	 * Creates a new float3 parameter
	 * @param name The name of the parameter
	 * @param x The first component
	 * @param y The second component
	 * @param z The third component
	 */
	static ShaderParameter float3Parameter(std::string name, float x, float y, float z);

	/**
	 * Creates a new float4 parameter
	 * @param name The name of the parameter
	 * @param x The first component
	 * @param y The second component
	 * @param y The third component
	 * @param w The fourth component
	 */
	static ShaderParameter float4Parameter(std::string name, float x, float y, float z, float w);

	/**
	 * Creates a new float 4x4 matrix parameter
	 * @param name The name of the parameter
	 * @param matrix The value
	 */
	static ShaderParameter float4x4MatrixParameter(std::string name, const glm::mat4x4& matrix);
};

/**
 * Represents a shader program
 */
class ShaderProgram {
private:
	GLuint mVertexShader;
	GLuint mFragmentShader;
	GLuint mShaderProgram;

	std::unordered_map<std::string, GLint> mUniforms;
public:
	/**
	 * Creates a new shader program
	 * @param vertexShaderContent The content of the vertex shader
	 * @param fragmentShaderContent The content of the fragment shader
	 */
	ShaderProgram(const std::string& vertexShaderContent, const std::string& fragmentShaderContent);
	~ShaderProgram();

	//Disable copy
	ShaderProgram(const ShaderProgram&) = default;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	/**
	 * Returns the underlying OpenGL shader program
	 */
	GLuint id() const;

	/**
	 * Returns the given uniform
	 * @param name The name of the uniform
	 * @param throwOnNotExist Indicates if throws if not exist
	 */
	GLint getUniform(const std::string& name, bool throwOnNotExist = true);

	/**
	 * Sets the parameters
	 * @param parameters The parameters
	 */
	void setParameters(const std::vector<ShaderParameter>& parameters);
};
