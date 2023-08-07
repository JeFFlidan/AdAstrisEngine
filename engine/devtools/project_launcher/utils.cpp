#include "utils.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace ad_astris::devtools::pl_impl;

bool Utils::load_texture_from_file(const std::string& filePath, TextureInfo& textureInfo)
{
	// Load from file
	unsigned char* image_data = stbi_load(filePath.c_str(), &textureInfo.width, &textureInfo.height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	glGenTextures(1, &textureInfo.textureID);
	glBindTexture(GL_TEXTURE_2D, textureInfo.textureID);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureInfo.width, textureInfo.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	return true;
}
