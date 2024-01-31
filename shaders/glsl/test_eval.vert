#version 450

const vec3 frame_quad_vertices[6] =
{
	vec3(-1.0, 1.0, 1.0),
	vec3(-1.0, -1.0, 1.0),
	vec3(1.0, -1.0, 1.0),
	vec3(1.0, -1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0)
};

void main()
{
	gl_Position = vec4(frame_quad_vertices[gl_VertexIndex], 1.0);
}
