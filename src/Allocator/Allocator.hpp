#pragma once

#include "MemoryType.hpp"
#include "Buffer.hpp"
#include "ImageBuffer.hpp"
#include <vector>
#include "../hrs/expected.hpp"
#include "../hrs/block.hpp"

namespace FireLand
{
	struct AllocatorError
	{
		vk::Result result;
		bool desired_unsupported;
		constexpr AllocatorError(vk::Result _result = vk::Result::eSuccess,
								 bool _desired_unsupported = false) noexcept
			: result(_result), desired_unsupported(_desired_unsupported) {}

		constexpr auto IsDesiredUnsupported() const noexcept -> bool
		{
			return desired_unsupported;
		}
	};

	class Allocator
	{
	public:
		Allocator() = default;
		Allocator(vk::PhysicalDevice ph_device, vk::Device &_device) noexcept;
		~Allocator() = default;
		Allocator(const Allocator &) = delete;
		constexpr Allocator(Allocator &&alloc) noexcept;
		auto operator=(const Allocator &) -> Allocator & = delete;
		constexpr auto operator=(Allocator &&alloc) noexcept -> Allocator &;
		constexpr explicit operator bool() const noexcept;
		constexpr auto IsCreated() const noexcept -> bool;

		constexpr auto GetDevice() const noexcept -> vk::Device;

		constexpr auto GetMemoryTypes() const noexcept -> std::vector<MemoryType>;
		constexpr auto GetMemoryType(std::size_t index) const noexcept -> MemoryType;

		auto AllocateBuffer(const vk::BufferCreateInfo &info,
							const MemoryType &mem_type,
							bool map_memory) -> hrs::expected<Buffer, AllocatorError>;

		auto AllocateBufferAny(const vk::BufferCreateInfo &info,
							   vk::MemoryPropertyFlags desired,
							   bool map_memory) -> hrs::expected<Buffer, AllocatorError>;

		auto AllocateBufferOnly(const vk::BufferCreateInfo &info,
								vk::MemoryPropertyFlags desired,
								bool map_memory) -> hrs::expected<Buffer, AllocatorError>;

		auto AllocateImageBuffer(const vk::ImageCreateInfo &info,
								 vk::DeviceSize size,
								 const MemoryType &mem_type,
								 bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>;

		auto AllocateImageBufferAny(const vk::ImageCreateInfo &info,
									vk::DeviceSize size,
									vk::MemoryPropertyFlags desired,
									bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>;

		auto AllocateImageBufferOnly(const vk::ImageCreateInfo &info,
									 vk::DeviceSize size,
									 vk::MemoryPropertyFlags desired,
									 bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>;

	private:
		constexpr auto assert_memory_type_valid(const MemoryType &mem_type) const noexcept -> void;

		auto allocate_buffer_by_desired(const vk::BufferCreateInfo &info,
												  vk::MemoryPropertyFlags desired,
												  bool map_memory,
												  bool desired_any) -> hrs::expected<Buffer, AllocatorError>;

		auto allocate_image_buffer_by_desired(const vk::ImageCreateInfo &info,
											  vk::DeviceSize size,
											  vk::MemoryPropertyFlags desired,
											  bool map_memory,
											  bool desired_any) -> hrs::expected<ImageBuffer, AllocatorError>;
		//auto allocate_buffer()

		vk::Device device;
		std::vector<MemoryType> memory_types;
	};


	inline Allocator::Allocator(vk::PhysicalDevice ph_device, vk::Device &_device) noexcept
	{
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");
		hrs::assert_true_debug(_device, "Device isn't created yet!");
		device = _device;
		auto memory_props = ph_device.getMemoryProperties();
		memory_types.reserve(memory_props.memoryTypeCount);
		for(uint32_t i = 0; i < memory_props.memoryTypes.size(); i++)
			memory_types.push_back({memory_props.memoryTypes[i].propertyFlags, i});
	}

	constexpr Allocator::Allocator(Allocator &&alloc) noexcept
	{
		device = std::move(alloc.device);
		memory_types = std::move(alloc.memory_types);
	}

	constexpr auto Allocator::operator=(Allocator &&alloc) noexcept -> Allocator &
	{
		device = std::move(alloc.device);
		memory_types = std::move(alloc.memory_types);
		return *this;
	}

	constexpr Allocator::operator bool() const noexcept
	{
		return device;
	}

	constexpr auto Allocator::IsCreated() const noexcept -> bool
	{
		return device;
	}

	constexpr auto Allocator::GetDevice() const noexcept -> vk::Device
	{
		return device;
	}

	constexpr auto Allocator::GetMemoryTypes() const noexcept -> std::vector<MemoryType>
	{
		return memory_types;
	}

	constexpr auto Allocator::GetMemoryType(std::size_t index) const noexcept -> MemoryType
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");
		return memory_types[index];
	}

	inline auto Allocator::AllocateBuffer(const vk::BufferCreateInfo &info,
										  const MemoryType &mem_type,
										  bool map_memory) -> hrs::expected<Buffer, AllocatorError>
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");
		assert_memory_type_valid(mem_type);

		if(map_memory && !mem_type.IsMappable())
			return {vk::Result::eErrorMemoryMapFailed};

		auto buffer = device.createBufferUnique(info);
		if(buffer.result != vk::Result::eSuccess)
			return {buffer.result};

		auto reqs = device.getBufferMemoryRequirements(buffer.value.get());
		if(!mem_type.IsSatisfy(reqs))
			return AllocatorError{vk::Result::eErrorUnknown, true};

		vk::MemoryAllocateInfo alloc_info(reqs.size, mem_type.index);
		auto allocation = device.allocateMemoryUnique(alloc_info);
		if(allocation.result != vk::Result::eSuccess)
			return {allocation.result};

		auto bind_res = device.bindBufferMemory(buffer.value.get(), allocation.value.get(), 0);
		if(bind_res != vk::Result::eSuccess)
			return {bind_res};

		std::byte *map_ptr = nullptr;
		if(map_memory)
		{
			auto map_result = device.mapMemory(allocation.value.get(), 0, reqs.size);
			if(map_result.result != vk::Result::eSuccess)
				return {map_result.result};

			map_ptr = static_cast<std::byte *>(map_result.value);
		}

		return Buffer(allocation.value.release(), buffer.value.release(), reqs.size, map_ptr);
	}

