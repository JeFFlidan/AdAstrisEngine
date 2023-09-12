#pragma once

#include "scene_submanager_base.h"
#include "common.h"
#include "renderer/common.h"
#include "engine_core/basic_components.h"

namespace ad_astris::renderer::impl
{
	class LightSubmanager : public SceneSubmanagerBase
	{
		public:
			LightSubmanager(SceneSubmanagerInitializationContext& initContext);

			virtual void update(rhi::CommandBuffer& cmdBuffer) override;
			virtual void cleanup_staging_buffers() override;
			virtual bool need_allocation() override;

			rhi::Buffer* get_point_light_storage_buffer()
			{
				return &_storageBuffers.pointLights.buffer;
			}

			rhi::Buffer* get_directional_light_storage_buffer()
			{
				return &_storageBuffers.directionalLights.buffer;
			}

			rhi::Buffer* get_spot_light_storage_buffer()
			{
				return &_storageBuffers.spotLights.buffer;
			}

		private:
			struct
			{
				GPUBuffer pointLights;
				GPUBuffer directionalLights;
				GPUBuffer spotLights;
			} _storageBuffers;
			bool _areGPUBuffersAllocated{ false };

			struct
			{
				rhi::Buffer pointLights;
				rhi::Buffer directionalLights;
				rhi::Buffer spotLights;
			} _stagingBuffers;
			std::vector<rhi::Buffer*> _stagingBuffersToDelete;

			struct
			{
				ObjectCPUCollection<PointLight> pointLights;
				ObjectCPUCollection<DirectionalLight> directionalLights;
				ObjectCPUCollection<SpotLight> spotLights;
			} _cpuCollections;
		
			struct PointLightTemp
			{
				uint32_t pointLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct DirectionalLightTemp
			{
				uint32_t directionalLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct SpotLightTemp
			{
				uint32_t spotLightCollectionIndex;
				ecore::ExtentComponent extent;
			};

			struct
			{
				std::vector<PointLightTemp> pointLights;
				std::vector<DirectionalLightTemp> directionalLights;
				std::vector<SpotLightTemp> spotLights;
			} _recentlyCreated;
		
			virtual void subscribe_to_events() override;

			void setup_lights();
			void allocate_staging_buffers();
			void allocate_gpu_buffers(rhi::CommandBuffer& cmdBuffer);

			template<typename LightType, typename LightTypeTemp>
			void allocate_staging_buffer(
				rhi::Buffer& stagingBuffer,
				GPUBuffer& gpuBuffer,
				std::vector<LightTypeTemp>& recentlyCreatedLights,
				ObjectCPUCollection<LightType>& cpuCollection)
			{
				if (recentlyCreatedLights.empty())
					return;

				rhi::BufferInfo bufferInfo;
				bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
				bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;

				uint64_t gpuBufferSize = gpuBuffer.buffer.size;
				uint64_t fullCPUCollectionByteSize = cpuCollection.size() * sizeof(LightType);

				if (fullCPUCollectionByteSize >= gpuBufferSize)
				{
					bufferInfo.size = fullCPUCollectionByteSize;
					_rhi->create_buffer(&stagingBuffer, &bufferInfo, cpuCollection.begin());
				}
				else
				{
					bufferInfo.size = recentlyCreatedLights.size() * sizeof(LightType);
					uint64_t cpuCollectionOffset = (cpuCollection.size() - recentlyCreatedLights.size()) * sizeof(LightType);
					_rhi->create_buffer(&stagingBuffer, &bufferInfo, cpuCollection.begin() + cpuCollectionOffset);
				}

				LOG_INFO("LightSubmanager::allocate_staging_buffer(): Allocated new {} staging buffer. Size: {}", get_type_name<LightType>(), stagingBuffer.size)
			}

			template<typename LightType>
			void allocate_gpu_buffer(
				rhi::CommandBuffer& cmdBuffer,
				rhi::Buffer& stagingBuffer,
				GPUBuffer& gpuBuffer,
				ObjectCPUCollection<LightType>& cpuCollection,
				uint64_t defaultObjectCount)
			{
				rhi::BufferInfo bufferInfo;
				bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::STORAGE_BUFFER;
				bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
	
				if (!_recentlyCreated.pointLights.empty())
				{
					if (!_areGPUBuffersAllocated)
					{
						uint64_t defaultSize = POINT_LIGHT_DEFAULT_COUNT * sizeof(LightType);
						uint64_t customSize = stagingBuffer.size * 2;
						bufferInfo.size = defaultSize > customSize ? defaultSize : customSize;
						_rhi->create_buffer(&gpuBuffer.buffer, &bufferInfo);
						_rhi->copy_buffer(&cmdBuffer, &stagingBuffer, &gpuBuffer.buffer);
						gpuBuffer.offset = stagingBuffer.size;
					}
					else if (cpuCollection.size() * sizeof(LightType) > gpuBuffer.buffer.size)
					{
						_rhi->destroy_buffer(&gpuBuffer.buffer);
						bufferInfo.size = cpuCollection.size() * sizeof(LightType) * 2;
						_rhi->create_buffer(&gpuBuffer.buffer, &bufferInfo);
						_rhi->copy_buffer(&cmdBuffer, &stagingBuffer, &gpuBuffer.buffer);
						gpuBuffer.offset = stagingBuffer.size;
					}
					else
					{
						_rhi->copy_buffer(
							&cmdBuffer,
							&stagingBuffer,
							&gpuBuffer.buffer,
							stagingBuffer.bufferInfo.size,
							0,
							gpuBuffer.offset);
						gpuBuffer.offset += stagingBuffer.bufferInfo.size;
					}
				}
				else if (!_areGPUBuffersAllocated)
				{
					bufferInfo.size = defaultObjectCount * sizeof(LightType);
					_rhi->create_buffer(&gpuBuffer.buffer, &bufferInfo);
				}
				
				LOG_INFO("LightSubmanager::allocated_gpu_buffers(): Allocated {} storage buffer. Size: {}. Offset: {}", get_type_name<LightType>(), gpuBuffer.buffer.size, gpuBuffer.offset)
			}
		
			void setup_point_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_directional_lights_matrices(tasks::TaskGroup& taskGroup);
			void setup_spot_lights_matrices(tasks::TaskGroup& taskGroup);
	};

	IMPLEMENT_SUBMANAGER_EVENT(LightSubmanager)
}