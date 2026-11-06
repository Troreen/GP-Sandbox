#include "GraphicsEngine.pch.h"
#include "RenderLight.h"

#include <algorithm>
#include <CommonUtilities/Maths.hpp>
#include <d3d11_1.h>
#include <GraphicsEngine/GraphicsEngine.h>

namespace
{
	constexpr float MaximumSpotConeDegrees = 89.0f;
}

const Texture* RenderLight::GetShadowMap() const
{
	if (myShadowMapSize.x == 0 || myShadowMapSize.y == 0)
	{
		return nullptr;
	}

	return &myShadowMap;
}

float RenderLight::GetIntensity() const
{
	return myIntensity * 10000;
}

void RenderLight::SetShadowMapSize(const CU::Vector2u& aSize)
{
	if (myShadowMapSize.x == aSize.x && myShadowMapSize.y == aSize.y)
	{
		return;
	}

	myShadowMapSize = aSize;
	if (myShadowMapSize.x == 0 || myShadowMapSize.y == 0)
	{
		myShadowMap = {};
		return;
	}

	std::string shadowMapName = "ShadowMap_SM";
	ensure(GraphicsEngine::Get().CreateShadowMap(shadowMapName, myShadowMapSize.x, myShadowMapSize.y, myShadowMap,
	                                             GetLightType() == LightType::Point));
	myProjectionMatrix = CreateProjectionMatrix();
}

void RenderLight::SetIntensity(float anIntensity)
{
	myIntensity = (std::max)(0.0f, anIntensity);
}

void RenderLight::SetRadius(float aRadius)
{
	myRadius = (std::max)(1.01f, aRadius);
	if (myShadowMapSize.x != 0 && myShadowMapSize.y != 0)
	{
		myProjectionMatrix = CreateProjectionMatrix();
	}
}

void RenderLight::SetConeAnglesDegrees(float anInnerConeDegrees, float anOuterConeDegrees)
{
	const float innerDegrees = CU::Clamp(anInnerConeDegrees, 0.0f, MaximumSpotConeDegrees);
	const float outerDegrees = CU::Clamp(anOuterConeDegrees, innerDegrees, MaximumSpotConeDegrees);
	myInnerCone = CU::Maths::DegreesToRadians(innerDegrees);
	myOuterCone = CU::Maths::DegreesToRadians((std::max)(0.1f, outerDegrees));
	if (myShadowMapSize.x != 0 && myShadowMapSize.y != 0)
	{
		myProjectionMatrix = CreateProjectionMatrix();
	}
}

const CU::Matrix4f RenderLight::CreateProjectionMatrix() const
{
	return CU::Matrix4f::Identity;
}

// ======================
// Directional Light
// ======================

DirectionalRenderLight::DirectionalRenderLight()
{
	SetIntensity(10.0f);
}

float DirectionalRenderLight::GetIntensity() const
{
	return myIntensity;
}

const Texture* DirectionalRenderLight::GetShadowCascadeMap(size_t aIndex) const
{
	if (aIndex >= ShadowCascadeCount)
	{
		return nullptr;
	}
	const CU::Vector2u* shadowMapSize = aIndex > 0 ? &myShadowCascadeMapSizes[aIndex - 1] : &myShadowMapSize;
	if (shadowMapSize->x == 0 || shadowMapSize->y == 0)
	{
		return nullptr;
	}

	const Texture* shadowMap = aIndex > 0 ? &myShadowCascadeMaps[aIndex - 1] : &myShadowMap;
	return shadowMap;
}

void DirectionalRenderLight::SetShadowCascadeMapSize(const CU::Vector2u& aSize, size_t aIndex)
{
	if (aIndex >= ShadowCascadeCount)
	{
		return;
	}
	CU::Vector2u* shadowMapSize = aIndex > 0 ? &myShadowCascadeMapSizes[aIndex - 1] : &myShadowMapSize;
	if (shadowMapSize->x == aSize.x && shadowMapSize->y == aSize.y)
	{
		return;
	}

	*shadowMapSize = aSize;
	if (shadowMapSize->x == 0 || shadowMapSize->y == 0)
	{
		Texture* shadowMap = aIndex > 0 ? &myShadowCascadeMaps[aIndex - 1] : &myShadowMap;
		*shadowMap = {};
		return;
	}

	const std::string shadowMapName = std::format("Cascade {}_ShadowMap", std::to_string(aIndex));
	Texture* shadowMap = aIndex > 0 ? &myShadowCascadeMaps[aIndex - 1] : &myShadowMap;
	ensure(GraphicsEngine::Get().CreateShadowMap(shadowMapName, shadowMapSize->x, shadowMapSize->y, *shadowMap, false));
}

const CU::Matrix4f DirectionalRenderLight::CreateProjectionMatrix() const
{
	return CU::Matrix4f::CreateOrthographicProjection(static_cast<float>(myShadowMapSize.x), static_cast<float>(myShadowMapSize.y), 1.0f,
	                                                  20000.0f);
}

// ======================
// Point Light
// ======================

PointRenderLight::PointRenderLight()
{
	SetIntensity(800.0f);
	SetRadius(800.0f);
}

const CU::Matrix4f PointRenderLight::CreateProjectionMatrix() const
{
	return CU::Matrix4f::CreatePerspectiveProjection(static_cast<float>(myShadowMapSize.x), static_cast<float>(myShadowMapSize.y),
	                                                 90 * CommonUtilities::Math::DEGREES_TO_RADIANS<float>, 1.0f, myRadius);
}

// ======================
// Spot Light
// ======================

SpotRenderLight::SpotRenderLight()
{
	SetIntensity(1200.0f);
	SetRadius(1000.0f);
	SetConeAnglesDegrees(20.0f, 35.0f);
}

const CU::Matrix4f SpotRenderLight::CreateProjectionMatrix() const
{
	return CU::Matrix4f::CreatePerspectiveProjection(static_cast<float>(myShadowMapSize.x), static_cast<float>(myShadowMapSize.y),
	                                                 myOuterCone * 2, 1.0f, myRadius);
}
