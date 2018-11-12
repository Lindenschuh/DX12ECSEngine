#include "Waves.h"

Waves::Waves(s32 m, s32 n, f32 dx, f32 dt, f32 speed, f32 damping)
{
	mNumRows = m;
	mNumCols = n;

	mVertexCount = m * n;
	mTriangleCount = (m - 1)*(n - 1) * 2;

	mTimeStep = dt;
	mSpatialStep = dx;

	f32 d = damping * dt + 2.0f;
	f32 e = (speed * speed)*(dt*dt) / (dx*dx);
	mK1 = (damping * dt - 2.0f) / d;
	mK2 = (4.0f - 8.0f*e) / d;
	mK3 = (2.0f*e) / d;

	mPrevSolution.resize(m*n);
	mCurrSolution.resize(m*n);
	mNormals.resize(m*n);
	mTagentX.resize(m*n);

	float halfWidth = (n - 1)*dx*0.5f;
	float halfDepth = (m - 1)*dx*0.5f;
	for (s32 i = 0; i < m; i++)
	{
		float z = halfDepth - i * dx;
		for (s32 j = 0; j < n; j++)
		{
			float x = -halfWidth + j * dx;

			mPrevSolution[i*n + j] = { x,0.0f,z };
			mCurrSolution[i*n + j] = { x,0.0f,z };
			mNormals[i*n + j] = { 0.0f,1.0f,0.0f };
			mTagentX[i*n + j] = { 1.0f,0.0f,0.0f };
		}
	}
}

s32 Waves::RowCount() const
{
	return mNumRows;
}

s32 Waves::ColumnCount() const
{
	return mNumCols;
}

s32 Waves::VertexCount() const
{
	return mVertexCount;
}

s32 Waves::TriangleCount() const
{
	return mTriangleCount;
}

f32 Waves::Width() const
{
	return mNumCols * mSpatialStep;
}

f32 Waves::Depth() const
{
	return mNumRows * mSpatialStep;
}

void Waves::Update(f32 dt)
{
	static float t = 0;

	t += dt;
	if (t >= mTimeStep)
	{
		for (s32 i = 1; i < mNumRows - 1; i++)
		{
			for (s32 j = 1; j < mNumCols - 1; j++)
			{
				mPrevSolution[i*mNumCols + j].y =
					mK1 * mPrevSolution[i*mNumCols + j].y +
					mK2 * mCurrSolution[i*mNumCols + j].y +
					mK3 * (mCurrSolution[(i + 1)*mNumCols + j].y +
						mCurrSolution[(i - 1)*mNumCols + j].y +
						mCurrSolution[i*mNumCols + j + 1].y +
						mCurrSolution[i*mNumCols + j - 1].y);
			}
		}

		std::swap(mPrevSolution, mCurrSolution);

		t = 0.0f;

		for (s32 i = 1; i < mNumRows - 1; i++)
		{
			for (s32 j = 1; j < mNumCols - 1; j++)
			{
				float l = mCurrSolution[i*mNumCols + j - 1].y;
				float r = mCurrSolution[i*mNumCols + j + 1].y;
				float t = mCurrSolution[(i - 1)*mNumCols + j].y;
				float b = mCurrSolution[(i + 1)*mNumCols + j].y;
				mNormals[i*mNumCols + j].x = -r + l;
				mNormals[i*mNumCols + j].y = 2.0f*mSpatialStep;
				mNormals[i*mNumCols + j].z = b - t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mNormals[i*mNumCols + j]));
				XMStoreFloat3(&mNormals[i*mNumCols + j], n);

				mTagentX[i*mNumCols + j] = XMFLOAT3(2.0f*mSpatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&mTagentX[i*mNumCols + j]));
				XMStoreFloat3(&mTagentX[i*mNumCols + j], T);
			}
		}
	}
}

void Waves::Disturb(s32 i, s32 j, f32 magnitude)
{
	float halfMag = 0.5f * magnitude;

	mCurrSolution[i*mNumCols + j].y += magnitude;
	mCurrSolution[i*mNumCols + j + 1].y += halfMag;
	mCurrSolution[i*mNumCols + j - 1].y += halfMag;
	mCurrSolution[(i + 1)*mNumCols + j].y += halfMag;
	mCurrSolution[(i - 1)*mNumCols + j].y += halfMag;
}

Waves::~Waves()
{
}