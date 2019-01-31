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
};

static TextureOptions DefaultTextureOptions()
{
	return
	{
		DDS,
		D3D12_SRV_DIMENSION_TEXTURE2D,
	};
}

class TextureSystem
{
private:
	DX12Context* mDXContext;
	std::vector<Texture> mAllTextures;
	std::vector<TextureOptions> mTexOptions;
	std::unordered_map<std::string, TextureID> mTextureIndex;
	Texture mSkybox;

public:
	ComPtr<ID3D12DescriptorHeap> mTextureHeap;
	u32 mSkyboxIndex;
	u32 mShadowMapIndex;
	u32 mNullCubemapIndex;
	u32 mNullTextureIndex;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNullGPUHandle;

	TextureSystem(DX12Context* DXContext);
	TextureID	LoadTexture(std::string name, std::wstring path, TextureOptions& opt);
	void	LoadSkybox(std::string name, std::wstring path);
	TextureID	LoadTexture2D(std::string name, std::wstring path);

	Texture&	GetTexture(std::string name);
	Texture&	GetTexture(TextureID id);
	Texture&	GetSkyBox();
	TextureID	GetTextureID(std::string name);
	TextureID	GetSkyBoxID();

	void		UploadTextures();
	~TextureSystem();
};
