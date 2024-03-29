#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colors;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec4 geom_vert_out[2];//color and normals

/*
set 0 -> rendering worker globals
set 1 -> renderpass descriptors
set 2 -> shader descriptors
set 3 -> mesh group descriptors
 */

layout(set = 0, binding = 0) uniform RenderGlobals
{
	mat4x4 view_proj_matrix;
} render_globals;

layout(set = 2, binding = 0, std430) readonly buffer InstanceIndices
{
	uint indices[];
} instance_indices;

struct InstanceData
{
	mat4x4 model_matrix;
};

layout(set = 2, binding = 1, std430) readonly buffer InstancesData
{
	InstanceData data[];
} instances_data;

layout(push_constant, std430) uniform InputVertPushConstants
{
	uint mesh_group_indices_offset;
} input_vert_push_constants;

void main()
{
	uint instance_data_index = instance_indices.indices[input_vert_push_constants.mesh_group_indices_offset + gl_InstanceIndex];
	gl_Position = render_globals.view_proj_matrix * instances_data.data[instance_data_index].model_matrix * vec4(position, 1.0);
	geom_vert_out[0] = vec4(colors, 1.0);
	geom_vert_out[1] = vec4(normals, 1.0);
}
