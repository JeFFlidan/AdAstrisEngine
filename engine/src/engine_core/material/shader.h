#pragma once

#include "shader_common.h"
#include "engine_core/object.h"
#include "rhi/resources.h"

namespace ad_astris::ecore
{
	/** @brief This class has info about one shader. After loading using ResourceManager, an object of this class
	 * will contain non-compiled shader code. To compile it into spv you must use ShaderCompiler from RenderCore
	 * module. After compilation, the old non-compiled data will be deleted and replaced with the spv binary blob
	 */
	class Shader : public Object
	{
		public:
			Shader() = default;
			// Need this constructor to tell the engine that we want to add a new shader which has been never loaded
			Shader(ObjectName* shaderName);

			~Shader() override;

			bool is_shader_compiled()
			{
				return _isCompiled;
			}

			void set_shader_compiled_flag()
			{
				if (!_isCompiled)
					_file->destroy_binary_blob();
				_isCompiled = true;
			}

			rhi::ShaderType get_shader_type()
			{
				return _shaderInfo.shaderType;
			}

			rhi::ShaderInfo& get_shader_info()
			{
				return _shaderInfo;
			}

			shader::CompilationContext get_compilation_context();
		
		private:
			rhi::ShaderInfo _shaderInfo;
			io::IFile* _file{ nullptr };
			bool _isCompiled{ false };
			UUID _uuid;

			rhi::ShaderType get_shader_type_by_file_ext(const io::URI& path);
		
		public:
			// ========== Begin Object interface ==========
					
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* objectName) override;
		
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
