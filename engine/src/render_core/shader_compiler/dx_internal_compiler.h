#pragma once

#include "internal_compiler.h"

#ifdef _WIN32
#define DXCOMPILER_ENABLED
#include <wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif

#include <dxc/dxcapi.h>
#include <unordered_set>

namespace ad_astris::rcore::impl
{
	class DXInternalCompiler : public IInternalCompiler
	{
		public:
			virtual void init(ShaderCompilerInitContext& initContext) override;
			virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) override;

		private:
			class IncludeHandler : public IDxcIncludeHandler
			{
				public:
					IncludeHandler(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc, ComPtr<IDxcUtils>& utils);
				
					HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override;
					HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) override
					{
						return _dxcIncludeHandler->QueryInterface(riid, ppvObject);
					}
				
					ULONG STDMETHODCALLTYPE AddRef() override { return 0; }
					ULONG STDMETHODCALLTYPE Release() override { return 0; }

				private:
					const ShaderInputDesc* _inputDesc{ nullptr };
					ShaderOutputDesc* _outputDesc{ nullptr };
					ComPtr<IDxcIncludeHandler> _dxcIncludeHandler;
			};
		
			DxcCreateInstanceProc _dxcCreateInstanceProc;
			std::unordered_set<std::wstring> _targetProfilesForShaderModel6_5 = { L"ms", L"as" };

			void set_shader_format_flags(ShaderInputDesc& inputDesc, std::vector<std::wstring>& compileArgs);
			void set_target_profile_flag(ShaderInputDesc& inputDesc, std::vector<std::wstring>& compileArgs);
			void set_shader_code_flags(ShaderInputDesc& inputDesc, std::vector<std::wstring>& compileArgs);
	};
}
