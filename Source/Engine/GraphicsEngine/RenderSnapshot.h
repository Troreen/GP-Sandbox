#pragma once

#include "Drawers/SpriteDrawer.h"
#include "TextWidget.h"

#include <GraphicsEngine/Scene/MeshInstance.h>
#include <GraphicsEngine/Scene/RenderCamera.h>
#include <GraphicsEngine/Scene/RenderLight.h>

struct RenderSnapshot
{
	// Elapsed scene simulation seconds, shared by camera and shadow material draws.
	float TotalTime = 0.0f;
	RenderCamera* ActiveCamera = nullptr;

	std::vector<MeshInstance*> Meshes;
	std::vector<RenderLight*> Lights;
	std::vector<std::shared_ptr<TextWidget>> ScreenTextItems;
	std::vector<SpriteDrawCall> ScreenSpriteItems;

	void Clear();
};
