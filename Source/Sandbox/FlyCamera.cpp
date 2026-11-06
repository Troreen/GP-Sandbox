#include "FlyCamera.h"
#include <algorithm>
#include <cmath>

namespace
{
	bool Down(int key)
	{
		return (GetAsyncKeyState(key) & 0x8000) != 0;
	}
}

FlyCamera::FlyCamera()
{
	Reset();
}

FlyCamera::~FlyCamera()
{
	StopLooking();
}

void FlyCamera::StopLooking()
{
	if (myLooking)
	{
		ClipCursor(nullptr);
		ReleaseCapture();
		ShowCursor(TRUE);
		SetCursorPos(myPreviousCursor.x, myPreviousCursor.y);
		myLooking = false;
	}
}

void FlyCamera::Reset()
{
	const CU::Vector3f position{0, 260, -950};
	const CU::Vector3f forward = (CU::Vector3f{25, 0, 260} - position).GetNormalized();
	Camera.GetTransform().SetLocalPosition(position);
	Camera.GetTransform().SetLocalRotationDegrees({std::atan2(forward.x, forward.z) * CU::Math::RADIANS_TO_DEGREES<float>,
	                                               -std::asin(forward.y) * CU::Math::RADIANS_TO_DEGREES<float>, 0});
}

void FlyCamera::Update(HWND window, float deltaTime, bool keyboardCaptured, bool mouseCaptured)
{
	if (GetForegroundWindow() != window)
	{
		StopLooking();
		return;
	}
	auto& transform = Camera.GetTransform();
	if (!keyboardCaptured)
	{
		CU::Vector3f movement;
		if (Down('W'))
		{
			movement += transform.GetLocalForward();
		}
		if (Down('S'))
		{
			movement -= transform.GetLocalForward();
		}
		if (Down('D'))
		{
			movement += transform.GetLocalRight();
		}
		if (Down('A'))
		{
			movement -= transform.GetLocalRight();
		}
		if (Down(VK_SPACE))
		{
			movement += CU::Vector3f::UnitY;
		}
		if (Down(VK_CONTROL))
		{
			movement -= CU::Vector3f::UnitY;
		}
		if (movement.LengthSqr() > 0)
		{
			const float speed = Speed * (Down(VK_SHIFT) ? 3.0f : 1.0f);
			transform.SetLocalPosition(transform.GetLocalPosition() + movement.GetNormalized() * speed * deltaTime);
		}
	}
	// Recenter while looking so rotation continues beyond the desktop edges.
	POINT cursor;
	GetCursorPos(&cursor);
	if (Down(VK_RBUTTON) && (!mouseCaptured || myLooking))
	{
		RECT client;
		GetClientRect(window, &client);
		POINT topLeft{client.left, client.top};
		POINT bottomRight{client.right, client.bottom};
		ClientToScreen(window, &topLeft);
		ClientToScreen(window, &bottomRight);
		RECT clip{topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
		const POINT center{(clip.left + clip.right) / 2, (clip.top + clip.bottom) / 2};
		if (myLooking)
		{
			auto rotation = transform.GetLocalRotationDegrees();
			rotation.x += static_cast<float>(cursor.x - center.x) * 0.15f;
			rotation.y = std::clamp(rotation.y + static_cast<float>(cursor.y - center.y) * 0.15f, -89.0f, 89.0f);
			rotation.z = 0;
			transform.SetLocalRotationDegrees(rotation);
		}
		else
		{
			myPreviousCursor = cursor;
			SetCapture(window);
			ShowCursor(FALSE);
			myLooking = true;
		}
		ClipCursor(&clip);
		SetCursorPos(center.x, center.y);
	}
	else
	{
		StopLooking();
	}
}
