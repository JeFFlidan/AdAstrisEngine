#pragma once

#include "material_parameter.h"

namespace ad_astris::ecore::material_utils
{
	template<typename ...Values>
	struct MaterialParameterTypeGetter
	{
		static MaterialParameterType execute(const MaterialParameterValue& value)
		{
			MaterialParameterType type = MaterialParameterType::UNDEFINED;
			(execute_internal<Values>(value, type), ...);
			return type;
		}

		private:
			template<typename ValueType>
			static void execute_internal(const MaterialParameterValue& value, MaterialParameterType& outType)
			{
				if (std::holds_alternative<ValueType>(value))
					outType = MATERIAL_PARAMETER_TYPE_BY_INDEX[typeid(ValueType)];
			}
	};
}

namespace ad_astris::ecore
{
	using MaterialParameterTypeGetter = material_utils::MaterialParameterTypesPack::Apply<material_utils::MaterialParameterTypeGetter>;
}