	inline auto Allocator::allocate_buffer_by_desired(const vk::BufferCreateInfo &info,
													  vk::MemoryPropertyFlags desired,
													  bool map_memory,
													  bool desired_any) -> hrs::expected<Buffer, AllocatorError>
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");

		auto buffer = device.createBufferUnique(info);
		if(buffer.result != vk::Result::eSuccess)
			return {buffer.result};

		auto reqs = device.getBufferMemoryRequirements(buffer.value.get());
		bool found = false;
		for(auto &mem_type : memory_types)
		{
			bool is_satisfy_desired = (desired_any ? mem_type.IsSatisfyAny(desired) :
										   mem_type.IsSatisfyOnly(desired));
			if(mem_type.IsSatisfy(reqs) && is_satisfy_desired)
			{
				if(map_memory && !mem_type.IsMappable())
					continue;

				found = true;
				vk::MemoryAllocateInfo alloc_info(reqs.size, mem_type.index);
				auto allocation = device.allocateMemoryUnique(alloc_info);
				if(allocation.result != vk::Result::eSuccess)
					continue;

				auto bind_res = device.bindBufferMemory(buffer.value.get(), allocation.value.get(), 0);
				if(bind_res != vk::Result::eSuccess)
					continue;

				std::byte *map_ptr = nullptr;
				if(map_memory)
				{
					auto map_result = device.mapMemory(allocation.value.get(), 0, reqs.size);
					if(map_result.result != vk::Result::eSuccess)
						continue;

					map_ptr = static_cast<std::byte *>(map_result.value);
				}

				return Buffer(allocation.value.release(), buffer.value.release(), reqs.size, map_ptr);
			}
		}

		if(found)
		{
			if(desired & vk::MemoryPropertyFlagBits::eDeviceLocal)
				return {vk::Result::eErrorOutOfDeviceMemory};
			else
				return {vk::Result::eErrorOutOfHostMemory};
		}
		else
			return AllocatorError{vk::Result::eErrorUnknown, true};
	}

	inline auto Allocator::AllocateBufferAny(const vk::BufferCreateInfo &info, vk::MemoryPropertyFlags desired, bool map_memory) -> hrs::expected<Buffer, AllocatorError>
	{
		return allocate_buffer_by_desired(info, desired, map_memory, true);
	}

	inline auto Allocator::AllocateBufferOnly(const vk::BufferCreateInfo &info, vk::MemoryPropertyFlags desired, bool map_memory) -> hrs::expected<Buffer, AllocatorError>
	{
		return allocate_buffer_by_desired(info, desired, map_memory, false);
	}

