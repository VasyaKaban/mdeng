#pragma once

#include "MemoryType.h"
#include "MemoryPool.h"

namespace FireLand
{
	struct BoundedBuffer : public hrs::non_copyable
	{
		BlockBindPool block_bind_pool;
		std::vector<MemoryType>::iterator memory_type;
		vk::Buffer buffer;

		BoundedBuffer(const BlockBindPool &_block_bind_pool = {},
					  std::vector<MemoryType>::iterator _memory_type = {},
					  vk::Buffer _buffer = {}) noexcept
			: block_bind_pool(_block_bind_pool),
			  memory_type(_memory_type),
			  buffer(_buffer) {}

		BoundedBuffer(BoundedBuffer &&buf) noexcept
			: block_bind_pool(buf.block_bind_pool),
			  memory_type(buf.memory_type),
			  buffer(std::exchange(buf.buffer, VK_NULL_HANDLE)) {}

		BoundedBuffer & operator=(BoundedBuffer &&buf) noexcept
		{
			block_bind_pool = buf.block_bind_pool;
			memory_type = buf.memory_type;
			buffer = std::exchange(buf.buffer, VK_NULL_HANDLE);

			return *this;
		}

		bool IsCreated() const noexcept
		{
			return buffer;
		}

		std::byte * GetBufferMapPtr() noexcept
		{
			return block_bind_pool.pool->GetMemory().GetMapPtr() + block_bind_pool.block.offset;
		}

		const std::byte * GetBufferMapPtr() const noexcept
		{
			return block_bind_pool.pool->GetMemory().GetMapPtr() + block_bind_pool.block.offset;
		}
	};

	struct BoundedImage : public hrs::non_copyable
	{
		BlockBindPool block_bind_pool;
		std::vector<MemoryType>::iterator memory_type;
		ResourceType res_type;
		vk::Image image;

		BoundedImage(const BlockBindPool &_block_bind_pool = {},
					 std::vector<MemoryType>::iterator _memory_type = {},
					 ResourceType _res_type = {},
					 vk::Image _image = {}) noexcept
			: block_bind_pool(_block_bind_pool),
			  memory_type(_memory_type),
			  res_type(_res_type),
			  image(_image) {}

		BoundedImage(BoundedImage &&img) noexcept
			: block_bind_pool(img.block_bind_pool),
			  memory_type(img.memory_type),
			  res_type(img.res_type),
			  image(std::exchange(img.image, VK_NULL_HANDLE)) {}

		BoundedImage & operator=(BoundedImage &&img) noexcept
		{
			block_bind_pool = img.block_bind_pool;
			memory_type = img.memory_type;
			res_type = img.res_type;
			image = std::exchange(img.image, VK_NULL_HANDLE);

			return *this;
		}

		bool IsCreated() const noexcept
		{
			return image;
		}

		std::byte * GetImageMapPtr() noexcept
		{
			return block_bind_pool.pool->GetMemory().GetMapPtr() + block_bind_pool.block.offset;
		}

		const std::byte * GetImageMapPtr() const noexcept
		{
			return block_bind_pool.pool->GetMemory().GetMapPtr() + block_bind_pool.block.offset;
		}
	};

	enum class MemoryTypeSatisfyOp
	{
		Any,
		Only
	};

	enum class AllocatorResult
	{
		Success,
		NoSatisfiedMemoryTypes
	};

	class Allocator : public hrs::non_copyable
	{
	public:
		Allocator(vk::Device _device, vk::PhysicalDevice ph_device);
		~Allocator();
		Allocator(Allocator &&allocator) noexcept;
		Allocator & operator=(Allocator &&allocator) noexcept;

		void Destroy() noexcept;

		hrs::expected<BoundedBuffer, hrs::error>
		Create(const vk::BufferCreateInfo &info,
			   MemoryTypeSatisfyOp op,
			   vk::MemoryPropertyFlags desired_props,
			   vk::DeviceSize alignment = 1,
			   hrs::flags<AllocationFlags> flags = {},
			   const std::function<NewPoolSizeCalculator> &calc = MemoryType::DefaultNewPoolSizeCalculator);

		hrs::expected<BoundedImage, hrs::error>
		Create(const vk::ImageCreateInfo &info,
			   MemoryTypeSatisfyOp op,
			   vk::MemoryPropertyFlags desired_props,
			   vk::DeviceSize alignment = 1,
			   hrs::flags<AllocationFlags> flags = {},
			   const std::function<NewPoolSizeCalculator> &calc = MemoryType::DefaultNewPoolSizeCalculator);

		void Release(const BoundedBuffer &bounded_buffer, MemoryPoolOnEmptyPolicy policy);
		void Release(const BoundedImage &bounded_image, MemoryPoolOnEmptyPolicy policy);

	private:
		bool is_memory_type_satisfy(const MemoryType &mem_type,
									MemoryTypeSatisfyOp op,
									vk::MemoryPropertyFlags desired_props,
									const vk::MemoryRequirements &req) const noexcept;

		bool is_non_terminate_error(const hrs::error &err) const noexcept;
	private:
		vk::Device device;
		std::vector<MemoryType> memory_types;
	};
};

template<>
struct hrs::enum_error_traits<FireLand::AllocatorResult>
{
	constexpr static void traits_hint() noexcept {};

	constexpr static std::string_view get_name(FireLand::AllocatorResult value) noexcept
	{
		switch(value)
		{
			case FireLand::AllocatorResult::Success:
				return "Success";
				break;
			case FireLand::AllocatorResult::NoSatisfiedMemoryTypes:
				return "NoSatisfiedMemoryTypes";
				break;
			default:
				return "";
				break;
		}
	}
};
