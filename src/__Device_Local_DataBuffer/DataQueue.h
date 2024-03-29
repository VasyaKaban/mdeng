#pragma once

#include <cstdint>
#include <vector>
#include "../../hrs/mem_req.hpp"
#include "../TransferChannel/Data.h"
#include "../../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	struct DataRemoveOp
	{
		std::size_t offset;
		constexpr DataRemoveOp(std::uint32_t _offset) noexcept
			: offset(_offset) {}
	};

	struct DataAddOp
	{
		std::size_t *output_write;
		Data data;

		constexpr DataAddOp(std::size_t *_output_write = {}, const Data &_data = {}) noexcept
			: output_write(_output_write), data(_data) {}
	};

	struct DataUpdateOp
	{
		std::size_t offset;
		Data data;
		hrs::block<vk::DeviceSize> data_block;
		vk::DeviceSize in_data_buffer_offset;

		constexpr DataUpdateOp(std::size_t _offset = {},
							   const Data &_data = {},
							   const hrs::block<vk::DeviceSize> &_data_block = {},
							   vk::DeviceSize _in_data_buffer_offset = {}) noexcept
			: offset(_offset),
			  data(_data),
			  data_block(_data_block),
			  in_data_buffer_offset(_in_data_buffer_offset) {}
	};

	struct DataUpdatedAddOp
	{
		DataAddOp add_op;
		std::size_t offset;

		constexpr DataUpdatedAddOp(DataAddOp &_add_op, std::size_t _offset = {}) noexcept
			: add_op(_add_op), offset(_offset) {}
	};

	struct DataQueueReserves
	{
		std::size_t removes_reserve;
		std::size_t adds_reserve;
		std::size_t updates_reserve;
		std::size_t updated_adds_reserve;

		constexpr DataQueueReserves(std::size_t _removes_reserve = {},
									std::size_t _adds_reserve = {},
									std::size_t _updates_reserve = {},
									std::size_t _updated_adds_reserve = {}) noexcept
			: removes_reserve(_removes_reserve),
			  adds_reserve(_adds_reserve),
			  updates_reserve(_updates_reserve),
			  updated_adds_reserve(_updated_adds_reserve) {}

		DataQueueReserves(const DataQueueReserves &) = default;
		DataQueueReserves & operator=(const DataQueueReserves &) = default;
	};

	class DataQueue
	{
	public:
		DataQueue(const DataQueueReserves &reserves = {});
		~DataQueue() = default;
		DataQueue(const DataQueue &) = default;
		DataQueue(DataQueue &&) = default;
		DataQueue & operator=(const DataQueue &) = default;
		DataQueue & operator=(DataQueue &&) = default;

		void NewRemoveOp(const DataRemoveOp &op);
		void NewAddOp(DataAddOp op);
		void NewUpdateOp(const DataUpdateOp &op);

		void Clear(const DataQueueReserves &reserves);

		std::vector<DataRemoveOp> & GetRemoves() noexcept;
		const std::vector<DataRemoveOp> & GetRemoves() const noexcept;
		std::vector<DataAddOp> & GetAdds() noexcept;
		const std::vector<DataAddOp> & GetAdds() const noexcept;
		std::vector<DataUpdateOp> & GetUpdates() noexcept;
		const std::vector<DataUpdateOp> & GetUpdates() const noexcept;
		std::vector<DataUpdatedAddOp> & GetUpdatedAdds() noexcept;
		const std::vector<DataUpdatedAddOp> & GetUpdatedAdds() const noexcept;

	private:
		std::vector<DataRemoveOp> removes;
		std::vector<DataAddOp> adds;
		std::vector<DataUpdateOp> updates;
		std::vector<DataUpdatedAddOp> updated_adds;
	};
};
