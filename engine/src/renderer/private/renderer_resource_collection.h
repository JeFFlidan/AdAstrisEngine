#pragma once

#include "core/platform.h"
#include "common.h"
#include "renderer_array.h"
#include <variant>

namespace ad_astris::renderer::impl
{
	template<typename T>
	class RendererResourceCollection
	{
		public:
			RendererResourceCollection(
				const std::string& cpuArrayName,
				const std::string& gpuBufferName,
				rhi::ResourceUsage bufferType,
				uint64_t size) : _collections(GpuCollection{RendererArray<T>(cpuArrayName, size), nullptr})
			{
				GpuCollection& gpuCollection = std::get<GpuCollection>(_collections);
				switch (bufferType)
				{
					case rhi::ResourceUsage::INDIRECT_BUFFER:
						gpuCollection.gpuBuffer = RENDERER_RESOURCE_MANAGER()->allocate_indirect_buffer(gpuBufferName, size);
						break;
					case rhi::ResourceUsage::STORAGE_BUFFER:
						gpuCollection.gpuBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(gpuBufferName, size);
						break;
					case rhi::ResourceUsage::UNIFORM_BUFFER:
						gpuCollection.gpuBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(gpuBufferName, size);
						break;
					default:
						LOG_ERROR("RendererResourceCollection::RendererResourceCollection: Buffer type is incorrect")
						break;
				}
			}

			RendererResourceCollection(const std::string& cpuToGpuBufferName, uint64_t size)
				: _collections(std::vector<RendererArray<T>>{ })
			{
				std::vector<RendererArray<T>>& arrays = std::get<std::vector<RendererArray<T>>>(_collections);
				for (size_t i = 0; i != RHI()->get_buffer_count(); ++i)
				{
					arrays.push_back(RendererArray<T>(cpuToGpuBufferName + std::to_string(i), size, rhi::MemoryUsage::CPU_TO_GPU));
				}
			}

			size_t push_back(const T* data, uint64_t count)
			{
				return std::visit([&](auto&& arg)->size_t
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.push_back(data, count);
					else
						return arg[FRAME_INDEX].push_back(data, count);
				}, _collections);
			}

			size_t push_back(const T& obj)
			{
				return std::visit([&](auto&& arg)->T*
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.push_back(obj);
					else
						return arg[FRAME_INDEX].push_back(obj);
				}, _collections);
			}

			T* push_back()
			{
				return std::visit([&](auto&& arg)->T*
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.push_back();
					else
						return arg[FRAME_INDEX].push_back();
				}, _collections);
			}

			T* get_data(uint64_t index)
			{
				return std::visit([&](auto&& arg)->T*
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.get_data(index);
					else
						return arg[FRAME_INDEX].get_data(index);
				}, _collections);
			}

			void remove(const RendererArrayElementDesc& element)
			{
				std::visit([&](auto&& arg)
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						arg.array.remove(element);
					else
						arg[FRAME_INDEX].remove(element);
				}, _collections);
			}

			void clear()
			{
				std::visit([&](auto&& arg)
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						arg.array.clear();
					else
						arg[FRAME_INDEX].clear();
				}, _collections);
			}

			bool empty() const
			{
				return std::visit([&](auto&& arg)->bool
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.empty();
					else
						return arg[FRAME_INDEX].empty();
				}, _collections);
			}
		
			rhi::Buffer* get_mapped_buffer() const
			{
				return std::visit([](auto&& arg)->rhi::Buffer*
                {
                	using ArgType = std::decay_t<decltype(arg)>;
                	if constexpr (std::is_same_v<ArgType, GpuCollection>)
                		return arg.array.get_buffer();
					else
						return arg[FRAME_INDEX].get_buffer();
                }, _collections);
			}
		
			uint64_t get_offset() const
			{
				return std::visit([](auto&& arg)->uint64_t
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.get_offset();
					else
						return arg[FRAME_INDEX].get_offset();
				}, _collections);
			}
		
			uint64_t get_element_count() const
			{
				return std::visit([](auto&& arg)->uint64_t
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.array.get_element_count();
					else
						return arg[FRAME_INDEX].get_element_count();
				}, _collections);
			}
		
			rhi::Buffer* get_gpu_buffer() const
			{
				return std::visit([](auto&& arg)->rhi::Buffer*
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return arg.gpuBuffer;
					else
						return nullptr;
				}, _collections);
			}
		
			bool is_gpu_collection() const
			{
				return std::visit([](auto&& arg)->bool
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, GpuCollection>)
						return true;
					else
						return false;
				}, _collections);
			}
		
		private:
			struct GpuCollection
			{
				RendererArray<T> array;
				rhi::Buffer* gpuBuffer{ nullptr };
			};

			std::variant<GpuCollection, std::vector<RendererArray<T>>> _collections;
	};
}