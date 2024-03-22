#pragma once

#include "engine_core/object.h"
#include <stb/stb_truetype.h>

namespace ad_astris::ecore
{
	struct FontInfo
	{
		std::vector<uint8_t> blob;
		stbtt_fontinfo info;
		int ascent = 0, descent = 0, lineGap = 0;

		void init();	// Must only be called if blob is filled
		void init(const uint8_t* fontBlob, size_t fontBlobSize);
	};
	
	class Font : public Object
	{
		public:
			Font() = default;
			Font(const FontInfo& info, ObjectName* name);

			void set_info(const FontInfo& info) { _info = info; }
			const FontInfo& get_info() const { return _info; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return 0; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Font"; }
			std::string get_type() override { return "font"; }
		
		private:
			FontInfo _info;
			UUID _uuid;
	};
}