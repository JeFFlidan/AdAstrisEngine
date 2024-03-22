#pragma once

#include <stb/stb_rect_pack.h>
#include <vector>
#include <algorithm>

namespace ad_astris
{
	using Rect = stbrp_rect;

	class RectPacker
	{
		public:
			void add_rect(const Rect& rect)
			{
				_rects.push_back(rect);
				_width = std::max(rect.w, _width);
				_height = std::max(rect.h, _height);
			}

			bool pack(uint32_t maxWidth)
			{
				return pack(maxWidth, maxWidth);
			}

			bool pack(uint32_t maxWidth, uint32_t maxHeight)
			{
				while (_width <= (int)maxWidth || _height <= (int)maxHeight)
				{
					int maxSize = std::max(_width, _height);
					if (_nodes.size() < maxSize)
						_nodes.resize(maxSize);
					
					if (stbrp_pack_rects(&_context, _rects.data(), int(_rects.size())))
						return true;
					
					if (_height < _width)
						_height *= 2;
					else
						_width *= 2;
				}
				_width = 0;
				_height = 0;
				return false;
			}

			void reset()
			{
				_rects.clear();
			}

			uint32_t get_width() const { return (uint32_t)_width; }
			uint32_t get_height() const { return (uint32_t)_height; }
			const std::vector<stbrp_node>& get_nodes() const { return _nodes; }
			const std::vector<Rect>& get_rects() const { return _rects; }

		private:
			stbrp_context _context{};
			std::vector<stbrp_node> _nodes;
			std::vector<Rect> _rects;
			int _width{ 0 };
			int _height{ 0 };
	};
}
