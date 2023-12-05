#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput color;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput depth;

layout(location = 0) out vec4 color_out;

void main()
{
	color_out = vec4(subpassLoad(color).rgb, 1.0);
}
