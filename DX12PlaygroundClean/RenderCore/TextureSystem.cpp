#include "TextureSystem.h"

TextureSystem::TextureSystem(DX12Context * DXContext)
{
	mDXContext = DXContext;
}

TextureID TextureSystem::LoadTexture(std::string name, std::wstring path, TextureOptions & opt)
{
	Texture tex;
	tex.Filename = path;
	HR(CreateDDSTextureFromFile12(mDXContext->mD3dDevice.Get(),
		mDXContext->mCmdList.Get(), tex.Filename.c_str(),
		tex.Resource, tex.UploadHeap));
	mTextureIndex[name] = mAllTextures.size();
	mAllTextures.push_back(tex);
}

Texture& TextureSystem::GetTexture(std::string name)
{
	return mAllTextures[mTextureIndex[name]];
}

Texture& TextureSystem::GetTexture(TextureID id)
{
	return mAllTextures[id];
}

TextureID TextureSystem::GetTextureID(std::string name)
{
	return mTextureIndex[name];
}

void TextureSystem::UploadTextures()
{
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NumDescriptors = mAllTextures.size();
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(mDXContext->mD3dDevice->CreateDescriptorHeap(&texHeapDesc,
		IID_PPV_ARGS(&mDXContext->mTextureHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mDXContext->
		mTextureHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mAllTextures.size(); i++)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC texSRVDesc = {};
		texSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		texSRVDesc.ViewDimension = mTexOptions[i].ViewDimension;
		texSRVDesc.Texture2D.MostDetailedMip = mTexOptions[i].MostDetailedMip;
		texSRVDesc.Texture2D.MipLevels = mTexOptions[i].MipLevels;
		texSRVDesc.Format = mAllTextures[i].Resource->GetDesc().Format;
		mDXContext->mD3dDevice->CreateShaderResourceView(mAllTextures[i].Resource.Get(), &texSRVDesc, hDescriptor);
		hDescriptor.Offset(1, mDXContext->mCbvSrvUavDescriptorSize);
	}
}

TextureSystem::~TextureSystem()
{
}