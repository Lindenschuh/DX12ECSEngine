#include "TextureSystem.h"

TextureSystem::TextureSystem(DX12Context * DXContext)
{
	mDXContext = DXContext;
}

TextureID TextureSystem::LoadTexture(std::string name, std::wstring path, TextureOptions & opt)
{
	Texture tex;
	TextureID id = mAllTextures.size();
	tex.Filename = path;
	HR(CreateDDSTextureFromFile12(mDXContext->mD3dDevice.Get(),
		mDXContext->mCmdList.Get(), tex.Filename.c_str(),
		tex.Resource, tex.UploadHeap));

	mTextureIndex[name] = id;
	mAllTextures.push_back(tex);
	mTexOptions.push_back(opt);
	return id;
}

TextureID TextureSystem::LoadTexture2D(std::string name, std::wstring path)
{
	TextureOptions TexOptions;
	TexOptions.type = DDS;
	TexOptions.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	return LoadTexture(name, path, TexOptions);
}

void TextureSystem::LoadSkybox(std::string name, std::wstring path)
{
	mSkybox.Filename = path;
	HR(CreateDDSTextureFromFile12(mDXContext->mD3dDevice.Get(),
		mDXContext->mCmdList.Get(), mSkybox.Filename.c_str(),
		mSkybox.Resource, mSkybox.UploadHeap));
}

Texture& TextureSystem::GetTexture(std::string name)
{
	return mAllTextures[mTextureIndex[name]];
}

Texture& TextureSystem::GetTexture(TextureID id)
{
	return mAllTextures[id];
}

Texture & TextureSystem::GetSkyBox()
{
	return mSkybox;
}

TextureID TextureSystem::GetTextureID(std::string name)
{
	return mTextureIndex[name];
}

TextureID TextureSystem::GetSkyBoxID()
{
	return  mAllTextures.size();
}

void TextureSystem::UploadTextures()
{
	mSkyboxIndex = mAllTextures.size();
	mShadowMapIndex = mSkyboxIndex + 1;
	mNullCubemapIndex = mShadowMapIndex + 1;
	mNullTextureIndex = mNullCubemapIndex + 1;

	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	// Texutures + skybox;
	texHeapDesc.NumDescriptors = mAllTextures.size() + 4;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(mDXContext->mD3dDevice->CreateDescriptorHeap(&texHeapDesc,
		IID_PPV_ARGS(&mTextureHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(
		mTextureHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mAllTextures.size(); i++)
	{
		D3D12_RESOURCE_DESC resourceDesc = mAllTextures[i].Resource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC texSRVDesc = {};
		texSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		texSRVDesc.ViewDimension = mTexOptions[i].ViewDimension;

		texSRVDesc.Texture2D.MostDetailedMip = 0;
		texSRVDesc.Texture2D.ResourceMinLODClamp = 0;
		texSRVDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

		texSRVDesc.TextureCube.MostDetailedMip = 0;
		texSRVDesc.TextureCube.ResourceMinLODClamp = 0;
		texSRVDesc.TextureCube.MipLevels = resourceDesc.MipLevels;

		texSRVDesc.Format = resourceDesc.Format;

		mDXContext->mD3dDevice->CreateShaderResourceView(mAllTextures[i].Resource.Get(), &texSRVDesc, hDescriptor);
		hDescriptor.Offset(1, mDXContext->mCbvSrvUavDescriptorSize);
	}

	//Load Skybox

	D3D12_RESOURCE_DESC resourceDesc = mSkybox.Resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC texSRVDesc = {};
	texSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	texSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	texSRVDesc.TextureCube.MostDetailedMip = 0;
	texSRVDesc.TextureCube.ResourceMinLODClamp = 0;
	texSRVDesc.TextureCube.MipLevels = resourceDesc.MipLevels;
	texSRVDesc.Format = resourceDesc.Format;
	mDXContext->mD3dDevice->CreateShaderResourceView(mSkybox.Resource.Get(), &texSRVDesc, hDescriptor);

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuStart = mTextureHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuStart = mTextureHeap->GetGPUDescriptorHandleForHeapStart();
	CD3DX12_CPU_DESCRIPTOR_HANDLE nullCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mNullCubemapIndex, mDXContext->mCbvSrvUavDescriptorSize);
	mNullGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, mNullCubemapIndex, mDXContext->mCbvSrvUavDescriptorSize);

	mDXContext->mD3dDevice->CreateShaderResourceView(nullptr, &texSRVDesc, nullCpuHandle);
	nullCpuHandle.Offset(1, mDXContext->mCbvSrvUavDescriptorSize);

	texSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	texSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texSRVDesc.Texture2D.MostDetailedMip = 0;
	texSRVDesc.Texture2D.MipLevels = 1;
	texSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	mDXContext->mD3dDevice->CreateShaderResourceView(nullptr, &texSRVDesc, nullCpuHandle);
}

TextureSystem::~TextureSystem()
{
}