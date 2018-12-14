#pragma once
#include "../Core/Default.h"
#include "DX12Context.h"
enum ShaderType
{
	PixelShader,
	VertexShader
};

struct Shader
{
	ShaderType Type;
	std::wstring Path;
	const D3D_SHADER_MACRO* macros;
	ComPtr<ID3DBlob> ShaderBlob;
};

class ShaderSystem
{
private:
	std::unordered_map<std::string, Shader> mShaders;
	DX12Context* mDXContext;
public:
	ShaderSystem(DX12Context* DXContext);
	void LoadShader(std::string name, std::wstring path, ShaderType type, const D3D_SHADER_MACRO* macro = nullptr);
	Shader& GetShader(std::string name);
	void ReloadShader(std::string name);
	std::pair<const char*, const char*> ResolveShaderType(ShaderType type);
	~ShaderSystem();
};
