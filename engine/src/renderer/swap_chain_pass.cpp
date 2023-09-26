#include "swap_chain_pass.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

SwapChainPass::SwapChainPass(RenderingInitContext& initContext, rhi::UIWindowBackend* uiWindowBackend)
	: RenderingBase(initContext), _uiWindowBackend(uiWindowBackend)
{
	
}

void SwapChainPass::prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager)
{
	
}

void SwapChainPass::execute(rhi::CommandBuffer* cmd)
{
	if (_mainWindow->is_running())
		_uiWindowBackend->draw(cmd);
}
