#define LOADER_GEN_LIST() \
	FIRE_LAND_LOADER_BEGIN_DEVICE(DeviceLoader, Init) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDevice) \
 \
		FIRE_LAND_LOADER_FUNCTION(vkAllocateMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkFreeMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkUnmapMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkMapMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateBuffer) \
		FIRE_LAND_LOADER_FUNCTION(vkBindBufferMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkGetBufferMemoryRequirements) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyBuffer) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateImage) \
		FIRE_LAND_LOADER_FUNCTION(vkBindImageMemory) \
		FIRE_LAND_LOADER_FUNCTION(vkGetImageMemoryRequirements) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyImage) \
 \
		FIRE_LAND_LOADER_FUNCTION(vkCreateFence) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyFence) \
		FIRE_LAND_LOADER_FUNCTION(vkWaitForFences) \
		FIRE_LAND_LOADER_FUNCTION(vkGetFenceStatus) \
		FIRE_LAND_LOADER_FUNCTION(vkResetFences) \
		FIRE_LAND_LOADER_FUNCTION(vkBeginCommandBuffer) \
		FIRE_LAND_LOADER_FUNCTION(vkEndCommandBuffer) \
		FIRE_LAND_LOADER_FUNCTION(vkCmdPipelineBarrier) \
		FIRE_LAND_LOADER_FUNCTION(vkCmdCopyBuffer) \
		FIRE_LAND_LOADER_FUNCTION(vkCmdCopyBufferToImage) \
		FIRE_LAND_LOADER_FUNCTION(vkQueueSubmit) \
 \
		FIRE_LAND_LOADER_FUNCTION(vkGetDeviceQueue) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateCommandPool) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyCommandPool) \
		FIRE_LAND_LOADER_FUNCTION(vkAllocateCommandBuffers) \
		FIRE_LAND_LOADER_FUNCTION(vkFreeCommandBuffers) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateSemaphore) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroySemaphore) \
 \
		FIRE_LAND_LOADER_FUNCTION(vkCreateDescriptorSetLayout) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDescriptorSetLayout) \
		FIRE_LAND_LOADER_FUNCTION(vkCreateDescriptorPool) \
		FIRE_LAND_LOADER_FUNCTION(vkResetDescriptorPool) \
		FIRE_LAND_LOADER_FUNCTION(vkDestroyDescriptorPool) \
		FIRE_LAND_LOADER_FUNCTION(vkAllocateDescriptorSets) \
		FIRE_LAND_LOADER_FUNCTION(vkFreeDescriptorSets) \
	FIRE_LAND_LOADER_END()

