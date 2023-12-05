/**
 * @file
 *
 * Represents vulkan allocator
 */

#pragma once

#include "MemoryType.hpp"
#include "Buffer.hpp"
#include <vector>
#include "../hrs/expected.hpp"
#include "../hrs/block.hpp"
#include "../hrs/scoped_call.hpp"

namespace FireLand
{
	/**
	 * @brief The AllocatorError class
	 *
	 * Allocator error
	 */
	struct AllocatorError
	{
		vk::Result result;///<vulkan error
		bool desired_unsupported;///<flag that signals if desired memory property flags are unsupported
		constexpr AllocatorError(vk::Result _result = vk::Result::eSuccess,
								 bool _desired_unsupported = false) noexcept
			: result(_result), desired_unsupported(_desired_unsupported) {}

		/**
		 * @brief IsDesiredUnsupported
		 * @return true if desired_unsupported_flag is set
		 */
		constexpr auto IsDesiredUnsupported() const noexcept -> bool
		{
			return desired_unsupported;
		}
	};

	/**
	 * @brief The Allocator class
	 *
	 * Used for allocations
	 */
	class Allocator
	{
	public:
		Allocator() = default;
		/**
		 * @brief Allocator
		 * @param ph_device physical device for querying memory properties
		 * @param _device logical device to handle allocations
		 */
		Allocator(vk::PhysicalDevice ph_device, vk::Device &_device) noexcept;
		~Allocator() = default;
		Allocator(const Allocator &) = delete;
		constexpr Allocator(Allocator &&alloc) noexcept;
		Allocator & operator=(const Allocator &) = delete;
		constexpr Allocator & operator=(Allocator &&alloc) noexcept;
		/**
		 * @brief operator bool
		 * Checks whether allocator is created
		 */
		constexpr explicit operator bool() const noexcept;
		/**
		 * @brief IsCreated
		 * @return same as operator bool
		 */
		constexpr bool IsCreated() const noexcept;

		/**
		 * @brief GetDevice
		 * @return device associated with allocator
		 */
		constexpr vk::Device GetDevice() const noexcept;

		/**
		 * @brief GetMemoryTypes
		 * @return vector of memory types
		 */
		constexpr std::vector<MemoryType> GetMemoryTypes() const noexcept;
		/**
		 * @brief GetMemoryType
		 * @param index position within vector
		 * @return memory type from vector at this index
		 */
		constexpr MemoryType GetMemoryType(std::size_t index) const noexcept;

		/**
		 * @brief Allocate
		 * @param mem_type memory type from which memory will be allocated
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Function for plain memory allocation
		 */
		hrs::expected<Memory, AllocatorError>
		Allocate(const MemoryType &mem_type, vk::DeviceSize size, bool map_memory);

		/**
		 * @brief Allocate
		 * @param mem_type memory type from which memory will be allocated
		 * @param info buffer create info
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated buffer or allocation error
		 *
		 * Function for buffer allocation
		 */
		hrs::expected<Buffer, AllocatorError>
		Allocate(const MemoryType &mem_type, const vk::BufferCreateInfo &info, bool map_memory);

		/**
		 * @brief Allocate
		 * @param mem_type memory type from which memory will be allocated
		 * @param info image create info
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Allocates memory based on image create info.
		 * Memory size will be choosed as:
		 * @code
		 *	size = max(size, image_requirements.size)
		 * @endcode
		 */
		hrs::expected<Memory, AllocatorError>
		Allocate(const MemoryType &mem_type,
				 const vk::ImageCreateInfo &info,
				 vk::DeviceSize size,
				 bool map_memory);

		/**
		 * @brief AllocateAny
		 * @param desired desired memory type property flags
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Any operation
		 */
		hrs::expected<Memory, AllocatorError>
		AllocateAny(vk::MemoryPropertyFlags desired, vk::DeviceSize size, bool map_memory);

		/**
		 * @brief AllocateAny
		 * @param desired desired memory type property flags
		 * @param info buffer create info
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated buffer or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Any operation
		 */
		hrs::expected<Buffer, AllocatorError>
		AllocateAny(vk::MemoryPropertyFlags desired, const vk::BufferCreateInfo &info, bool map_memory);

		/**
		 * @brief AllocateAny
		 * @param desired desired memory type property flags
		 * @param info image create info
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Any operation
		 */
		hrs::expected<Memory, AllocatorError>
		AllocateAny(vk::MemoryPropertyFlags desired,
					const vk::ImageCreateInfo &info,
					vk::DeviceSize size,
					bool map_memory);

