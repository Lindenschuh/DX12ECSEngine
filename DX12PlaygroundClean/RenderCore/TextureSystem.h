#pragma once
#include "../Core/Default.h"
#include "DXData.h"
#include "../Ext/DDSTextureLoader.h"

enum TextureFileType
{
	DDS,
};
struct TextureOptions
{
	TextureFileType type;
	D3D12_SRV_DIMENSION ViewDimension;
	u32 MostDetailedMip;
	s32 MipLevels;
};

static TextureOptions DefaultTextureOptions()
{
	return
	{
		DDS,
		D3D12_SRV_DIMENSION_TEXTURE2D,
		0,
		-1
	};
}

class TextureSystem
{
private:
	DX12Context* mDXContext;
	std::vector<Texture> mAllTextures;
	std::vector<TextureOptions> mTexOptions;
	std::unordered_map<std::string, TextureID> mTextureIndex;
public:
	TextureSystem(DX12Context* DXContext);
	TextureID	LoadTexture(std::string name, std::wstring path, TextureOptions& opt);
	Texture&	GetTexture(std::string name);
	Texture&	GetTexture(TextureID id);
	TextureID	GetTextureID(std::string name);
	void		UploadTextures();
	~TextureSystem();
};
