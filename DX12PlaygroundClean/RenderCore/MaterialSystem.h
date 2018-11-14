#pragma once
#include "../Core/Default.h"
#include "DX12Context.h";
#include "TextureSystem.h"
#include "FrameResource.h"

class MaterialSystem
{
private:
	DX12Context* mDXContext;
	TextureSystem* mTextureSystem;

	std::vector<Material> mAllMaterials;
	std::unordered_map<std::string, u32> mMaterialIDs;
public:
	MaterialSystem(DX12Context* DXContext, TextureSystem* tSystem);
	MaterialID BuildMaterial(std::string name, std::string TextureName, MaterialConstants& options);
	MaterialID BuildMaterial(std::string name, TextureID textureID, MaterialConstants& options);

	u32 GetMaterialCount();
	Material& GetMaterial(std::string name);
	Material& GetMaterial(MaterialID id);
	MaterialID GetMaterialID(std::string name);
	void UpdateMaterials(UploadBuffer<MaterialConstants>* frameMaterial);

	~MaterialSystem();
};