		/**
		 * @brief AllocateOnly
		 * @param desired desired memory type property flags
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Only operation
		 */
		hrs::expected<Memory, AllocatorError>
		AllocateOnly(vk::MemoryPropertyFlags desired, vk::DeviceSize size, bool map_memory);

		/**
		 * @brief AllocateOnly
		 * @param desired desired memory type property flags
		 * @param info buffer create info
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated buffer or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Only operation
		 */
		hrs::expected<Buffer, AllocatorError>
		AllocateOnly(vk::MemoryPropertyFlags desired, const vk::BufferCreateInfo &info, bool map_memory);

		/**
		 * @brief AllocateOnly
		 * @param desired desired memory type property flags
		 * @param info buffer create info
		 * @param size size of memory
		 * @param map_memory true if memory must be mapped to host memory
		 * @return allocated memory or allocation error
		 *
		 * Searching memory type which supports desired property flags and allocates memory in it
		 * by using Only operation
		 */
		hrs::expected<Memory, AllocatorError>
		AllocateOnly(vk::MemoryPropertyFlags desired,
					 const vk::ImageCreateInfo &info,
					 vk::DeviceSize size,
					 bool map_memory);

	private:
		/**
		 * @brief assert_memory_type_valid
		 * @param mem_type memory type to check
		 *
		 * Inner debug assertion for memory type validation
		 */
		constexpr void assert_memory_type_valid(const MemoryType &mem_type) const noexcept;

		/**
		 * @brief allocate_by_desired
		 * @tparam F must satisfy invocable concept with passed const MemoryType & as argument
		 * @param func functional object that will be called if compliant memory type is found
		 * @param desired desired memory property falgs
		 * @param is_any_operation true if Any operation and false if Only operation
		 * @return result value of the func call
		 *
		 * Base function for searching memory type functions(whose postfix looks like *Any or *Only)
		 */
		template<std::invocable<const MemoryType &> F>
			requires std::constructible_from<std::invoke_result_t<F, const MemoryType &>, AllocatorError>
		std::invoke_result_t<F, const MemoryType &>
		allocate_by_desired(F &&func, vk::MemoryPropertyFlags desired, bool is_any_operation);

		vk::Device device;///<device that takes ownership of all allocations
		std::vector<MemoryType> memory_types;///<array of mmeory types
	};

	/**
	 * Initializes memory_types array
	 * @warning Aborts if ph_device or _device aren't created!
	 */
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

	constexpr Allocator & Allocator::operator=(Allocator &&alloc) noexcept
	{
		device = std::move(alloc.device);
		memory_types = std::move(alloc.memory_types);
		return *this;
	}

	constexpr Allocator::operator bool() const noexcept
	{
		return device;
	}

	constexpr bool Allocator::IsCreated() const noexcept
	{
		return device;
	}

	constexpr vk::Device Allocator::GetDevice() const noexcept
	{
		return device;
	}

	constexpr std::vector<MemoryType> Allocator::GetMemoryTypes() const noexcept
	{
		return memory_types;
	}

	/**
	 * @warning It doesn't check if the index is out of bounds!
	 */
	constexpr MemoryType Allocator::GetMemoryType(std::size_t index) const noexcept
	{
		return memory_types[index];
	}

	/**
	 * @warning Aborts if the allocator isn't created!
	 */
	inline hrs::expected<Memory, AllocatorError>
	Allocator::Allocate(const MemoryType &mem_type, vk::DeviceSize size, bool map_memory)
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");
		assert_memory_type_valid(mem_type);

		if(map_memory && !mem_type.IsMappable())
			return {vk::Result::eErrorMemoryMapFailed};

		vk::MemoryAllocateInfo alloc_info(size, mem_type.index);
		auto allocation = device.allocateMemoryUnique(alloc_info);
		if(allocation.result != vk::Result::eSuccess)
			return {allocation.result};

		std::byte *map_ptr = nullptr;
		if(map_memory)
		{
			auto map_result = device.mapMemory(allocation.value.get(), 0, size);
			if(map_result.result != vk::Result::eSuccess)
				return {map_result.result};

			map_ptr = static_cast<std::byte *>(map_result.value);
		}

