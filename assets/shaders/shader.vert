#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 model;
} ubo;

// Vertex shader inputs
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

// Vertex shader outputs
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 vsColor;

void main()
{
	vsColor = inColor;
  gl_Position = ubo.model * vec4(inPosition, 0.0, 1.0);
}
