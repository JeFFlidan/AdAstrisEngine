#pragma once

#include "rhi/engine_rhi.h"
#include "render_graph.h"
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

			std::string get_name()
			{
				return _name;
			}
		
			void set_name(const std::string& newName)
			{
				_name = newName;
			}

			uint32_t get_logical_index()
			{
				return _logicalIndex;
			}

			void set_logical_index(uint32_t newLogicalIndex)
			{
				_logicalIndex = newLogicalIndex;
			}

			std::unordered_set<uint32_t>& get_written_in_passes()
			{
				return _writtenInPasses;
			}
		
			void add_written_in_pass(uint8_t passIndex)
			{
				_writtenInPasses.insert(passIndex);
			}

			std::unordered_set<uint32_t>& get_read_in_passes()
			{
				return _readInPasses;
			}
		
			void add_read_in_pass(uint8_t passIndex)
			{
				_readInPasses.insert(passIndex);
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
			uint32_t _logicalIndex = Unused;
			std::unordered_set<uint32_t> _writtenInPasses;
			std::unordered_set<uint32_t> _readInPasses;
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
				if (bufferInfo)
				{
					rhi::ResourceUsage oldUsage = _info.bufferUsage;
					_info = *bufferInfo;
					_info.bufferUsage |= oldUsage;
					_hasInfo = true;
				}
			}

			bool has_buffer_info()
			{
				return _hasInfo;
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
			bool _hasInfo{ false };
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
				if (textureInfo)
				{
					rhi::ResourceUsage oldUsage = _info.textureUsage;
					_info = *textureInfo;
					_info.textureUsage |= oldUsage;
					_hasInfo = true;
				}
			}

			bool has_texture_info()
			{
				return _hasInfo;
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
			bool _hasInfo{ false };
	};
}