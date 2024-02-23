#pragma once

#include "module_objects.h"

namespace ad_astris::renderer::impl
{
	class CpuBufferElementDesc
	{
		public:
			explicit CpuBufferElementDesc(uint64_t offset, uint64_t size)
				: _offset(offset), _size(size) { }
			uint64_t get_offset() const { return _offset; }
			uint64_t get_size() const { return _size; }

			bool operator==(const CpuBufferElementDesc& other) const
			{
				return _offset == other._offset && _size == other._size;
			}

		private:
			uint64_t _offset;
			uint64_t _size{ 0 };
	};
	
	template<typename T>
	class CpuBuffer
	{
		public:
			CpuBuffer(const std::string& bufferName, uint64_t size) : _bufferName(bufferName)
			{
				rhi::BufferInfo bufferInfo;
				bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC | rhi::ResourceUsage::STORAGE_BUFFER;
				bufferInfo.size = size;
				bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
				RENDERER_RESOURCE_MANAGER()->allocate_buffer(bufferName, bufferInfo);
			}

			size_t push_back(const T* data, uint64_t count)
			{
				rhi::Buffer* buffer = get_buffer();
				rhi::BufferInfo& bufferInfo = buffer->bufferInfo;
				const uint64_t newDataSize = sizeof(T) * count;
				if (_offset + newDataSize >= bufferInfo.size)
				{
					RENDERER_RESOURCE_MANAGER()->reallocate_buffer(buffer, bufferInfo.size * 2);
				}

				uint8_t* mappedData = static_cast<uint8_t*>(buffer->mappedData);
				memcpy(mappedData + _offset, data, newDataSize);
				const uint64_t oldOffset = _offset;
				_offset += newDataSize;
				uint32_t index = _elements.size();
				_elements.emplace_back(oldOffset, newDataSize);
				return index;
			}

			void remove(const CpuBufferElementDesc& element)
			{
				uint64_t offset = element.get_offset();
				uint64_t size = element.get_size();
				rhi::Buffer* buffer = get_buffer();
				rhi::BufferInfo& bufferInfo = buffer->bufferInfo;

				uint8_t* mappedData = static_cast<uint8_t*>(buffer->mappedData);
				memcpy(mappedData + offset, mappedData + offset + size, bufferInfo.size - offset - size);
				
				auto it = _elements.erase(std::find(_elements.begin(), _elements.end(), element));
				for (; it != _elements.end(); ++it)
				{
					*it = CpuBufferElementDesc(offset, it->get_size());
					offset += it->get_size();
				}
			}

			void clear()
			{
				_offset = 0;
				_elements.clear();
			}
		
			bool empty() const { return _elements.empty(); }
			rhi::Buffer* get_buffer() const { return RENDERER_RESOURCE_MANAGER()->get_buffer(_bufferName); }
			const std::string& get_name() const { return _bufferName; }
			uint64_t get_offset() const { return _offset; }
			uint64_t get_element_count() const { return _elements.size(); }

			T* get_data(size_t index)
			{
				const CpuBufferElementDesc& element = _elements[index];
				uint8_t* mappedData = static_cast<uint8_t*>(get_buffer()->mappedData);
				return reinterpret_cast<T*>(mappedData + element.get_offset());
			}
		
		private:
			std::string _bufferName;
			uint64_t _offset{ 0 };
			std::vector<CpuBufferElementDesc> _elements;
	};
}