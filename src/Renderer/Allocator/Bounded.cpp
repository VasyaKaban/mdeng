#include "Bounded.h"

namespace FireLand
{
	BoundedBlock::BoundedBlock(const MemoryTypeAcquireResult &_acquire_data,
							   std::vector<MemoryType>::iterator _memory_type) noexcept
		: acquire_data(_acquire_data),
		  memory_type(_memory_type) {}

	std::byte * BoundedBlock::GetBufferMapPtr() noexcept
	{
		return acquire_data.pool->GetMemory().GetMapPtr() + acquire_data.block.offset;
	}

	const std::byte * BoundedBlock::GetBufferMapPtr() const noexcept
	{
		return acquire_data.pool->GetMemory().GetMapPtr() + acquire_data.block.offset;
	}

	BoundedBuffer::BoundedBuffer(const BoundedBlock &bb,
								 VkBuffer _buffer) noexcept
		: BoundedBlock(bb),
		  buffer(_buffer) {}

	BoundedBuffer::BoundedBuffer(const MemoryTypeAcquireResult &_acquire_data,
								 std::vector<MemoryType>::iterator _memory_type,
								 VkBuffer _buffer) noexcept
		: BoundedBlock(_acquire_data, _memory_type),
		  buffer(_buffer) {}

	BoundedBuffer::BoundedBuffer(BoundedBuffer &&buf) noexcept
		: BoundedBlock(std::move(buf)),
		  buffer(std::exchange(buf.buffer, VK_NULL_HANDLE)) {}

	BoundedBuffer & BoundedBuffer::operator=(BoundedBuffer &&buf) noexcept
	{
		BoundedBlock::operator=(std::move(buf));
		buffer = std::exchange(buf.buffer, VK_NULL_HANDLE);

		return *this;
	}

	bool BoundedBuffer::IsCreated() const noexcept
	{
		return buffer != VK_NULL_HANDLE;
	}

	BoundedImage::BoundedImage(const BoundedBlock &bb,
							   ResourceType _image_type,
							   VkImage _image) noexcept
		: BoundedBlock(bb),
		  image_type(_image_type),
		  image(_image) {}

	BoundedImage::BoundedImage(const MemoryTypeAcquireResult &_acquire_data,
							   std::vector<MemoryType>::iterator _memory_type,
							   ResourceType _image_type,
							   VkImage _image) noexcept
		: BoundedBlock(_acquire_data, _memory_type),
		  image_type(_image_type),
		  image(_image) {}

	BoundedImage::BoundedImage(BoundedImage &&img) noexcept
		: BoundedBlock(std::move(img)),
		  image_type(img.image_type),
		  image(img.image) {}

	BoundedImage & BoundedImage::operator=(BoundedImage &&img) noexcept
	{
		BoundedBlock::operator=(std::move(img));
		image_type = img.image_type;
		image = img.image;

		return *this;
	}

	bool BoundedImage::IsCreated() const noexcept
	{
		return image != VK_NULL_HANDLE;
	}
};

