#pragma once

#include "Apex/Game.h"
#include "Containers/AxArray.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry/Mesh.h"
#include "Math/Vector3.h"

enum class IKSolverType
{
	CCD,
	FABRIK,
	PsuedoInverse,
	JacobianTranspose,
	JacobianDampedLeastSquares,
	TikhonovRegularization = JacobianDampedLeastSquares,
};

struct IKChain
{
	IKChain() = default;
	IKChain(size_t length);

	void reserve(size_t length);
	void computeBfsOrder();
	void updateLocalTransforms();
	void updateGlobalTransforms();
	void solveIK(IKSolverType solver);

	size_t length() const { return translations.size(); }
	apex::AxArray<apex::math::Vector3> translations;
	apex::AxArray<apex::math::Vector3> eulerAngles;
	apex::AxArray<apex::u32> parents;
	apex::AxArray<apex::u32> bfsOrder;
	apex::AxArray<apex::u32> bfsIndex;
	apex::AxArray<bool> isEndEffector;
	apex::AxArray<apex::u32> endEffectors;
	apex::AxArray<apex::math::Vector3> endEffectorTargets;
	apex::AxArray<apex::math::Matrix4x4> jointLocalTransforms; // with respect to parent
	apex::AxArray<apex::math::Matrix4x4> jointGlobalTransforms; // with respect to origin
};

struct IKChainBuilderJoint
{
	apex::math::Vector3 translation;
	apex::math::Vector3 eulerAngles;
	apex::u32 parent;
	bool isEndEffector;
};

struct IKChainBuilder
{
	apex::AxArray<IKChainBuilderJoint> joints;

	void addJoint(apex::math::Vector3 translation, apex::math::Vector3 euler_angles, apex::u32 parent_index, bool is_end_effector = false);
	auto build() -> IKChain;
	auto build(IKChain& chain) -> void;
	auto reserve(size_t length) -> void;
};

class IKTrial : public apex::Game
{
public:
	IKTrial() = default;

	void initialize() override;
	void update(float deltaTimeMs) override;
	void stop() override;

	void initIK();
	void updateIK(float dt);

private:
	IKChain m_ikchain{};
	apex::gfx::Camera m_camera;
	apex::gfx::StaticMesh meshes[2];
	apex::math::Matrix4x4 m_cameraTransform;
};
