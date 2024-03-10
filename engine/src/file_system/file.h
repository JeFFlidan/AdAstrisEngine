#pragma once

#include "file_system.h"
#include "utils.h"
#include "core/visitor.h"

#include <stdint.h>
#include <string>

namespace ad_astris::io
{
	class File
	{
		public:
			File() = default;
			File(const io::URI& path) : _path(path) { }
			virtual ~File() {}
		
			virtual void serialize(uint8_t*& data, uint64_t& size);
			virtual void deserialize(uint8_t* data, uint64_t size);
			virtual void serialize(
				std::vector<uint8_t>& inputBinData,
				std::string& inputMetadata,
				std::vector<uint8_t>& outputBinBlob);
			virtual void deserialize(
				std::vector<uint8_t>& inputData,
				std::vector<uint8_t>& outputBinData,
				std::string& outputMetadata);

			virtual bool is_valid()
			{
				return _binBlob && !_metadata.empty();
			}
		
			void set_binary_blob(uint8_t* blob, uint64_t blobSize)
			{
				_binBlob = blob;
				_binBlobSize = blobSize;
			}
		
			uint8_t* get_binary_blob()
			{
				return _binBlob;
			}
		
			uint64_t get_binary_blob_size()
			{
				return _binBlobSize;
			}
			
			URI get_file_path()
			{
				return _path;
			}
		
			std::string get_file_name()
			{
				return Utils::get_file_name(_path);
			}
		
			void set_metadata(const std::string& newMetadata)
			{
				_metadata = newMetadata;
			}
		
			std::string& get_metadata()
			{
				return _metadata;
			}
		
			virtual void destroy_binary_blob()
			{
				// TODO
			}
			virtual void accept(IVisitor& visitor) { }

		protected:
			std::string _metadata;
			URI _path;
			uint8_t* _binBlob{ nullptr };
			uint64_t _binBlobSize{ 0 };
	};
}
