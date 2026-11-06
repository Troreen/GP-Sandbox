#include "DemoScene.h"
#include "PrimitiveMeshBuilder.h"
#include <GraphicsEngine/GraphicsEngine.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
	std::shared_ptr<MaterialInterface> LoadSceneMaterial(const std::filesystem::path& contentRoot, const std::filesystem::path& shaderRoot,
	                                                     const std::string& name)
	{
		MaterialDescription description;
		if (!LoadMaterialDescription(contentRoot / "Materials" / (name + ".mat"), description))
		{
			throw std::runtime_error("Cannot read reference material: " + name);
		}
		description.MaterialShaderCode = shaderRoot / description.MaterialShaderCode.filename();
		auto material = std::make_shared<Material>();
		if (!GraphicsEngine::Get().CreateMaterial(description, *material))
		{
			throw std::runtime_error("Cannot compile reference material: " + name);
		}
		return material;
	}

	void Aim(RenderObject& object, const CU::Vector3f& target)
	{
		const auto forward = (target - object.GetTransform().GetWorldPosition()).GetNormalized();
		object.GetTransform().SetLocalRotationDegrees(std::atan2(forward.x, forward.z) * CU::Math::RADIANS_TO_DEGREES<float>,
		                                              -std::asin(std::clamp(forward.y, -1.0f, 1.0f)) * CU::Math::RADIANS_TO_DEGREES<float>,
		                                              0);
	}
}

DemoScene::DemoScene(const std::filesystem::path& contentRoot, const std::filesystem::path& shaderRoot)
{
	myMeshes.Initialize(contentRoot);
	const auto floor = LoadSceneMaterial(contentRoot, shaderRoot, "FloorMaterial");
	const auto chest = LoadSceneMaterial(contentRoot, shaderRoot, "ChestMaterial");
	const auto alphaChest = LoadSceneMaterial(contentRoot, shaderRoot, "ChestMaterial_Alpha");
	const auto colorChecker = LoadSceneMaterial(contentRoot, shaderRoot, "ColorCheckerMaterial");
	const auto character = LoadSceneMaterial(contentRoot, shaderRoot, "CharacterMaterial");
	const auto chestMesh = myMeshes.LoadMesh("Meshes/Props/SM_Chest.fbx");

	// Match the active objects and authored transforms in d583fa1's LoadScene.
	// Its smooth-sphere position was declared but no sphere was instantiated.
	auto& floorInstance = AddMesh(PrimitiveMeshBuilder::CreateFloor(), {0, 0, 260}, {1100, 1100, 1100}, floor);
	floorInstance.GetTransform().SetLocalRotationDegrees({0, -90, 0});
	AddMesh(chestMesh, {135, 0, 285}, {1, 1, 1}, chest);
	auto& checkerInstance = AddMesh(myMeshes.LoadMesh("Meshes/Props/SM_Color_Checker.fbx"), {-145, 40, 365}, {1, 1, 1}, colorChecker);
	checkerInstance.GetTransform().SetLocalRotationDegrees({0, 0, -90});
	myAnimatedMesh = &AddMesh(myMeshes.LoadMesh("Meshes/Characters/TGA_Bro/SK_C_TGA_Bro.fbx"), {0, 0, 250}, {1, 1, 1}, character);
	myAnimatedMesh->GetTransform().SetLocalRotationDegrees({180, 0, 0});
	myAnimation = myMeshes.LoadFBXAnimation("Breathing", "Animations/Characters/TGA_Bro/Idle/A_C_TGA_Bro_Idle_Brething.fbx");
	if (!myAnimation || !myAnimation->IsValid() || !myAnimatedMesh->Geometry->HasSkeleton())
	{
		throw std::runtime_error("Cannot load the reference character and breathing animation");
	}

	// Keep translucent instances last for the existing panel's visibility toggle.
	for (const auto& [position, opacity] :
	     std::array{std::pair{CU::Vector3f{-200, 0, -100}, 0.35f}, std::pair{CU::Vector3f{0, 0, -100}, 0.75f}})
	{
		auto material = MaterialInstance::Create("ChestMaterial_Alpha_Instance", alphaChest);
		if (!material || !material->SetValue("MB_Tint", CU::Vector4f{1, 1, 1, opacity}))
		{
			throw std::runtime_error("Cannot configure reference chest opacity");
		}
		AddMesh(chestMesh, position, {1, 1, 1}, material);
	}

	// Match the original lighting values; local lights retain the same 10,000 unit conversion.
	const CU::Vector3f sceneFocus{25, 0, 260};
	mySun.SetColor({1, 0.96f, 0.9f});
	mySun.SetIntensity(10);
	mySun.GetTransform().SetLocalPosition({-450, 650, -350});
	Aim(mySun, sceneFocus);
	for (size_t cascade = 0; cascade < DirectionalRenderLight::ShadowCascadeCount; ++cascade)
	{
		mySun.SetShadowCascadeMapSize({2048, 2048}, cascade);
	}
	myPoint.SetColor({1, 0.42f, 0.22f});
	myPoint.SetIntensity(20);
	myPoint.SetRadius(760);
	myPoint.GetTransform().SetLocalPosition({-90, 180, 150});
	myPoint.SetShadowMapSize({2048, 2048});
	mySpot.SetColor({0.55f, 0.7f, 1});
	mySpot.SetIntensity(30);
	mySpot.SetRadius(1200);
	mySpot.SetConeAnglesDegrees(18, 34);
	mySpot.GetTransform().SetLocalPosition({430, 430, -210});
	Aim(mySpot, sceneFocus);
	mySpot.SetShadowMapSize({2048, 2048});
	if (!GraphicsEngine::Get().SetEnvironmentMap(contentRoot / "Textures/T_Shipyard.dds"))
	{
		throw std::runtime_error("Cannot load the reference environment lighting");
	}
	Update(0);
}

