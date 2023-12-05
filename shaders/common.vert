#version 450

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 color;

layout(push_constant) uniform PushConstantInput
{
	mat4 proj_view;//projection * view matrix
	uint draw_indices_offset;//per instance-group start index inside buffer of all indices
} object_push_constant;

struct InstanceData
{
	mat4 model;//each instance model matrix
};

layout(std430, set = 0, binding = 0) readonly buffer instances_data
{
	InstanceData per_instance_data[];//buffer of uniform-data for all meshes
};

layout(std430, set = 0, binding = 0) readonly buffer draw_indices
{
	uint indices[];//buffer for all instance-groups indices
};

layout(location = 0) out vec4 out_color;

void main()
{
	uint instance_index = object_push_constant.draw_indices_offset + draw_indices.indices[gl_InstanceIndex];//index for per_instance_data
	gl_Position = object_push_constant.proj_view * instances_data.per_instance_data[instance_index].model * vec4(vertex, 1.0);
	out_color = vec4(color, 1.0);
}
