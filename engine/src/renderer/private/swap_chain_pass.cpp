#include "swap_chain_pass.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

SwapChainPass::SwapChainPass(RenderingInitContext& initContext)
	: RenderingBase(initContext)
{
	
}

void SwapChainPass::prepare_render_pass()
{
	
}

void SwapChainPass::execute(rhi::CommandBuffer* cmd)
{
	if (_mainWindow->is_running())
		UI_WINDOW_BACKEND()->draw(cmd);
}
