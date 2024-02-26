#include "dx_internal_compiler.h"
#include "core/module_manager.h"
#include "file_system/utils.h"
#include "profiler/logger.h"
#include "core/utils.h"
#include <memory>

using namespace ad_astris;
using namespace rcore;
using namespace impl;

void DXInternalCompiler::init(ShaderCompilerInitContext& initContext)
{
	FARPROC registerFunc = initContext.moduleManager->load_third_party_module("DXCompiler", "DxcCreateInstance");
	_dxcCreateInstanceProc = reinterpret_cast<DxcCreateInstanceProc>(registerFunc);

	ComPtr<IDxcCompiler3> dxcCompiler;
	HRESULT result = _dxcCreateInstanceProc(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(result));

	ComPtr<IDxcVersionInfo> versionInfo;
	result = dxcCompiler->QueryInterface(IID_PPV_ARGS(&versionInfo));
	assert(SUCCEEDED(result));

	uint32_t majorVer = 0;
	uint32_t minorVer = 0;
	result = versionInfo->GetVersion(&majorVer, &minorVer);
	assert(SUCCEEDED(result));

	LOG_INFO("DXInternalCompiler::init(): Loaded and initialized DXCompiler module. Version: {}.{}", majorVer, minorVer)
}

void DXInternalCompiler::compile(io::FileSystem* fileSystem, ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc)
{
	std::vector<std::wstring> compileArgs;
	set_shader_format_flags(inputDesc, compileArgs);
	set_target_profile_flag(inputDesc, compileArgs);
	set_shader_code_flags(inputDesc, compileArgs);

	std::vector<const wchar_t*> compilerArgsPtrs;
	for (auto& compileArg : compileArgs)
		compilerArgsPtrs.push_back(compileArg.c_str());

	ComPtr<IDxcCompiler3> dxcCompiler;
	HRESULT result = _dxcCreateInstanceProc(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(result));
	
	ComPtr<IDxcUtils> dxcUtils;
	result = _dxcCreateInstanceProc(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	IncludeHandler includeHandler(inputDesc, outputDesc, dxcUtils);

	std::vector<uint8_t> rawShaderData;
	io::Utils::read_file(fileSystem, inputDesc.shaderPath, rawShaderData);
	DxcBuffer srcBuffer;
	srcBuffer.Ptr = rawShaderData.data();
	srcBuffer.Size = rawShaderData.size();
	srcBuffer.Encoding = DXC_CP_ACP;

	ComPtr<IDxcResult> dxcResult;
	result = dxcCompiler->Compile(
		&srcBuffer,
		compilerArgsPtrs.data(),
		compileArgs.size(),
		&includeHandler,
		IID_PPV_ARGS(&dxcResult));
	assert(SUCCEEDED(result));

	ComPtr<IDxcBlobUtf8> dxcErrors = nullptr;
	result = dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&dxcErrors), nullptr);
	assert(SUCCEEDED(result));
	if (dxcErrors && dxcErrors->GetStringLength() != 0)
	{
		LOG_ERROR("DXInternalCompiler::compile(): {}", dxcErrors->GetStringPointer())
		return;
	}

	ComPtr<IDxcBlob> dxcShaderObject = nullptr;
	result = dxcResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxcShaderObject), nullptr);
	assert(SUCCEEDED(result));
	if (dxcShaderObject)
	{
		outputDesc.data = (const uint8_t*)dxcShaderObject->GetBufferPointer();
		outputDesc.dataSize = dxcShaderObject->GetBufferSize();
		outputDesc.dependencies.insert(inputDesc.shaderPath.c_str());

		auto internalBlob = std::make_shared<ComPtr<IDxcBlob>>();
		*internalBlob = dxcShaderObject;
		outputDesc.internalBlob = internalBlob;
	}

	if (inputDesc.format == rhi::ShaderFormat::HLSL6)
	{
		// TODO some features like saving debug and reflection data
	}
}

void DXInternalCompiler::set_shader_format_flags(ShaderInputDesc& inputDesc, std::vector<std::wstring>& compileArgs)
{
	switch (inputDesc.format)
	{
		case rhi::ShaderFormat::UNDEFINED:
			LOG_FATAL("DXInternalCompiler::compile(): Can't compile shader with undefined format")
		case rhi::ShaderFormat::GLSL_TO_SPIRV:
		case rhi::ShaderFormat::GLSL_TO_HLSL6:
			LOG_FATAL("DXInternalCompiler::compile(): Can't compile glsl shader.")
		case rhi::ShaderFormat::HLSL6:
			// TODO FOR D3D12
			break;
		case rhi::ShaderFormat::HLSL_TO_SPIRV:
			compileArgs.push_back(L"-spirv");
			compileArgs.push_back(L"-fspv-target-env=vulkan1.3");
			compileArgs.push_back(L"-fvk-use-dx-layout");
			compileArgs.push_back(L"-fvk-allow-rwstructuredbuffer-arrays");
				
			compileArgs.push_back(L"-fvk-t-shift");
			compileArgs.push_back(L"1000");
			compileArgs.push_back(L"0");

			compileArgs.push_back(L"-fvk-u-shift");
			compileArgs.push_back(L"2000");
			compileArgs.push_back(L"0");
			
			compileArgs.push_back(L"-fvk-s-shift");
			compileArgs.push_back(L"3000"); 
			compileArgs.push_back(L"0");
		
#ifdef DEBUG_SHADER
			compileArgs.push_back(L"/Zi");
#endif
			break;
	}
}

