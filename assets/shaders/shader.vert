#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex shader inputs
//layout (location = 0) in vec2 inPosition;
//layout (location = 1) in vec3 inColor;

// Vertex shader outputs
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 vsColor;


vec2 positions[6] = vec2[](vec2(-1.0, -1.0),
													 vec2(1.0, 1.0),
													 vec2(-1.0, 1.0),

													 vec2(-1.0, -1.0),
													 vec2(1.0, -1.0),
													 vec2(1.0, 1.0));

vec3 colors[6] = vec3[](vec3(1.0, 0.0, 0.0),
												vec3(0.0, 1.0, 0.0),
												vec3(0.0, 0.0, 1.0),
												vec3(1.0, 0.0, 0.0),
												vec3(0.0, 1.0, 0.0),
												vec3(0.0, 0.0, 1.0));



void main()
{

	vsColor = colors[gl_VertexIndex];
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);


	//vsColor = inColor;
//	gl_Position = vec4(inPosition, 0.0, 1.0);
}
