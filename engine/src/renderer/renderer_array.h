#pragma once

#include "module_objects.h"

namespace ad_astris::renderer::impl
{
	class RendererArrayElementDesc
	{
		public:
			explicit RendererArrayElementDesc(uint64_t offset, uint64_t size)
				: _offset(offset), _size(size) { }
			uint64_t get_offset() const { return _offset; }
			uint64_t get_size() const { return _size; }

			bool operator==(const RendererArrayElementDesc& other) const
			{
				return _offset == other._offset && _size == other._size;
			}

		private:
			uint64_t _offset;
			uint64_t _size{ 0 };
	};
	
	template<typename T>
	class RendererArray
	{
		public:
			RendererArray(const std::string& bufferName, uint64_t size, rhi::MemoryUsage memoryUsage = rhi::MemoryUsage::CPU)
			{
				rhi::BufferInfo bufferInfo;
				bufferInfo.size = size;
				bufferInfo.memoryUsage = memoryUsage;
				switch (bufferInfo.memoryUsage)
				{
					case rhi::MemoryUsage::CPU:
					case rhi::MemoryUsage::AUTO:
						bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC | rhi::ResourceUsage::STORAGE_BUFFER;
						break;
					case rhi::MemoryUsage::CPU_TO_GPU:
						if (!RHI()->has_capability(rhi::GpuCapability::CACHE_COHERENT_UMA))
						{
							LOG_ERROR("RendererArray::RendererArray(): Can't create buffer with usage CPU_TO_GPU because GPU does not support cache coherent uma capability")
						}
						bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC | rhi::ResourceUsage::STORAGE_BUFFER;
						break;
					case rhi::MemoryUsage::GPU:
					case rhi::MemoryUsage::GPU_TO_CPU:
						LOG_ERROR("RendererArray::RendererArray(): Can't use buffers with memory usage GPU or GPU_TO_CPU")
						break;
				}
				_buffer = RENDERER_RESOURCE_MANAGER()->allocate_buffer(bufferName, bufferInfo);
			}

			size_t push_back(const T* data, uint64_t count)
			{
				rhi::BufferInfo& bufferInfo = _buffer->bufferInfo;
				const uint64_t newDataSize = sizeof(T) * count;
				if (_offset + newDataSize >= bufferInfo.size)
				{
					RENDERER_RESOURCE_MANAGER()->reallocate_buffer(_buffer, bufferInfo.size * 2);
				}

				uint8_t* mappedData = static_cast<uint8_t*>(_buffer->mappedData);
				memcpy(mappedData + _offset, data, newDataSize);
				const uint64_t oldOffset = _offset;
				_offset += newDataSize;
				uint32_t index = _elements.size();
				_elements.emplace_back(oldOffset, newDataSize);
				return index;
			}

			T* push_back()
			{
				rhi::BufferInfo& bufferInfo = _buffer->bufferInfo;
				const uint64_t newDataSize = sizeof(T);
				if (_offset + newDataSize >= bufferInfo.size)
				{
					RENDERER_RESOURCE_MANAGER()->reallocate_buffer(_buffer, bufferInfo.size * 2);
				}
				
				const uint64_t oldOffset = _offset;
				_offset += newDataSize;
				uint32_t index = _elements.size();
				_elements.emplace_back(oldOffset, newDataSize);
				return get_data(index);
			}

			void remove(const RendererArrayElementDesc& element)
			{
				uint64_t offset = element.get_offset();
				uint64_t size = element.get_size();
				rhi::BufferInfo& bufferInfo = _buffer->bufferInfo;

				uint8_t* mappedData = static_cast<uint8_t*>(_buffer->mappedData);
				memcpy(mappedData + offset, mappedData + offset + size, bufferInfo.size - offset - size);
				
				auto it = _elements.erase(std::find(_elements.begin(), _elements.end(), element));
				for (; it != _elements.end(); ++it)
				{
					*it = RendererArrayElementDesc(offset, it->get_size());
					offset += it->get_size();
				}
			}

			void clear()
			{
				_offset = 0;
				_elements.clear();
			}
		
			bool empty() const { return _elements.empty(); }
			rhi::Buffer* get_buffer() const { return _buffer; }
			uint64_t get_offset() const { return _offset; }
			uint64_t get_element_count() const { return _elements.size(); }

			T* get_data(size_t index)
			{
				const RendererArrayElementDesc& element = _elements[index];
				uint8_t* mappedData = static_cast<uint8_t*>(_buffer->mappedData);
				return reinterpret_cast<T*>(mappedData + element.get_offset());
			}
		
		private:
			rhi::Buffer* _buffer{ nullptr };
			uint64_t _offset{ 0 };
			std::vector<RendererArrayElementDesc> _elements;
	};
}