#pragma once

#include "Default.h"

inline u32 CalcConstantBufferByteSize(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, u32 elementCount, bool isConstant)
	{
		elementByteSize = sizeof(T);

		if (isConstant)
			elementByteSize = CalcConstantBufferByteSize(sizeof(T));

		HR(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uBuffer)
		));

		HR(uBuffer->Map(0, nullptr, (void**)&mappedData))
	}

	ID3D12Resource* Resource()const
	{
		return uBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&mappedData[elementIndex*elementByteSize], &data, sizeof(T));
	}

	~UploadBuffer()
	{
		if (uBuffer)
			uBuffer->Unmap(0, nullptr);

		mappedData = nullptr;
	}
private:
	ComPtr<ID3D12Resource> uBuffer;
	BYTE* mappedData;
	u32 elementByteSize;
};

ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device * device,
	ID3D12GraphicsCommandList * cmdList, const void * initData, u64 byteSize,
	ComPtr<ID3D12Resource>& uploadBuffer);

ComPtr<ID3DBlob> CompileShader(
	const char* filename,
	const D3D_SHADER_MACRO* defines,
	const char* entryPoint,
	const char* target);