	constexpr auto Allocator::assert_memory_type_valid(const MemoryType &mem_type) const noexcept -> void
	{
	#ifndef NDEBUG
		bool found = false;
		for(const auto &memory_type : memory_types)
			if(memory_type == mem_type)
			{
				found = true;
				break;
			}

		hrs::assert_true_debug(found, "Memory type is not a part of this allocator!");
	#endif
	}

	inline auto Allocator::AllocateImageBuffer(const vk::ImageCreateInfo &info,
											   vk::DeviceSize size,
											   const MemoryType &mem_type,
											   bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");
		assert_memory_type_valid(mem_type);

		if(map_memory && !mem_type.IsMappable())
			return {vk::Result::eErrorMemoryMapFailed};

		auto image = device.createImageUnique(info);
		if(image.result != vk::Result::eSuccess)
			return {image.result};

		auto reqs = device.getImageMemoryRequirements(image.value.get());
		if(!mem_type.IsSatisfy(reqs))
			return AllocatorError{vk::Result::eErrorUnknown, true};

		size = hrs::round_up_size_to_alignment(size, reqs.size);
		vk::MemoryAllocateInfo alloc_info(size, mem_type.index);
		auto allocation = device.allocateMemoryUnique(alloc_info);
		if(allocation.result != vk::Result::eSuccess)
			return {allocation.result};

		std::byte *map_ptr = nullptr;
		if(map_memory)
		{
			auto map_result = device.mapMemory(allocation.value.get(), 0, reqs.size);
			if(map_result.result != vk::Result::eSuccess)
				return {map_result.result};

			map_ptr = static_cast<std::byte *>(map_result.value);
		}

		return ImageBuffer(allocation.value.release(), size, map_ptr);
	}

	inline auto Allocator::allocate_image_buffer_by_desired(const vk::ImageCreateInfo &info,
															vk::DeviceSize size,
															vk::MemoryPropertyFlags desired,
															bool map_memory,
															bool desired_any)
		-> hrs::expected<ImageBuffer, AllocatorError>
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");

		auto image = device.createImageUnique(info);
		if(image.result != vk::Result::eSuccess)
			return {image.result};

		auto reqs = device.getImageMemoryRequirements(image.value.get());
		bool found = false;
		for(auto &mem_type : memory_types)
		{
			bool is_satisfy_desired = (desired_any ? mem_type.IsSatisfyAny(desired) :
										   mem_type.IsSatisfyOnly(desired));
			if(mem_type.IsSatisfy(reqs) && is_satisfy_desired)
			{
				if(map_memory && !mem_type.IsMappable())
					continue;

				found = true;
				vk::MemoryAllocateInfo alloc_info(hrs::round_up_size_to_alignment(size, reqs.alignment),
												  mem_type.index);
				auto allocation = device.allocateMemoryUnique(alloc_info);
				if(allocation.result != vk::Result::eSuccess)
					continue;

				std::byte *map_ptr = nullptr;
				if(map_memory)
				{
					auto map_result = device.mapMemory(allocation.value.get(), 0, reqs.size);
					if(map_result.result != vk::Result::eSuccess)
						continue;

					map_ptr = static_cast<std::byte *>(map_result.value);
				}

				return ImageBuffer(allocation.value.release(), reqs.size, map_ptr);
			}
		}

		if(found)
		{
			if(desired & vk::MemoryPropertyFlagBits::eDeviceLocal)
				return {vk::Result::eErrorOutOfDeviceMemory};
			else
				return {vk::Result::eErrorOutOfHostMemory};
		}
		else
			return AllocatorError{vk::Result::eErrorUnknown, true};
	}

	inline auto Allocator::AllocateImageBufferAny(const vk::ImageCreateInfo &info,
												  vk::DeviceSize size,
												  vk::MemoryPropertyFlags desired,
												  bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>
	{
		return allocate_image_buffer_by_desired(info, size, desired, map_memory, true);
	}

	inline auto Allocator::AllocateImageBufferOnly(const vk::ImageCreateInfo &info,
												   vk::DeviceSize size,
												   vk::MemoryPropertyFlags desired,
												   bool map_memory) -> hrs::expected<ImageBuffer, AllocatorError>
	{
		return allocate_image_buffer_by_desired(info, size, desired, map_memory, false);
	}
};
