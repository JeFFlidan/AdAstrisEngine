#pragma once

#include "engine_core/object.h"
#include "rhi/resources.h"

namespace ad_astris::ecore
{
	class Shader : public Object
	{
		public:
			Shader() = default;

			bool is_shader_compiled()
			{
				return _isCompiled;
			}

			void set_shader_compiled_flag()
			{
				_isCompiled = true;
			}

		private:
			rhi::ShaderInfo _shaderInfo;
			bool _isCompiled{ false };
		
		public:
			// ========== Begin Object interface ==========
					
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;

			// ========== End Object interface ==========
	};
}
