#version 450

layout(location = 0) in vec4 frag_in_color;
layout(location = 0) out vec4 frag_out_color;

void main()
{
	frag_out_color = frag_in_color;
}
