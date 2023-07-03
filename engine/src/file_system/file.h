#pragma once

#include "file_system.h"

#include <stdint.h>
#include <string>

namespace ad_astris::io
{
	template<typename T>
	struct ConversionContext
	{
		
	};
	
	struct Name
	{
		std::string name{};
		uint32_t nameID{ 0 };
	};
	
	class IFile
	{
		public:
			virtual ~IFile() {}
			virtual void serialize(uint8_t*& data, uint64_t& size) = 0;
			virtual void deserialize(uint8_t* data, uint64_t size) = 0;

			virtual bool is_valid() = 0;
			virtual uint8_t* get_binary_blob() = 0;
			virtual uint64_t get_binary_blob_size() = 0;
		
			URI get_file_path();
			std::string get_file_name();
			void set_metadata(std::string& newMetaData);
			std::string& get_metadata();

		protected:
			std::string _metaData;
			URI _path;
	};
	
	class ResourceFile : public IFile
	{
		public:
			template<typename T>
			ResourceFile(ConversionContext<T>& context);
			ResourceFile(const URI& uri);
			virtual ~ResourceFile() final override;
		
			virtual void serialize(uint8_t*& data, uint64_t& size) final override;
			virtual void deserialize(uint8_t* data, uint64_t size) final override;

			virtual bool is_valid() final override;
			virtual uint8_t* get_binary_blob() final override;
			virtual uint64_t get_binary_blob_size() final override;
			
		private:
			uint8_t* _binBlob{ nullptr };
			uint64_t _binBlobSize{ 0 };
	};

	class LevelFile : public IFile
	{
		public:
			LevelFile(const URI& uri);
			
			virtual ~LevelFile() final override;
			
			virtual void serialize(uint8_t*& data, uint64_t& size) final override;
			virtual void deserialize(uint8_t* data, uint64_t size) final override;

			virtual bool is_valid() final override;
			virtual uint8_t* get_binary_blob() final override;
			virtual uint64_t get_binary_blob_size() final override;
	};

	// class ConfigFile : public IFile
	// {
	// 	public:
	// 		ConfigFile(URI& uri);
	// 		virtual ~ConfigFile() final override;
	//
	// 		virtual void serialize(uint8_t* data, uint64_t& size) final override;
	// 		virtual void deserialize(uint8_t* data, uint64_t size) final override;
	//
	// 		virtual bool is_valid() final override;
	// 		virtual URI get_file_path() final override;
	// 		virtual std::string get_file_name() final override;
	// 		virtual uint8_t* get_binary_blob() final override;
	// 		virtual uint64_t get_binary_blob_size() final override;
	// 		virtual std::string& get_metadata() final override;
	// };
}
