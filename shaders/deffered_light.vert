#version 450

const vec3 frame_quad_vertices[3] =
{
	vec3(-9.0, -1.0, 1.0),
	vec3(0.0, 2.0, 1.0),
	vec3(9.0, -1.0, 1.0)
};

void main()
{
	gl_Position = vec4(frame_quad_vertices[gl_VertexIndex], 1.0);
}
