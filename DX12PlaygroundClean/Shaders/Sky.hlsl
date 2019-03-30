
#include "Common.hlsl"


struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosL   : POSITION;

};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    InstanceData iData = gInstanceData[instanceID];
    
    VertexOut vout;
    vout.PosL = vin.PosL;

    float4 PosW = mul(float4(vin.PosL, 1.0f), iData.World);

    PosW.xyz += gEyePosW;

    vout.PosH = mul(PosW, gViewProj).xyww;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
}