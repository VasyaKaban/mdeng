#version 450

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput color_buffer;
layout(input_attachment_index = 0, set = 1, binding = 1) uniform subpassInput depth_stencil_buffer;
layout(input_attachment_index = 0, set = 1, binding = 2) uniform subpassInput normals_buffer;

layout(location = 0) out vec4 geom_frag_out;

layout(push_constant, std430) uniform InputFragPushConstants
{
	vec2 inverted_resolution;
} input_frag_push_constants;

void main()
{
	float x = gl_FragCoord.x * input_frag_push_constants.inverted_resolution.x;
	if(x <= 0.33)
		geom_frag_out = vec4(subpassLoad(color_buffer).rgb, 1.0);
	else if(x > 0.66)
		geom_frag_out = vec4(subpassLoad(normals_buffer).rgb, 1.0);
	else
		geom_frag_out = vec4(subpassLoad(depth_stencil_buffer).rrr, 1.0);
}
