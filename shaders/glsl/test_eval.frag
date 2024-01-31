#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput color_buffer;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput depth_buffer;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = subpassLoad(color_buffer).rgba;
}
