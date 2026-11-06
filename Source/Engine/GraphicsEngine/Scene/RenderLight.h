#pragma once

#include "RenderObject.h"

#include <CommonUtilities/CommonUtilities.h>
#include <GraphicsEngine/Objects/Texture.h>
#include <cstdint>

enum class LightType : uint32_t
{
	Directional = 0,
	Point = 1,
	Spot = 2,
};

// Application-owned light state. Shadow textures and projection are renderer resources.
class RenderLight : public RenderObject
{
public:
	virtual LightType GetLightType() const = 0;

	const Texture* GetShadowMap() const;

	const CU::Matrix4f& GetProjectionMatrix() const
	{
		return myProjectionMatrix;
	}

	const CU::Vector3f& GetColor() const
	{
		return myColor;
	}

	virtual float GetIntensity() const;

	float GetRadius() const
	{
		return myRadius;
	}

	float GetFalloffExponent() const
	{
		return myFalloffExponent;
	}

	float GetSourceColorAlpha() const
	{
		return mySourceColorAlpha;
	}

	float GetInnerCone() const
	{
		return myInnerCone;
	}

	float GetOuterCone() const
	{
		return myOuterCone;
	}

	bool CastShadows() const
	{
		return myCastShadows;
	}

	void SetColor(const CU::Vector3f& aColor)
	{
		myColor = aColor;
	}

	void SetShadowMapSize(const CU::Vector2u& aSize);

	void SetIntensity(float anIntensity);
	void SetRadius(float aRadius);

	void SetFalloffExponent(float value)
	{
		myFalloffExponent = value;
	}

	void SetSourceColorAlpha(float value)
	{
		mySourceColorAlpha = value;
	}

	void SetConeAnglesDegrees(float anInnerConeDegrees, float anOuterConeDegrees);

	void SetCastShadows(bool aCastShadows)
	{
		myCastShadows = aCastShadows;
	}

protected:
	virtual const CU::Matrix4f CreateProjectionMatrix() const;

	Texture myShadowMap;

	CU::Matrix4f myProjectionMatrix;
	CU::Vector3f myColor = CU::Vector3f::One;
	CU::Vector2u myShadowMapSize;

	float myIntensity = 1.0f;
	float myRadius = 1000.0f;
	float myFalloffExponent = 0.0f;
	float mySourceColorAlpha = 1.0f;
	float myInnerCone = 0.349066f;
	float myOuterCone = 0.610865f;

	bool myCastShadows = true;
};

class DirectionalRenderLight final : public RenderLight
{
public:
	static constexpr unsigned ShadowCascadeCount = 3;

	DirectionalRenderLight();

	LightType GetLightType() const override
	{
		return LightType::Directional;
	}

	float GetIntensity() const override;

	const Texture* GetShadowCascadeMap(size_t aIndex) const;
	void SetShadowCascadeMapSize(const CU::Vector2u& aSize, size_t aIndex);

protected:
	const CU::Matrix4f CreateProjectionMatrix() const override;

private:
	Texture myShadowCascadeMaps[ShadowCascadeCount - 1];

	CU::Vector2u myShadowCascadeMapSizes[ShadowCascadeCount - 1];
};

class PointRenderLight final : public RenderLight
{
public:
	PointRenderLight();

	LightType GetLightType() const override
	{
		return LightType::Point;
	}

protected:
	const CU::Matrix4f CreateProjectionMatrix() const override;
};

class SpotRenderLight final : public RenderLight
{
public:
	SpotRenderLight();

	LightType GetLightType() const override
	{
		return LightType::Spot;
	}

protected:
	const CU::Matrix4f CreateProjectionMatrix() const override;
};
