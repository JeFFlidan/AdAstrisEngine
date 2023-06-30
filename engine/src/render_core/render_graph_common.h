#pragma once

#include "rhi/resources.h"
#include <cstdint>
#include <unordered_set>

namespace ad_astris::rcore
{
	class ResourceDesc
	{
		public:
			enum { Unused = ~0u };
		
			enum class Type
			{
				BUFFER,
				TEXTURE,
				UNDEFINED
			};

			ResourceDesc(Type type, const std::string& name, uint32_t logicalIndex)
				: _type(type), _name(name), _logicalIndex(logicalIndex) { }
	
			Type get_type()
			{
				return _type;
			}	

			std::string get_name() { return _name; }
			void set_name(const std::string& newName)
			{
				_name = newName;
			}

			uint32_t get_physical_index()
			{
				return _physicalIndex;
			}
		
			void set_physical_index(uint32_t physicalIndex)
			{
				_physicalIndex = physicalIndex;
			}

			std::unordered_set<uint8_t>& get_written_in_passes()
			{
				return _writtenInPasses;
			}
		
			void add_written_in_pass(uint8_t passIndex)
			{
				_writtenInPasses.insert(passIndex);
			}

			std::unordered_set<uint8_t>& get_read_in_passes()
			{
				return _writtenInPasses;
			}
		
			void add_read_in_pass(uint8_t passIndex)
			{
				_writtenInPasses.insert(passIndex);
			}

			RenderGraphQueue get_queues()
			{
				return _queues;
			}
		
			void add_queue(RenderGraphQueue queue)
			{
				_queues |= queue;
			}

			rhi::ShaderType get_shader_stages()
			{
				return _shaderStages;
			}

			void add_shader_stage(rhi::ShaderType stage)
			{
				_shaderStages |= stage;
			}
		
		private:
			Type _type = Type::UNDEFINED;
			std::string _name;
			uint32_t _physicalIndex = Unused;
			uint32_t _logicalIndex = Unused;
			std::unordered_set<uint8_t> _writtenInPasses;
			std::unordered_set<uint8_t> _readInPasses;
			RenderGraphQueue _queues{ RenderGraphQueue::UNDEFINED };		// Do I need this?
			rhi::ShaderType _shaderStages{ rhi::ShaderType::UNDEFINED };
	};

	class BufferDesc : public ResourceDesc
	{
		public:
			BufferDesc(const std::string& name, uint32_t logicalIndex)
				: ResourceDesc(Type::BUFFER, name, logicalIndex) { }

			/** Sets new BufferInfo. Old buffer usage will be added to the new BufferInfo
			 * @param bufferInfo should be valid pointer to the BufferInfo object.
			 */
			void set_buffer_info(rhi::BufferInfo* bufferInfo)
			{
				rhi::ResourceUsage oldUsage = _info.bufferUsage;
				_info = *bufferInfo;
				_info.bufferUsage |= oldUsage;
			}
		
			rhi::BufferInfo& get_buffer_info()
			{
				return _info;
			}

			void add_buffer_usage(rhi::ResourceUsage usage)
			{
				_info.bufferUsage |= usage;
			}
		
			rhi::ResourceUsage get_buffer_usage()
			{
				return _info.bufferUsage;
			}
		
		private:
			rhi::BufferInfo _info;
			rhi::ShaderType _shaderStages{ rhi::ShaderType::UNDEFINED };
	};

	class TextureDesc : public ResourceDesc
	{
		public:
			TextureDesc(const std::string& name, uint32_t logicalIndex)
				: ResourceDesc(Type::TEXTURE, name, logicalIndex) { }

			/** Sets new TextureInfo. Old texture usage will be added to the new TextureInfo
			* @param textureInfo should be valid pointer to the TextureInfo object.
			*/
			void set_texture_info(rhi::TextureInfo* textureInfo)
			{
				rhi::ResourceUsage oldUsage = _info.textureUsage;
				_info = *textureInfo;
				_info.textureUsage |= oldUsage;
			}
		
			rhi::TextureInfo& get_texture_info()
			{
				return _info;
			}

			void add_texture_usage(rhi::ResourceUsage usage)
			{
				_info.textureUsage |= usage;
			}
		
			rhi::ResourceUsage get_texture_usage()
			{
				return _info.textureUsage;
			}

		private:
			rhi::TextureInfo _info;
	};
}
