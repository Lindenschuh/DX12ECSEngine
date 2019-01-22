#include "MaterialSystem.h"

MaterialSystem::MaterialSystem(DX12Context * DXContext, TextureSystem * tSystem)
{
	mDXContext = DXContext;
	mTextureSystem = tSystem;
}

MaterialID MaterialSystem::BuildMaterial(std::string  name,
	std::string diffuseMapName, std::string normalMapName, MaterialConstants & options)
{
	return BuildMaterial(name, mTextureSystem->GetTextureID(diffuseMapName),
		mTextureSystem->GetTextureID(normalMapName), options);
}

MaterialID MaterialSystem::BuildMaterial(std::string name, TextureID diffuseMapID, TextureID normalMapID, MaterialConstants & options)
{
	Material material;
	material.MatCBIndex = mAllMaterials.size();
	material.DiffuseSrvHeapIndex = diffuseMapID;
	material.NormalSrvHeapIndex = normalMapID;
	material.DiffuseAlbedo = options.DiffuseAlbedo;
	material.FresnelR0 = options.FresnelR0;
	material.Roughness = options.Roughness;
	material.MatTransform = options.MatTransform;
	mAllMaterials.push_back(material);
	mMaterialIDs[name] = material.MatCBIndex;

	return material.MatCBIndex;
}

u32 MaterialSystem::GetMaterialCount()
{
	return mAllMaterials.size();
}

Material& MaterialSystem::GetMaterial(std::string name)
{
	return mAllMaterials[mMaterialIDs[name]];
}

Material& MaterialSystem::GetMaterial(MaterialID id)
{
	return mAllMaterials[id];
}

MaterialID MaterialSystem::GetMaterialID(std::string name)
{
	return mMaterialIDs[name];
}

void MaterialSystem::UpdateMaterials(UploadBuffer<MaterialData>* materialBuffer)
{
	for (int i = 0; i < mAllMaterials.size(); i++)
	{
		Material& mat = mAllMaterials[i];

		if (mat.NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat.MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat.DiffuseAlbedo;
			matData.FresnelR0 = mat.FresnelR0;
			matData.Roughness = mat.Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat.DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat.NormalSrvHeapIndex;
			materialBuffer->CopyData(mat.MatCBIndex, matData);

			mat.NumFramesDirty--;
		}
	}
}