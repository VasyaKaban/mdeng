#version 450

layout(location = 0) in vec4 deffered_vert_out_color;
layout(location = 0) out vec4 deffered_frag_out_color;

void main()
{
	deffered_frag_out_color = deffered_vert_out_color;
}
