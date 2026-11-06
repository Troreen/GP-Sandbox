#pragma once
#include <GraphicsEngine/Assets/MeshLibrary.h>
#include <GraphicsEngine/Scene/RenderCamera.h>
#include <GraphicsEngine/Scene/MeshInstance.h>
#include <GraphicsEngine/RenderSnapshot.h>
#include <memory>
#include <vector>

// Edit this class to set up an assignment. Objects are owned here, not by a framework.
class DemoScene
{
public:
	explicit DemoScene(const std::filesystem::path& contentRoot, const std::filesystem::path& shaderRoot);
	void Update(float deltaTime);
	RenderSnapshot GetSnapshot(RenderCamera& camera);
	bool Animate = true;
	bool ShowTranslucency = true;
	bool Shadows = true;

private:
	MeshInstance& AddMesh(const std::shared_ptr<Mesh>& mesh, const CU::Vector3f& position, const CU::Vector3f& scale,
	                      const std::shared_ptr<MaterialInterface>& material);
	void UpdateJoint(size_t index, const CU::Matrix4f& parent, const Animation::Frame& frame);
	MeshLibrary myMeshes;
	std::vector<std::unique_ptr<MeshInstance>> myInstances;
	DirectionalRenderLight mySun;
	PointRenderLight myPoint;
	SpotRenderLight mySpot;
	MeshInstance* myAnimatedMesh = nullptr;
	std::shared_ptr<Animation> myAnimation;
	float myTime = 0;
};