void DXInternalCompiler::set_target_profile_flag(
	ShaderInputDesc& inputDesc,
	std::vector<std::wstring>& compileArgs)
{
	compileArgs.push_back(L"-T");
	std::wstring& targetProfile = compileArgs.emplace_back();
	switch (inputDesc.type)
	{
		case rhi::ShaderType::VERTEX:
			targetProfile = L"vs";
			break;
		case rhi::ShaderType::FRAGMENT:
			targetProfile = L"ps";
			break;
		case rhi::ShaderType::COMPUTE:
			targetProfile = L"cs";
			break;
		case rhi::ShaderType::TESSELLATION_CONTROL:
			targetProfile = L"hs";
			break;
		case rhi::ShaderType::TESSELLATION_EVALUATION:
			targetProfile = L"ds";
			break;
		case rhi::ShaderType::MESH:
			targetProfile = L"ms";
			break;
		case rhi::ShaderType::TASK:
			targetProfile = L"as";
			break;
	}

	switch (inputDesc.minHlslShaderModel)
	{
		case rhi::HLSLShaderModel::SM_6_0:
			if (_targetProfilesForShaderModel6_5.find(targetProfile) == _targetProfilesForShaderModel6_5.end())
				targetProfile += L"_6_0";
			break;
		case rhi::HLSLShaderModel::SM_6_1:
			if (_targetProfilesForShaderModel6_5.find(targetProfile) == _targetProfilesForShaderModel6_5.end())
				targetProfile += L"_6_1";
			break;
		case rhi::HLSLShaderModel::SM_6_2:
			if (_targetProfilesForShaderModel6_5.find(targetProfile) == _targetProfilesForShaderModel6_5.end())
				targetProfile += L"_6_2";
			break;
		case rhi::HLSLShaderModel::SM_6_3:
			if (_targetProfilesForShaderModel6_5.find(targetProfile) == _targetProfilesForShaderModel6_5.end())
				targetProfile += L"_6_3";
			break;
		case rhi::HLSLShaderModel::SM_6_4:
			if (_targetProfilesForShaderModel6_5.find(targetProfile) == _targetProfilesForShaderModel6_5.end())
				targetProfile += L"_6_4";
			break;
		case rhi::HLSLShaderModel::SM_6_5:
			targetProfile += L"_6_5";
			break;
		case rhi::HLSLShaderModel::SM_6_6:
			targetProfile += L"_6_6";
			break;
		case rhi::HLSLShaderModel::SM_6_7:
			targetProfile += L"_6_7";
			break;
	}

	std::wstring newProfileName(compileArgs.back());
	std::string profileName;
	CoreUtils::convert_string(newProfileName, profileName);
}

void DXInternalCompiler::set_shader_code_flags(ShaderInputDesc& inputDesc, std::vector<std::wstring>& compileArgs)
{
	for (auto& define : inputDesc.defines)
	{
		compileArgs.push_back(L"-D");
		CoreUtils::convert_string(define, compileArgs.emplace_back());
	}

	for (auto& includePath : inputDesc.includePaths)
	{
		compileArgs.push_back(L"-I");
		CoreUtils::convert_string(includePath, compileArgs.emplace_back());
	}
	
	compileArgs.push_back(L"-E");
	CoreUtils::convert_string(inputDesc.entryPoint, compileArgs.emplace_back());

	// Shader name for error messages
	CoreUtils::convert_string(io::Utils::get_file_name(inputDesc.shaderPath), compileArgs.emplace_back());
}

DXInternalCompiler::IncludeHandler::IncludeHandler(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc, ComPtr<IDxcUtils>& utils)
	: _inputDesc(&inputDesc), _outputDesc(&outputDesc)
{
	HRESULT result = utils->CreateDefaultIncludeHandler(&_dxcIncludeHandler);
	assert(SUCCEEDED(result));
}

HRESULT DXInternalCompiler::IncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
{
	HRESULT result = _dxcIncludeHandler->LoadSource(pFilename, ppIncludeSource);
	if (SUCCEEDED(result))
	{
		std::string path;
		CoreUtils::convert_string(pFilename, path);
		_outputDesc->dependencies.insert(path);
	}
	return result;
}
