#pragma once
#include "Transform.h"

// Render objects own their transform directly; there is no world or actor owner.
class RenderObject
{
public:
	virtual ~RenderObject() = default;

	Transform& GetTransform()
	{
		return myTransform;
	}

	const Transform& GetTransform() const
	{
		return myTransform;
	}

private:
	Transform myTransform;
};
