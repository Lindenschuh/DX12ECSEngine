#include "ShaderSystem.h"
#include "DXHelpers.h"

ShaderSystem::ShaderSystem(DX12Context * DXContext)
{
	mDXContext = DXContext;
}

void ShaderSystem::LoadShader(std::string name, std::wstring path, ShaderType type)
{
	std::pair<const char*, const char*> typeOptions = ResolveShaderType(type);

	Shader shader = {};
	shader.Path = path;
	shader.Type = type;
	shader.ShaderBlob = CompileShader(path.c_str(), nullptr,
		typeOptions.first, typeOptions.second);
	mShaders[name] = shader;
}

Shader& ShaderSystem::GetShader(std::string name)
{
	return mShaders[name];
}

void ShaderSystem::ReloadShader(std::string name)
{
	mShaders[name].ShaderBlob->Release();
	LoadShader(name, mShaders[name].Path, mShaders[name].Type);
}

std::pair<const char*, const char*> ShaderSystem::ResolveShaderType(ShaderType type)
{
	switch (type)
	{
	case PixelShader:
		return std::pair<const char*, const char*>("PS", "ps_5_1");
		break;
	case VertexShader:
		return std::pair<const char*, const char*>("VS", "vs_5_1");
		break;
	default:
		return std::pair<const char*, const char*>();
	}
}

ShaderSystem::~ShaderSystem()
{
}