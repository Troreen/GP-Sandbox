#pragma once
#include <Windows.h>
#include <GraphicsEngine/Scene/RenderCamera.h>

class FlyCamera
{
public:
	FlyCamera();
	~FlyCamera();
	void Update(HWND window, float deltaTime, bool keyboardCaptured, bool mouseCaptured);
	void Reset();
	RenderCamera Camera;
	float Speed = 500;

private:
	void StopLooking();
	bool myLooking = false;
	POINT myPreviousCursor{};
};
