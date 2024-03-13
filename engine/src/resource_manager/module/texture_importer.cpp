#include "texture_importer.h"
#include "engine_core/texture/color8bit.h"
#include "engine_core/texture/color16bit.h"
#include "core/global_objects.h"
#include <basis_universal/transcoder/basisu_transcoder.h>
#include <stb_image/stb_image.h>

using namespace ad_astris;
using namespace resource::impl;

void TextureImporter::init()
{
	basist::basisu_transcoder_init();
}

void TextureImporter::import(const io::URI& path, ecore::TextureInfo& outTextureInfo)
{
	std::vector<uint8_t> textureRawData;
	io::Utils::read_file(FILE_SYSTEM(), path, textureRawData);
	std::string extension = io::Utils::get_file_extension(path);

	if (extension == "basis")
	{
		// TODO
	}
	else if (extension == "ktx2")
	{
		// TODO
	}
	else if (extension == "hdr")
	{
		// TODO
	}
	else
	{
		int width = 0, height = 0, channels = 0;
		if (stbi_is_16_bit_from_memory(textureRawData.data(), textureRawData.size()))
		{
			void* textureData = stbi_load_16_from_memory(textureRawData.data(), textureRawData.size(), &width, &height, &channels, 0);
			
			outTextureInfo.width = width;
			outTextureInfo.height = height;
			outTextureInfo.is16Bit = true;
			outTextureInfo.size = width * height * channels * sizeof(uint16_t);
			switch (channels)
			{
				case 1:
				{
					outTextureInfo.format = rhi::Format::BC4_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
				case 2:
				{
					outTextureInfo.format = rhi::Format::BC5_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
				case 3:
				{
					outTextureInfo.format = rhi::Format::BC1_RGBA_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::ONE };

					struct Color3Channels
					{
						uint16_t r, g, b;
					};

					Color3Channels* textureData3Channels = static_cast<Color3Channels*>(textureData);
					ecore::Color16Bit* color16BitData = new ecore::Color16Bit[width * height];
					for (size_t i = 0; i < width * height; ++i)
					{
						color16BitData[i] = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };
					}

					outTextureInfo.data = reinterpret_cast<uint8_t*>(color16BitData);
					outTextureInfo.size = width * height * 4 * sizeof(uint16_t);
					break;
				}
				case 4:
				{
					outTextureInfo.format = rhi::Format::BC3_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
			}
			
			stbi_image_free(textureData);
		}
		else
		{
			void* textureData = stbi_load_from_memory(textureRawData.data(), textureRawData.size(), &width, &height, &channels, 0);

			outTextureInfo.width = width;
			outTextureInfo.height = height;
			outTextureInfo.is16Bit = false;
			outTextureInfo.size = width * height * channels;
			switch (channels)
			{
				case 1:
				{
					outTextureInfo.format = rhi::Format::BC4_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
				case 2:
				{
					outTextureInfo.format = rhi::Format::BC5_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
				case 3:
				{
					outTextureInfo.format = rhi::Format::BC1_RGBA_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::ONE };

					struct Color3Channels
					{
						uint8_t r, g, b;
					};

					Color3Channels* textureData3Channels = static_cast<Color3Channels*>(textureData);
					ecore::Color8Bit* color8BitData = new ecore::Color8Bit[width * height];

					for (size_t i = 0; i != width * height; ++i)
					{
						color8BitData[i] = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };
					}

					outTextureInfo.size = width * height * 4;
					outTextureInfo.data = reinterpret_cast<uint8_t*>(color8BitData);

					break;
				}
				case 4:
				{
					outTextureInfo.format = rhi::Format::BC3_UNORM;
					outTextureInfo.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
					outTextureInfo.data = new uint8_t[outTextureInfo.size];
					memcpy(outTextureInfo.data, textureData, outTextureInfo.size);
					break;
				}
			}

			stbi_image_free(textureData);
		}
	}
}