MeshInstance& DemoScene::AddMesh(const std::shared_ptr<Mesh>& mesh, const CU::Vector3f& position, const CU::Vector3f& scale,
                                 const std::shared_ptr<MaterialInterface>& material)
{
	if (!mesh)
	{
		throw std::runtime_error("Cannot load reference scene mesh");
	}
	auto instance = std::make_unique<MeshInstance>(mesh);
	instance->GetTransform().SetLocalPosition(position);
	instance->GetTransform().SetLocalScale(scale);
	std::fill(instance->Materials.begin(), instance->Materials.end(), material);
	myInstances.push_back(std::move(instance));
	return *myInstances.back();
}

void DemoScene::Update(float deltaTime)
{
	if (Animate)
	{
		myTime += deltaTime;
	}
	mySun.SetCastShadows(Shadows);
	myPoint.SetCastShadows(Shadows);
	mySpot.SetCastShadows(Shadows);
	const size_t frameIndex = static_cast<size_t>(myTime * myAnimation->FramesPerSecond) % myAnimation->Frames.size();
	const Skeleton& skeleton = *myAnimatedMesh->Geometry->GetSkeleton();
	for (size_t index = 0; index < skeleton.Joints.size(); ++index)
	{
		if (skeleton.Joints[index].Parent < 0)
		{
			UpdateJoint(index, CU::Matrix4f::Identity, myAnimation->Frames[frameIndex]);
		}
	}
}

void DemoScene::UpdateJoint(size_t index, const CU::Matrix4f& parent, const Animation::Frame& frame)
{
	const Skeleton& skeleton = *myAnimatedMesh->Geometry->GetSkeleton();
	if (index >= skeleton.Joints.size() || index >= myAnimatedMesh->JointTransforms.size())
	{
		return;
	}
	const auto& joint = skeleton.Joints[index];
	const auto found = frame.Transforms.find(joint.Name);
	const CU::Matrix4f local = found != frame.Transforms.end() ? found->second : CU::Matrix4f::Identity;
	const CU::Matrix4f world = local * parent;
	myAnimatedMesh->JointTransforms[index] = joint.BindPoseInverse * world;
	for (unsigned child : joint.Children)
	{
		UpdateJoint(child, world, frame);
	}
}

RenderSnapshot DemoScene::GetSnapshot(RenderCamera& camera)
{
	RenderSnapshot snapshot;
	snapshot.ActiveCamera = &camera;
	snapshot.TotalTime = myTime;
	for (size_t index = 0; index < myInstances.size(); ++index)
	{
		if (ShowTranslucency || index < myInstances.size() - 2)
		{
			snapshot.Meshes.push_back(myInstances[index].get());
		}
	}
	// Lights are borrowed for immediate rendering, just like the instance pointers.
	snapshot.Lights = {&mySun, &myPoint, &mySpot};
	return snapshot;
}
