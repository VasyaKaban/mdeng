World:
	RenderPass:
		Shader:
			MaterialHolder:
				MeshHolder:
					RenderGroup

local FRAMES = 3

-- set 0 -> Global(World)
-- set 1 -> RenderPass
-- set 2, 3 -> Shader

World =
{
	global_descriptor_set_0 = nil,
	mesh_instance_cluster_list = nil,
	renderpasses = {}
}

RenderPass =
{
	renderpass_descriptor_set_1 = nil,
	parent_world = nil,
	shaders = {}
}

Shader =
{
	parent_renderpass = nil,
	descriptor_storage_2 = nil,
	descriptor_storage_3 = nil,
	shader_descriptor_set_required_count = 0, 1, 2
	static_object_uniform_data_buffer,--fill ones, without changes -> DEVICE_LOCAL
	dynamic_object_uniform_data_buffer,--fill multiple times, with changes -> HOST_VISIBLE
	static_object_index_buffer[FRAMES],--fill ones -> HOST_VISIBLE + DEVICE_LOCAL
	dynamic_object_index_buffer[FRAMES],--fill multiple times -> HOST_VISIBLE
	material_holders = {}
}

MaterialHolder =
{
	parent_shader = nil,
	material = nil,
	mesh_holders = {},
	descriptor_set_2[FRAMES], --on demand
	descriptor_set_3[FRAMES] --on demand
}

MeshHolder =
{
	parent_material_holder = nil,
	mesh = nil,
	render_groups = {}
}

RenderGroup =
{
	parent_mesh_holder = nil,
	mesh_instance_cluster_count = 12345,
	mesh_instance_count = 12345,
	mesh_instance_cluster_begin = nil,
	shader_object_type = SATTIC/DYNAMIC,
	shader_index_buffer_begin = 12345,
	cull_index_offset = 0,--for culling and for render(checking whether we need to render this group or not)
	cull_viewed = 0--for culling
}

local MESH_INSTANCES_CLUSTER_SIZE = 32

MeshInstanceCluster =
{
	parent_render_group = nil,
	mesh_instances[max = MESH_INSTANCES_CLUSTER_SIZE],
	count = 123,
	mesh_instance_cluster_next = nil
}

MeshInstance =
{
	cull_simplex = nil,
	cull_region = nil,
	data = nil,
	object_type = STATIC/DYNAMIC,
	shader_object_uniform_data_buffer_index = 12345,
	get_transform_matrix = function(self)
		--do something with data
		return self.data
	end
}

function culled(instance, camera)
	---bla bla bla check camera and (instance.cull_simplex, instamce.cull_region)

	return true/false
end

function cull_clusters(world, camera)
	local cluster_begin = world.mesh_instance_cluster_list;
	for thread in thread
	do
		if cluster_begin == nil then
			break;
		else
			local target = cluster_begin
			cluster_begin = cluster_begin.mesh_instance_cluster_next

			for instance in target.mesh_instances
			do
				if ~culled(instance, camera) then
					local offset = target.parent_render_group.cull_index_offset
					target.parent_render_group.cull_index_offset = target.parent_render_group.cull_index_offset + 1
					target.parent_render_group.shader.select_index_buffer(target.parent_render_group.shader_object_type) + target.parent_render_group.shader_index_buffer_begin + offset = instance.shader_object_uniform_data_buffer_index
				end

				target.parent_render_group.cull_viewed = target.parent_render_group.cull_viewed + 1
				if target.parent_render_group.cull_viewed == mesh_instance_count then
					target.parent_render_group.cull_viewed = 0--for next cull cycle
					--target.parent_render_group.cull_index_offset = 0--for next cull cycle WE WILL SET ZERO ON RENDER!!! WE CAN USE IT TO DETERMINE WHETHER RENDER OR NOT!!!
				end
			end
		end
	end
end
