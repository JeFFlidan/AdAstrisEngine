#include <iostream>

#include <json.hpp>

#include <asset_loader.h>
#include <material_asset.h>

namespace assets
{
	void parse_textures_and_properties(nlohmann::json& data, MaterialInfo& info)
	{
		for (auto& it : data.items())
		{
			if (it.key().find("texture_") == 0)
			{
				info.textures[it.key()] = it.value();
			}
			else if (it.key().find("prop_") == 0)
			{
				info.customProperties[it.key()] = it.value();
			}
		}
	}

	nlohmann::json parse_textures_and_properties(MaterialInfo& info)
	{
		nlohmann::json data;

		for (auto it1_s = info.textures.cbegin(), it1_e = info.textures.cend(),
			it2_s = info.customProperties.cbegin(), it2_e = info.customProperties.cend(); 
			it1_s != it1_e || it2_s != it2_e;)
		{
			if (it1_s != it1_e)
			{
				data[it1_s->first] = it1_s->second;
				++it1_s;
			}
			if (it2_s != it2_e)
			{
				data[it2_s->first] = it2_s->second;
				++it2_s;
			}
		}

		return data;
	}

	MaterialMode parse_mode(const std::string& mode)
	{
		if (mode == "opaque")
			return MaterialMode::OPAQUE;
		else if (mode == "transparent")
			return MaterialMode::TRANSPARENT;
		else if (mode == "masked")
			return MaterialMode::MASKED;

		return MaterialMode::NONE;
	}

	std::string parse_mode(MaterialMode mode)
	{
		switch ((int)mode)
		{
			case (int)MaterialMode::OPAQUE:
				return "opaque";
			case (int)MaterialMode::TRANSPARENT:
				return "transparent";
			case (int)MaterialMode::MASKED:
				return "masked";
		}
		
		return "none";
	}

	MaterialInfo read_material_info(AssetFile* file)
	{
		MaterialInfo info;
		nlohmann::json matData = nlohmann::json::parse(file->json);
		info.baseEffect = matData["base_effect"];
		info.materialName = matData["material_name"];
		parse_textures_and_properties(matData, info);
		std::string mode = matData["mode"];
		info.mode = parse_mode(mode);
		return info;
	}

	AssetFile pack_material(MaterialInfo* info)
	{
		nlohmann::json matData = parse_textures_and_properties(*info);
		matData["base_effect"] = info->baseEffect;
		matData["material_name"] = info->materialName;
		matData["mode"] = parse_mode(info->mode);

		AssetFile file;
		file.type[0] = 'M';
		file.type[1] = 'A';
		file.type[2] = 'T';
		file.type[3] = 'E';
		file.version = 1;

		file.json = matData.dump();
		file.binaryBlob = { '1' };

		return file;
	}
}