		return Memory(allocation.value.release(), size, map_ptr);
	}

	inline hrs::expected<Buffer, AllocatorError>
	Allocator::Allocate(const MemoryType &mem_type, const vk::BufferCreateInfo &info, bool map_memory)
	{
		auto memory_exp = Allocate(mem_type, info.size, map_memory);
		if(!memory_exp)
			return memory_exp.error();

		hrs::scoped_call memory_destroy([&memory_exp, this]()
		{
			memory_exp->Free(this->device);
		});

		auto buffer = device.createBufferUnique(info);
		if(buffer.result != vk::Result::eSuccess)
			return {buffer.result};

		auto bind_res = device.bindBufferMemory(buffer.value.get(), memory_exp->device_memory, 0);
		if(bind_res != vk::Result::eSuccess)
			return {bind_res};

		memory_destroy.Drop();
		return Buffer(memory_exp.value(), buffer.value.release());
	}

	inline hrs::expected<Memory, AllocatorError>
	Allocator::Allocate(const MemoryType &mem_type,
						const vk::ImageCreateInfo &info,
						vk::DeviceSize size,
						bool map_memory)
	{
		auto image = device.createImageUnique(info);
		if(image.result != vk::Result::eSuccess)
			return {image.result};

		auto reqs = device.getImageMemoryRequirements(image.value.get());
		auto memory_exp = Allocate(mem_type, hrs::round_up_size_to_alignment(size, reqs.size), map_memory);
		if(!memory_exp)
			return memory_exp.error();

		return Memory(memory_exp.value());
	}

	inline hrs::expected<Memory, AllocatorError>
	Allocator::AllocateAny(vk::MemoryPropertyFlags desired, vk::DeviceSize size, bool map_memory)
	{
		return allocate_by_desired([this, size, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, size, map_memory);
		}, desired, true);
	}

	inline hrs::expected<Buffer, AllocatorError>
	Allocator::AllocateAny(vk::MemoryPropertyFlags desired, const vk::BufferCreateInfo &info, bool map_memory)
	{
		return allocate_by_desired([this, &info, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, info, map_memory);
		}, desired, true);
	}

	inline hrs::expected<Memory, AllocatorError>
	Allocator::AllocateAny(vk::MemoryPropertyFlags desired,
						   const vk::ImageCreateInfo &info,
						   vk::DeviceSize size,
						   bool map_memory)
	{

		return allocate_by_desired([this, &info, size, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, info, size, map_memory);
		}, desired, true);
	}

	inline hrs::expected<Memory, AllocatorError>
	Allocator::AllocateOnly(vk::MemoryPropertyFlags desired, vk::DeviceSize size, bool map_memory)
	{
		return allocate_by_desired([this, size, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, size, map_memory);
		}, desired, false);
	}

	inline hrs::expected<Buffer, AllocatorError>
	Allocator::AllocateOnly(vk::MemoryPropertyFlags desired, const vk::BufferCreateInfo &info, bool map_memory)
	{
		return allocate_by_desired([this, &info, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, info, map_memory);
		}, desired, false);
	}

	inline hrs::expected<Memory, AllocatorError>
	Allocator::AllocateOnly(vk::MemoryPropertyFlags desired,
							const vk::ImageCreateInfo &info,
							vk::DeviceSize size,
							bool map_memory)
	{
		return allocate_by_desired([this, &info, size, map_memory](const MemoryType &mem_type)
		{
			return Allocate(mem_type, info, size, map_memory);
		}, desired, false);
	}

	/**
	 * Checks that memory type is part of this allocator
	 * @warning Works only on debug build!
	 */
	constexpr void Allocator::assert_memory_type_valid(const MemoryType &mem_type) const noexcept
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

	/**
	 * @warning Aborts if the allocator isn't created
	 */
	template<std::invocable<const MemoryType &> F>
		requires std::constructible_from<std::invoke_result_t<F, const MemoryType &>, AllocatorError>
	std::invoke_result_t<F, const MemoryType &>
	Allocator::allocate_by_desired(F &&func, vk::MemoryPropertyFlags desired, bool is_any_operation)
	{
		hrs::assert_true_debug(device, "Allocator isn't crated yet!");
		bool is_desired_satisfy = false;
		for(const auto &mem_type : memory_types)
		{
			bool is_satisfy = (is_any_operation ?
								   mem_type.IsSatisfyAny(desired) :
								   mem_type.IsSatisfyOnly(desired));
			if(is_satisfy)
			{
				is_desired_satisfy = true;
				auto object = std::forward<F>(func)(mem_type);
				if(object)
					return object;
			}
		}

		if(!is_desired_satisfy)
			return AllocatorError(vk::Result::eErrorUnknown, true);

		if(desired & vk::MemoryPropertyFlagBits::eDeviceLocal)
			return {vk::Result::eErrorOutOfDeviceMemory};
		else
			return {vk::Result::eErrorOutOfHostMemory};
	}
};
