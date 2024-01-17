#pragma once

#include <imgui/imgui.h>
#include <string>
#include <cstring>
#include <functional>

namespace ad_astris::uicore
{
	template<typename T>
	using ComboBoxValueGetter = std::function<const char*(const T&)>;

	template<typename T, template <typename, typename> class Container>
	class ComboBox
	{
		public:
			ComboBox(
				const std::string& label,
				const Container<T, std::allocator<T>>& values,
				const ComboBoxValueGetter<T>& valueGetter)
					: _label(label), _objects(values), _valueGetter(valueGetter) { }

			void draw()
			{
				const char* selectedValue = _valueGetter(_selectedObject);
				if (ImGui::BeginCombo(_label.c_str(), selectedValue))
				{
					for (const T& obj : _objects)
					{
						const char* currentValue = _valueGetter(obj);
						bool isSelected = strcmp(currentValue, selectedValue) == 0;
						if (ImGui::Selectable(currentValue, isSelected))
						{
							_selectedObject = obj;
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			const T& get_selected_object() const
			{
				return _selectedObject;
			}

			void set_default_object(const T& object)
			{
				_selectedObject = object;
			}

		private:
			std::string _label;
			T _selectedObject;
			const Container<T, std::allocator<T>>& _objects;
			ComboBoxValueGetter<T> _valueGetter;
	};
}