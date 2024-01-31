#version 450

const vec3 vertices[3] =
{
	vec3(-0.66, -0.66, 0.5),
	vec3(0.0, 0.66, 0.5),
	vec3(0.66, -0.66, 0.5)
};

const vec3 colors[3] =
{
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
};

layout(push_constant) uniform InputPushConstants
{
	mat4 proj_view;
} input_push_constants;

layout(location = 0) out vec4 deffered_vert_out_color;

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex].x,
					   -vertices[gl_VertexIndex].y,
					   vertices[gl_VertexIndex].z,
					   1.0);
	//gl_Position = input_push_constants.proj_view * vec4(vertices[gl_VertexIndex], 1.0);
	deffered_vert_out_color = vec4(colors[gl_VertexIndex], 1.0);
}
