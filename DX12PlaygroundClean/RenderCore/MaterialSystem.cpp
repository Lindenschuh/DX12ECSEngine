#include "MaterialSystem.h"

MaterialSystem::MaterialSystem(DX12Context * DXContext, TextureSystem * tSystem)
{
	mDXContext = DXContext;
	mTextureSystem = tSystem;
}

MaterialID MaterialSystem::BuildMaterial(std::string & name,
	std::string & TextureName, MaterialConstants & options)
{
	return BuildMaterial(name, mTextureSystem->GetTextureID(TextureName), options);
}

MaterialID MaterialSystem::BuildMaterial(std::string & name, TextureID textureID, MaterialConstants & options)
{
	Material material;
	material.MatCBIndex = mAllMaterials.size();
	material.DiffuseSrvHeapIndex = textureID;
	material.DiffuseAlbedo = options.DiffuseAlbedo;
	material.FresnelR0 = options.FresnelR0;
	material.Roughness = options.Roughness;
	material.MatTransform = options.MatTransform;
	mAllMaterials.push_back(material);
	mMaterialIDs[name] = material.MatCBIndex;

	return material.MatCBIndex;
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

void MaterialSystem::UpdateMaterials(UploadBuffer<MaterialConstants>* materialBuffer)
{
	for (int i = 0; i < mAllMaterials.size(); i++)
	{
		Material& mat = mAllMaterials[i];

		if (mat.NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat.MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat.DiffuseAlbedo;
			matConst.FresnelR0 = mat.FresnelR0;
			matConst.Roughness = mat.Roughness;
			XMStoreFloat4x4(&matConst.MatTransform, XMMatrixTranspose(matTransform));
			materialBuffer->CopyData(mat.MatCBIndex, matConst);

			mat.NumFramesDirty--;
		}
	}
}