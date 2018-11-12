#pragma once
#include "../Core/Default.h"

class Waves
{
public:
	Waves(s32 m, s32 n, f32 dx, f32 dt, f32 speed, f32 damping);

	std::vector<XMFLOAT3> mPrevSolution;
	std::vector<XMFLOAT3> mCurrSolution;
	std::vector<XMFLOAT3> mNormals;
	std::vector<XMFLOAT3> mTagentX;

	s32 RowCount() const;
	s32 ColumnCount() const;
	s32 VertexCount() const;
	s32 TriangleCount() const;
	f32 Width() const;
	f32 Depth() const;

	void Update(f32 dt);
	void Disturb(s32 i, s32 j, f32 magnitude);

	~Waves();

private:
	s32 mNumRows = 0;
	s32 mNumCols = 0;

	s32 mVertexCount = 0;
	s32 mTriangleCount = 0;

	f32 mK1 = 0.0f;
	f32 mK2 = 0.0f;
	f32 mK3 = 0.0f;

	f32 mTimeStep = 0.0f;
	f32 mSpatialStep = 0.0f;
};
