#include <iostream>
#include <cstdlib>

#include "Transfer/Transfer.hpp"
#include "FreeBlockBuffer/FreeBlockBuffer.hpp"

using namespace std;

#define CHECK_SUCCESS(obj) if(obj.result != vk::Result::eSuccess) return obj.result;

auto TestVk() -> vk::Result
{
	vk::ApplicationInfo app_info("app",
								 1,
								 "engine",
								 1,
								 VK_API_VERSION_1_0);

	std::array<const char *, 1> layers = {"VK_LAYER_KHRONOS_validation"};
	std::array<const char *, 1> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
	vk::InstanceCreateInfo instance_info({},
										  &app_info,
										  layers,
										  extensions);

	auto inst = vk::createInstance(instance_info);
	CHECK_SUCCESS(inst);

	vk::Instance instance = inst.value;
	auto physical_devices = instance.enumeratePhysicalDevices();
	CHECK_SUCCESS(physical_devices);
	hrs::assert_true(!physical_devices.value.empty(), "No Physical devices!");
	auto ph_device = physical_devices.value[0];
	auto queue_props = ph_device.getQueueFamilyProperties();
	std::optional<vk::DeviceQueueCreateInfo> queue_info;
	float prior[1] = {1.0f};
	uint32_t ind = 0;
	for(const auto &q : queue_props)
	{
		if(q.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queue_info = vk::DeviceQueueCreateInfo({},
												   ind,
												   1, prior);
			break;
		}
		ind++;
	}

	hrs::assert_true(queue_info.has_value(), "No queue!");
	vk::DeviceCreateInfo device_info({},
									 1,
									 &*queue_info,
									 0,
									 nullptr,
									 0,
									 nullptr,
									 nullptr);

	auto dev = ph_device.createDevice(device_info);
	CHECK_SUCCESS(dev);
	vk::Device device = dev.value;

	constexpr static vk::DeviceSize BUFFER_SIZE = 1024 * 1024;
	using namespace FireLand;
	Allocator allocator(ph_device, device);
	auto tr = Transfer::Create(allocator, ind, 0);
	if(!tr.has_value())
		return tr.error();

	auto transfer = std::move(tr.value());
	vk::BufferCreateInfo host_buffer_info({},
									 BUFFER_SIZE,
									 vk::BufferUsageFlagBits::eTransferDst,
									 vk::SharingMode::eExclusive,
									 1,
									 &ind);

	/*vk::BufferCreateInfo device_buffer_info({},
											1024 * 1024,
											vk::BufferUsageFlagBits::eTransferDst |
											vk::BufferUsageFlagBits::eTransferSrc,
											vk::SharingMode::eExclusive,
											1,
											&ind);*/

	auto free_blk_buf = FreeBlockBuffer::Create(&allocator,
												1,
												BUFFER_SIZE,
												vk::BufferUsageFlagBits::eTransferDst |
												vk::BufferUsageFlagBits::eTransferSrc,
												ind);

	if(!free_blk_buf.has_value())
		return free_blk_buf.error();

	FreeBlockBuffer free_block_buffer = std::move(free_blk_buf.value());

	auto host_buf = allocator.AllocateBufferAny(host_buffer_info, vk::MemoryPropertyFlagBits::eHostVisible, true);
	if(!host_buf.has_value())
		return vk::Result::eErrorOutOfHostMemory;

	/*auto dev_buf = allocator.AllocateBufferAny(device_buffer_info, vk::MemoryPropertyFlagBits::eDeviceLocal, false);
	if(!host_buf.has_value())
		return vk::Result::eErrorOutOfHostMemory;*/

	auto host_buffer = std::move(host_buf.value());
	//auto device_buffer = std::move(dev_buf.value());
	uint32_t data[BUFFER_SIZE] = {};
	for(std::size_t i = 0; i < BUFFER_SIZE; i++)
		data[i] = i;

	DUAregions dua;
	AddRegion add_region;
	add_region.data = reinterpret_cast<std::byte *>(data);
	add_region.data_offset = 0;
	add_region.data_size = BUFFER_SIZE;
	dua.add_regions.push_back(add_region);
	auto blk_commands = free_block_buffer.ReceiveCommands(dua);
	if(!blk_commands.has_value())
		return blk_commands.error();

	Commands commands = std::move(blk_commands.value());
	for(auto &cmd : commands.commands_chain)
		transfer.TransferAnyObject(std::move(cmd), 1);

	//BufferRegion region;
	//region.data_offset = 0;
	//region.copy = vk::BufferCopy({}, 0, 1024);
	//std::array<BufferRegion, 1> regions = {region};
	//transfer.TransferToBuffer(regions, free_block_buffer.GetBuffer().buffer, reinterpret_cast<std::byte *>(data), 4);
	auto copy_to_host = [&](const vk::CommandBuffer &comm_buffer) -> void
	{
		vk::BufferMemoryBarrier buf_barrier(vk::AccessFlagBits::eMemoryWrite,
											vk::AccessFlagBits::eMemoryRead,
											VK_QUEUE_FAMILY_IGNORED,
											VK_QUEUE_FAMILY_IGNORED,
											free_block_buffer.GetBuffer().buffer,
											0,
											BUFFER_SIZE);

		comm_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
									vk::PipelineStageFlagBits::eTransfer,
									{},
									{},
									buf_barrier,
									{});

		vk::BufferCopy copy(0, 0, BUFFER_SIZE);
		comm_buffer.copyBuffer(free_block_buffer.GetBuffer().buffer,
							   host_buffer.buffer,
							   copy);
	};
	std::array<EmbeddedOperation, 1> ops = {copy_to_host};
	transfer.EmbedOperation(ops);
	auto res = transfer.Flush(std::ranges::empty_view<vk::SubmitInfo>{});
	if(res != vk::Result::eSuccess)
		return res;

	res = transfer.Wait();
	if(res != vk::Result::eSuccess)
		return res;

	for(std::size_t i = 0; i < BUFFER_SIZE / 4; i++)
		cout<<reinterpret_cast<uint32_t *>(host_buffer.map_ptr)[i]<<endl;

	DUAregions clear_dua;
	std::vector<hrs::block<vk::DeviceSize>> delete_regions =
	{
		hrs::block<vk::DeviceSize>{1024, 1024},
		hrs::block<vk::DeviceSize>{8192, 4096},
		hrs::block<vk::DeviceSize>{1024, 0},
		hrs::block<vk::DeviceSize>{BUFFER_SIZE - 8192 - 4096, 8192 + 4096},
	};
	clear_dua.delete_regions = std::move(delete_regions);
	auto clear_commands = free_block_buffer.ReceiveCommands(clear_dua);

	//cout<<endl;
	return vk::Result::eSuccess;
}

auto main(int argc, char **argv) -> int
{
	vk::Result result = TestVk();
	cout<<vk::to_string(result)<<endl;

	using namespace hrs;

	int a = 5, b = 7;
	assert_true(a > b,
				"{} must be greater than {}!",
				a, b);

	return EXIT_SUCCESS;
}
