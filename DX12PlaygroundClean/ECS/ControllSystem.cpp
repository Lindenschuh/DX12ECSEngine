#include "ControllSystem.h"

ControllSystem::ControllSystem(EntityManager * eManager, PositionSystem * pSystem)
{
	mEManager = eManager;
	mPositionSystem = pSystem;
}

void ControllSystem::AddToSystem(EntityID id)
{
	mEntities.emplace_back(id);
}

void ControllSystem::RemoveFromSystem(EntityID id)
{
	for (int i = 0; i < mEntities.size(); i++)
	{
		if (mEntities[i] == id)
		{
			mEntities.erase(mEntities.begin() + i);
		}
	}
}

void ControllSystem::UpdateSystem(float time, float deltaTime)
{
	for (int i = 0; i < mEntities.size(); i++)
	{
		EntityID eId = mEntities[i];

		if (ImGui::IsKeyDown(VK_SHIFT))
		{
			speed = 50.0f;
		}
		else
		{
			speed = 10.0f;
		}

		if (ImGui::IsKeyDown('W'))
		{
			mPositionSystem->Walk(eId, speed * deltaTime);
		}

		if (ImGui::IsKeyDown('S'))
		{
			mPositionSystem->Walk(eId, (-speed) * deltaTime);
		}

		if (ImGui::IsKeyDown('A'))
		{
			mPositionSystem->Strafe(eId, (-speed) * deltaTime);
		}

		if (ImGui::IsKeyDown('D'))
		{
			mPositionSystem->Strafe(eId, speed* deltaTime);
		}

		if (ImGui::IsMouseDown(0))
		{
			float dx = XMConvertToRadians(0.25f*(
				(float)ImGui::GetMousePos().x - mLastMouseX));

			float dy = XMConvertToRadians(0.25f*(
				(float)ImGui::GetMousePos().y - mLastMouseY));

			mPositionSystem->RotateY(eId, dx);
			mPositionSystem->Pitch(eId, dy);
		}
	}

	mLastMouseX = ImGui::GetMousePos().x;
	mLastMouseY = ImGui::GetMousePos().y;
}