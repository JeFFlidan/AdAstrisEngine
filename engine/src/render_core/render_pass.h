#pragma once

#include "engine/render_core_module.h"
#include "render_graph_common.h"
#include "api.h"

namespace ad_astris::rcore::impl
{
	class RENDER_CORE_API RenderPass final : public IRenderPass
	{
		public:
			RenderPass(IRenderGraph* renderGraph, const std::string& name, uint32_t index, RenderGraphQueue queue);
			virtual ~RenderPass() override;

			virtual std::string get_name() override
			{
				return _name;
			}

			virtual uint32_t get_logical_pass_index() override
			{
				return _logicalIndex;
			}

			virtual uint32_t get_physical_pass_index() override
			{
				return _physicalIndex;
			}
		
			virtual void set_physical_pass_index(uint32_t newPhysicalIndex) override
			{
				_physicalIndex = newPhysicalIndex;
			}
			// TODO Do I need this depth stencil input
			virtual TextureDesc* set_depth_stencil_input(const std::string& inputName) override;
			virtual TextureDesc* set_depth_stencil_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) override;
			virtual TextureDesc* add_color_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) override;
			virtual TextureDesc* add_history_input(const std::string& inputName) override;

			virtual TextureDesc* add_texture_input(
				const std::string& inputName,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) override;

			virtual TextureDesc* add_texture_input(
				const std::string& inputName,
				rhi::TextureInfo* textureInfo,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) override;
		
			virtual TextureDesc* add_storage_texture_input(const std::string& inputName) override;
			virtual TextureDesc* add_storage_texture_output(
				const std::string& outputName,
				rhi::TextureInfo* textureInfo) override;

			virtual std::pair<TextureDesc*, TextureDesc*>& add_texture_blit_input_and_output(
				const std::string& inputName,
				const std::string& outputName,
				rhi::TextureInfo* outputTextureInfo) override;

			virtual BufferDesc* add_uniform_buffer_input(
				const std::string& inputName,
				rhi::BufferInfo* bufferInfo,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) override;

			virtual BufferDesc* add_storage_buffer_read_only_input(
				const std::string& inputName,
				rhi::ShaderType shaderStages = rhi::ShaderType::UNDEFINED) override;
			virtual BufferDesc* add_storage_buffer_read_write_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo) override;

			virtual BufferDesc* add_transfer_output(
				const std::string& outputName,
				rhi::BufferInfo* bufferInfo) override;

			virtual BufferDesc* add_vertex_buffer_input(const std::string& inputName) override;
			virtual BufferDesc* add_index_buffer_input(const std::string& inputName) override;
			virtual BufferDesc* add_indirect_buffer_input(const std::string& inputName) override;

			TextureDesc* get_depth_stencil_input()
			{
				return _depthStencilInput;			
			}
			
			TextureDesc* get_depth_stencil_output()
			{
				return _depthStencilOutput;
			}
			
			std::vector<TextureDesc*>& get_color_outputs()
			{
				return _colorOutputs;
			}
			
			std::vector<TextureDesc*>& get_history_inputs()
			{
				return _historyInputs;
			}
			
			std::vector<TextureDesc*>& get_custom_textures()
			{
				return _customTextures;
			}
			
			std::vector<TextureDesc*>& get_storage_texture_inputs()
			{
				return _storageTextureInputs;
			}
			
			std::vector<TextureDesc*>& get_storage_texture_outputs()
			{
				return _storageTextureOutputs;
			}
			
			std::vector<std::pair<TextureDesc*, TextureDesc*>>& get_blit_textures()
			{
				return _blitTextureInputsAndOutputs;
			}
			
			std::vector<BufferDesc*>& get_uniform_buffer_inputs()
			{
				return _uniformBufferInputs;
			}
			
			std::vector<BufferDesc*>& get_storage_buffer_inputs()
			{
				return _storageBufferInputs;
			}
			
			std::vector<BufferDesc*>& get_storage_buffer_outputs()
			{
				return _storageBufferOutputs;	
			}
			
			std::vector<BufferDesc*>& get_transfer_outputs()
			{
				return _transferOutputs;	
			}
			
			std::vector<BufferDesc*>& get_vertex_buffer_inputs()
			{
				return _vertexBufferInputs;
			}
			
			std::vector<BufferDesc*>& get_index_buffer_inputs()
			{
				return _indexBufferInputs;
			}
			
			std::vector<BufferDesc*>& get_indirect_buffer_inputs()
			{
				return _indirectBufferInputs;
			}

		private:
			IRenderGraph* _renderGraph{ nullptr };
			std::string _name;
			uint32_t _logicalIndex = ResourceDesc::Unused;
			uint32_t _physicalIndex = ResourceDesc::Unused;
			RenderGraphQueue _queue;

			TextureDesc* _depthStencilInput{ nullptr };
			TextureDesc* _depthStencilOutput{ nullptr };
			std::vector<TextureDesc*> _colorOutputs;
			std::vector<TextureDesc*> _historyInputs;
			std::vector<TextureDesc*> _customTextures;
			std::vector<TextureDesc*> _storageTextureInputs;
			std::vector<TextureDesc*> _storageTextureOutputs;
			std::vector<std::pair<TextureDesc*, TextureDesc*>> _blitTextureInputsAndOutputs;
			std::vector<BufferDesc*> _uniformBufferInputs;
			std::vector<BufferDesc*> _storageBufferInputs;
			std::vector<BufferDesc*> _storageBufferOutputs;
			std::vector<BufferDesc*> _transferOutputs;
			std::vector<BufferDesc*> _vertexBufferInputs;
			std::vector<BufferDesc*> _indexBufferInputs;
			std::vector<BufferDesc*> _indirectBufferInputs;

			void define_shader_stages(ResourceDesc* desc, rhi::ShaderType shaderStages);
	};
}