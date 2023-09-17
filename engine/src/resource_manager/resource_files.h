#pragma once

#include "file_system/file.h"

namespace ad_astris::resource
{
	template<typename T>
	struct ConversionContext
	{
		
	};
	
	class ResourceFile : public io::File
	{
		public:
			ResourceFile() = default;
			
			template<typename T>
			ResourceFile(ConversionContext<T>& context);
			ResourceFile(const io::URI& uri);
			virtual ~ResourceFile() final override;
			
			virtual void serialize(uint8_t*& data, uint64_t& size) final override;
			virtual void deserialize(uint8_t* data, uint64_t size) final override;

			virtual bool is_valid() final override;
			virtual void destroy_binary_blob() override;
			virtual void accept(IVisitor& visitor) final override;
	};

	class LevelFile : public io::File
	{
		public:
			LevelFile() = default;
			LevelFile(const io::URI& uri);
				
			virtual ~LevelFile() final override;
				
			virtual void serialize(uint8_t*& data, uint64_t& size) final override;
			virtual void deserialize(uint8_t* data, uint64_t size) final override;

			virtual bool is_valid() final override;
			virtual void accept(IVisitor& visitor) final override;
	};

}