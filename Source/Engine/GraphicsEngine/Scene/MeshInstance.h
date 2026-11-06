#pragma once
#include "RenderObject.h"
#include "GraphicsEngine/Objects/Mesh.h"
#include "GraphicsEngine/Materials/Material.h"
#include <array>
#include <memory>
#include <vector>

// The application owns instances; snapshots borrow them only for the current frame.
// Meshes/materials are shared GPU resources. Each instance owns its animation pose.
class MeshInstance : public RenderObject
{
public:
	explicit MeshInstance(std::shared_ptr<Mesh> mesh) : Geometry(std::move(mesh))
	{
		if (Geometry)
		{
			Materials.resize(Geometry->GetNumMaterialSlots());
		}
	}

	const std::array<CU::Matrix4f, 128>* GetJointTransforms() const
	{
		return &JointTransforms;
	}

	std::shared_ptr<Mesh> Geometry;
	std::vector<std::shared_ptr<MaterialInterface>> Materials;
	std::array<CU::Matrix4f, 128> JointTransforms{};
};
