#pragma once

#include "../hrs/free_block_allocator.hpp"
#include "../Allocator/Allocator.hpp"
#include "../hrs/expected.hpp"
#include "ImageBlock.hpp"

namespace FireLand
{
	struct ImagesFreeBlockBufferError
	{
		vk::Result result;
		bool is_not_enough_space;

		constexpr ImagesFreeBlockBufferError(vk::Result _result = vk::Result::eSuccess,
											 bool _is_not_enough_space = false) noexcept
			: result(_result), is_not_enough_space(_is_not_enough_space) {}

		constexpr bool IsNotEnoughSpace() const noexcept
		{
			return is_not_enough_space;
		}
	};

	class ImagesFreeBlockBuffer
	{
	public:
		ImagesFreeBlockBuffer(vk::Device _device, Memory &&_memory, vk::DeviceSize alignment);
		~ImagesFreeBlockBuffer();
		ImagesFreeBlockBuffer(const ImagesFreeBlockBuffer &) = delete;
		ImagesFreeBlockBuffer(ImagesFreeBlockBuffer &&buffer) noexcept;
		ImagesFreeBlockBuffer & operator=(const ImagesFreeBlockBuffer &) = delete;
		ImagesFreeBlockBuffer & operator=(ImagesFreeBlockBuffer &&buffer) noexcept;

		void Release(const ImageBlock &image_block) noexcept;
		void Release(const hrs::block<vk::DeviceSize> &blk) noexcept;

		hrs::expected<ImageBlock, ImagesFreeBlockBufferError> Acquire(vk::Image image);

		constexpr vk::DeviceSize GetSize() noexcept;

		void Clear();

		bool IsEmpty() const noexcept;

		constexpr vk::Device GetDevice() const noexcept;
		constexpr Memory & GetMemory() noexcept;
		constexpr const Memory & GetMemory() const noexcept;

	private:
		vk::Device device;
		Memory memory;
		hrs::free_block_allocator<vk::DeviceSize> blocks;
	};

	inline ImagesFreeBlockBuffer::ImagesFreeBlockBuffer(vk::Device _device,
												 Memory &&_memory,
												 vk::DeviceSize alignment)
		: blocks(alignment, _memory.size / alignment)
	{
		hrs::assert_true_debug(_device, "Allocator isn't created yet!");
		hrs::assert_true_debug(hrs::is_multiple_of(_memory.size, alignment),
							   "Size={} isn't power of alignemnt={}",
							   _memory.size, alignment);
		device = _device;
		memory = std::move(_memory);
	}

	inline ImagesFreeBlockBuffer::~ImagesFreeBlockBuffer()
	{
		memory.Free(device);
	}

	inline ImagesFreeBlockBuffer::ImagesFreeBlockBuffer(ImagesFreeBlockBuffer &&buffer) noexcept
		: device(buffer.device), memory(std::move(buffer.memory)), blocks(std::move(buffer.blocks)) {}

	inline ImagesFreeBlockBuffer & ImagesFreeBlockBuffer::operator=(ImagesFreeBlockBuffer &&buffer) noexcept
	{
		this->~ImagesFreeBlockBuffer();
		device = buffer.device;
		memory = std::move(buffer.memory);
		blocks = std::move(buffer.blocks);
		return *this;
	}

	inline void ImagesFreeBlockBuffer::Release(const ImageBlock &image_block) noexcept
	{
		if(image_block.image)
			blocks.release_block(image_block.block);
	}

	inline void ImagesFreeBlockBuffer::Release(const hrs::block<vk::DeviceSize> &blk) noexcept
	{
		blocks.release_block(blk);
	}

	inline hrs::expected<ImageBlock, ImagesFreeBlockBufferError> ImagesFreeBlockBuffer::Acquire(vk::Image image)
	{
		auto req = device.getImageMemoryRequirements(image);

		auto opt_it = blocks.acquire_blocks_no_append_hint(req.size, 1);
		if(!opt_it)
			return ImagesFreeBlockBufferError(vk::Result::eErrorUnknown, true);

		auto blk = blocks.acquire_blocks_hint(req.size, 1, opt_it.value());

		auto bind_res = device.bindImageMemory(image, memory.device_memory, blk.offset);
		if(bind_res != vk::Result::eSuccess)
		{
			blocks.release_block(blk);
			return ImagesFreeBlockBufferError(bind_res);
		}

		return ImageBlock(image, blk);
	}

	constexpr vk::DeviceSize ImagesFreeBlockBuffer::GetSize() noexcept
	{
		return memory.size;
	}

	inline void ImagesFreeBlockBuffer::Clear()
	{
		blocks.clear(blocks.get_block_size(), memory.size / blocks.get_block_size());
	}

	inline bool ImagesFreeBlockBuffer::IsEmpty() const noexcept
	{
		const auto &free_blocks = blocks.get_free_blocks();
		if(free_blocks.size() != 1)
			return false;

		if(free_blocks.front().offset == 0)
			return true;

		return false;
	}

	constexpr vk::Device ImagesFreeBlockBuffer::GetDevice() const noexcept
	{
		return device;
	}

	constexpr Memory & ImagesFreeBlockBuffer::GetMemory() noexcept
	{
		return memory;
	}

	constexpr const Memory & ImagesFreeBlockBuffer::GetMemory() const noexcept
	{
		return memory;
	}
};
