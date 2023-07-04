#pragma once

#include "engine_core/object.h"
#include "rhi/resources.h"

namespace ad_astris::ecore
{
	class Shader : public Object
	{
		public:
			Shader() = default;
			// Need this constructor to tell the engine that we want to add a new shader which has been never loaded
			Shader(const std::string& shaderName);

			bool is_shader_compiled()
			{
				return _isCompiled;
			}

			void set_shader_compiled_flag()
			{
				_isCompiled = true;
			}

			rhi::ShaderType get_shader_type()
			{
				return _shaderInfo.shaderType;
			}
		
		private:
			rhi::ShaderInfo _shaderInfo;
			//io::IFile* _file{ nullptr };
			bool _isCompiled{ false };
			UUID _uuid;

			rhi::ShaderType get_shader_type_by_file_ext(const io::URI& path);
		
		public:
			// ========== Begin Object interface ==========
					
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) override;
		
			virtual uint64_t get_size() override
			{
				return _shaderInfo.size;
			}
		
			virtual bool is_resource() override
			{
				return true;
			}
		
			virtual UUID get_uuid()
			{
				return _uuid;
			}
		
			virtual std::string get_description() override
			{
				// TODO
				return std::string();
			}
		
			virtual std::string get_type() override
			{
				return "shader";
			}

			// ========== End Object interface ==========
	};
}
