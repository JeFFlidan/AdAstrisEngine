#pragma once

#include "rhi/resources.h"
#include "engine_core/material/material_common.h"
#include "engine/render_core_module.h"
#include <unordered_map>
#include <string>

namespace ad_astris::renderer::impl
{
	// This class stores info about all render passes to create pipelines in material manager. Must be passed to classes
	// that set up render passes for render graph
	class RenderPassContext
	{
		public:
			RenderPassContext(rcore::IRenderGraph* renderGraph) : _renderGraph(renderGraph) { }

			void add_pass_name(ecore::material::ShaderPassType shaderPassType, const std::string& renderPassName)
			{
				auto it = _passNameByItsType.find(shaderPassType);
				if (it != _passNameByItsType.end())
				{
					LOG_ERROR("RenderPassContext::add_pass_name(): Render pass {} has been already added", renderPassName)
					return;
				}

				_passNameByItsType[shaderPassType] = renderPassName;
			}

			rhi::RenderPass* get_render_pass(ecore::material::ShaderPassType shaderPassType)
			{
				auto it = _passNameByItsType.find(shaderPassType);
				if (it == _passNameByItsType.end())
				{
					LOG_FATAL("RenderPassContext::get_reender_pass(): Context doesn't have render pass for shader pass {}", ecore::material::Utils::get_str_shader_pass_type(shaderPassType))
				}

				return _renderGraph->get_physical_pass(it->second);
			}
		
		private:
			rcore::IRenderGraph* _renderGraph{ nullptr };
			std::unordered_map<ecore::material::ShaderPassType, std::string> _passNameByItsType;
	};
}