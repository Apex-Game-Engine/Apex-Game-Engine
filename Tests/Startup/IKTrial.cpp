#include "InverseKinematics.h"

#include <deque>

#include "Apex/Application.h"
#include "Apex/Window.h"
#include "Containers/AxList.h"
#include "Containers/AxRange.h"
#include "Graphics/ForwardRenderer.h"
#include "Graphics/Primitives/Cube.h"
#include "Graphics/Primitives/Pyramid.h"

namespace math = apex::math;

namespace detail {

	void solveIK_CCD(IKChain& chain, apex::uint32 max_iterations);
	void solveIK_FABRIK(IKChain& chain, apex::uint32 max_iterations);

}

IKChain::IKChain(size_t length)
{
	reserve(length);
}

void IKChain::reserve(size_t length)
{
	translations.reserve(length);
	eulerAngles.reserve(length);
	parents.reserve(length);
	bfsOrder.reserve(length);
	bfsIndex.reserve(length);
	isEndEffector.reserve(length);
	endEffectors.reserve(length);
	endEffectorTargets.reserve(length);
	jointLocalTransforms.reserve(length);
	jointGlobalTransforms.reserve(length);
}

void IKChain::computeBfsOrder()
{
	//bfsOrder.reserve(parents.size());
	//bfsIndex.reserve(parents.size());

	apex::AxArray<apex::uint32> visited;
	visited.resize(parents.size(), 0);

	// TODO: make own deque class using AxArray
	std::deque<apex::uint32> queue;

	queue.push_back(0);
	visited[0] = 1;

	while (!queue.empty())
	{
		apex::uint32 current = queue.front();
		queue.pop_front();
		bfsOrder.append(current);

		for (apex::uint32 i = 0; i < parents.size(); ++i)
		{
			if (visited[i] == 0 && parents[i] == current)
			{
				queue.push_back(i);
				visited[i] = 1;
			}
		}
	}

	for (apex::uint32 i = 0; i < bfsOrder.size(); ++i)
	{
		bfsIndex.append(i);
	}
}

void IKChain::updateLocalTransforms()
{
	jointLocalTransforms.clear();

	for (size_t i = 0; i < translations.size(); ++i)
	{
		math::Matrix4x4 localTransform = math::eulerZYX(eulerAngles[i].x, eulerAngles[i].y, eulerAngles[i].z);
		localTransform = math::translate(localTransform, translations[i]);

		jointLocalTransforms.append(localTransform);
	}
}

void IKChain::updateGlobalTransforms()
{
	jointGlobalTransforms.clear();

	for (auto idx : bfsOrder)
	{
		if (parents[idx] == idx)
		{
			jointGlobalTransforms.append(jointLocalTransforms[idx]);
		}
		else
		{
			jointGlobalTransforms.append(jointGlobalTransforms[parents[idx]] * jointLocalTransforms[idx]);
		}
	}
}

void IKChain::solveIK(IKSolverType solver)
{
	switch (solver)
	{
	case IKSolverType::CCD:
		detail::solveIK_CCD(*this, 25);
		break;
	case IKSolverType::FABRIK:
		detail::solveIK_FABRIK(*this, 25);
		break;
	case IKSolverType::PsuedoInverse:
	case IKSolverType::JacobianTranspose:
	case IKSolverType::JacobianDampedLeastSquares:
	default:
		TODO("Only CCD solver implemented yet!");
		break;
	}
}

void IKChainBuilder::addJoint(apex::math::Vector3 translation, apex::math::Vector3 euler_angles, apex::uint32 parent_index, bool is_end_effector)
{
	joints.emplace_back(translation, euler_angles, parent_index, is_end_effector);
}

IKChain IKChainBuilder::build()
{
	IKChain chain;
	build(chain);
	return chain;
}

void IKChainBuilder::build(IKChain& chain)
{
	chain.reserve(joints.size());

	for (apex::uint32 i = 0; i < joints.size(); ++i)
	{
		chain.translations.append(joints[i].translation);
		chain.eulerAngles.append(joints[i].eulerAngles);
		chain.parents.append(joints[i].parent);
		chain.isEndEffector.append(joints[i].isEndEffector);
		if (joints[i].isEndEffector)
			chain.endEffectors.append(i);
	}

	chain.computeBfsOrder();
	chain.updateLocalTransforms();
	chain.updateGlobalTransforms();

	for (apex::uint32 i = 0; i < chain.endEffectors.size(); i++)
	{
		chain.endEffectorTargets.append(chain.jointGlobalTransforms[chain.endEffectors[i]].getTranslation());
	}
}

auto IKChainBuilder::reserve(size_t length) -> void
{
	joints.reserve(length);
}

void IKTrial::initialize()
{
	auto& renderer = *apex::Application::Instance()->getRenderer();

	renderer.setActiveCamera(&m_camera);
	m_cameraTransform = math::translate(math::Matrix4x4::identity(), { 0, 2, 10 });

	auto pyramidMeshCpu = apex::gfx::Pyramid::getMesh();
	meshes[0].create(renderer.getContext().m_device, &pyramidMeshCpu, nullptr);

	auto cubeMeshCpu = apex::gfx::Cube::getMesh();
	meshes[1].create(renderer.getContext().m_device, &cubeMeshCpu, nullptr);

	auto& commandList = renderer.getCurrentCommandList();
	commandList.getCommands().reserve(100);

	initIK();
}

void IKTrial::update(float deltaTimeMs)
{
	// Update
	static auto startTime = std::chrono::high_resolution_clock::now();
	static auto prevTime = startTime;
	const auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
	prevTime = currentTime;

	// Update camera
	//math::Vector3 cameraPos = math::Vector3{ 4 * sinf(time * math::radians(30.f)), 2 + 4 * cosf(time * math::radians(30.f)), 7 };
	//m_cameraTransform = math::translate(math::Matrix4x4::identity(), cameraPos);

	// m_camera.view = math::inverse(m_cameraTransform);
	m_camera.view = math::lookAt(m_cameraTransform.getTranslation(), { 0, 2, 0 }, math::Vector3::unitY());

	int width, height;
	apex::Application::Instance()->getWindow()->getFramebufferSize(width, height);
	apex::float32 aspect = static_cast<apex::float32>(width) / static_cast<apex::float32>(height);
	apex::float32 fov = math::radians(60.f);
	m_camera.projection = math::perspective(fov, aspect, 0.1f, 1000.f);
	m_camera.projection[1][1] *= -1;

	// Update IK
	m_ikchain.endEffectorTargets[0] = math::Vector3{ -3.f + 1.5f * sinf(2.f * time), 0.5f * cosf(time), 1.f * cosf(2.f * time) };
	//m_ikchain.endEffectorTargets[0] = math::Vector3{ -2.f, 0, 0 };

	static float t = 0;
	t += deltaTime;
	//if (t > 1.f)
	{
		t = 0;
		updateIK(deltaTime);
	}

	// Render
	auto app = apex::Application::Instance();
	auto& commandList = app->getRenderer()->getCurrentCommandList();

	commandList.clear();

	apex::gfx::DrawCommand drawCommand;

	math::Matrix4x4 baseJointTransform = math::scale(math::translate(math::Matrix4x4::identity(), { 0, 0.5f, 0 }), { 0.5f, 1.f, 0.5f });
	math::Matrix4x4 baseJointTransform2 = math::scale(math::rotateZ(math::translate(math::Matrix4x4::identity(), { 0, 0.15f, 0 }), math::radians(180)), { 0.5f, 0.3f, 0.5f });
	math::Matrix4x4 endEffectorTransform = math::scale(math::Matrix4x4::identity(), { 0.25f, 0.25f, 0.25f });

	for (size_t i = 0; i < m_ikchain.length(); i++)
	{
		if (m_ikchain.isEndEffector[i])
		{
			auto& jointTransform = m_ikchain.jointGlobalTransforms[i];
			drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[1];
			drawCommand.transform = jointTransform * endEffectorTransform;
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}
		else
		{
			auto& jointTransform = m_ikchain.jointGlobalTransforms[i];
			drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[0];
			drawCommand.transform = jointTransform * baseJointTransform;
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

			drawCommand.transform = jointTransform * baseJointTransform2;
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}
	}

	for (size_t i = 0; i < m_ikchain.endEffectors.size(); i++)
	{
		auto jointTransform = math::translate(math::Matrix4x4::identity(), m_ikchain.endEffectorTargets[i]);
		drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[1];
		drawCommand.transform = jointTransform * endEffectorTransform;
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
	}

	commandList.sortCommands();
}

void IKTrial::stop()
{
	auto& renderer = *apex::Application::Instance()->getRenderer();

	for (auto& mesh : meshes)
		if (mesh)
			mesh.destroy(renderer.getContext().m_device, nullptr);
}

void IKTrial::initIK()
{
	IKChainBuilder builder;
	builder.reserve(5);
    builder.addJoint({ 0, 0, 0 }, { 0, 0, 0 }, 0);
	builder.addJoint({ 0, 1, 0 }, { 0, 0, 0 }, 0);
	builder.addJoint({ 0, 1, 0 }, { 0, 0, 0 }, 1);
	builder.addJoint({ 0, 1, 0 }, { 0, 0, 0 }, 2);
	builder.addJoint({ 0, 1, 0 }, { 0, 0, 0 }, 3, true);

	m_ikchain = builder.build();
}

void IKTrial::updateIK(float dt)
{
	auto method = IKSolverType::FABRIK;
	m_ikchain.solveIK(method);

	if (method == IKSolverType::CCD)
		m_ikchain.updateGlobalTransforms();
}

void detail::solveIK_CCD(IKChain& chain, apex::uint32 max_iterations)
{
	const size_t N = chain.length();
	const int neffectors = chain.endEffectors.size();
	bool converged = false;

	for (apex::uint32 iter = 0; iter < max_iterations && !converged; iter++)
	{
		int nconverged = 0;

		// for each end effector
		for (apex::uint32 i = 0; i < chain.endEffectors.size(); i++)
		{
			apex::uint32 endEffectorIdx = chain.endEffectors[i];
			apex::uint32 jointIdx = chain.parents[endEffectorIdx];
			apex::uint32 parentIdx = chain.parents[jointIdx];

			apex::float32 distance = (chain.jointGlobalTransforms[endEffectorIdx].getTranslation() - chain.endEffectorTargets[i]).lengthSquared();

			// traverse up the chain
			while (distance > 0.0001f)
			{
				// compute the vector from the joint to the end effector
				math::Vector3 jointToEnd = (chain.jointGlobalTransforms[endEffectorIdx].getTranslation() - chain.jointGlobalTransforms[jointIdx].getTranslation()).normalize_();
				// compute the vector from the joint to the target
				math::Vector3 jointToTarget = (chain.endEffectorTargets[i] - chain.jointGlobalTransforms[jointIdx].getTranslation()).normalize_();
				// compute the angle between the two vectors
				apex::float32 cosAngle = math::dot(jointToEnd, jointToTarget); // cos(angle) = a . b when a and b are unit vectors
				if (cosAngle < 0.9999f)
				{
					math::Vector3 crossProd = math::cross(jointToEnd, jointToTarget);
					apex::float32 sinAngle = crossProd.length(); // sin(angle) = |a x b| when a and b are unit vectors
					// apex::float32 angle = atan2(sinAngle, cosAngle);

					// rotate the joint by the angle
					math::Matrix4x4 localTransform = chain.jointLocalTransforms[jointIdx];
					localTransform.m_columns[3] = math::Vector4{ 0, 0, 0, 1 }; // remove translation component
					localTransform = math::rotateAxisAngle(localTransform, crossProd, cosAngle, sinAngle);
					math::Vector3 eulerAngles = math::decomposeRotation(localTransform);

					eulerAngles.x = math::clamp(eulerAngles.x, math::radians(-60.f), math::radians(60.f));
					eulerAngles.y = 0;
					eulerAngles.z = math::clamp(eulerAngles.x, math::radians(-60.f), math::radians(60.f));

					localTransform = math::eulerZYX(eulerAngles.x, eulerAngles.y, eulerAngles.z);

					chain.jointLocalTransforms[jointIdx] = math::translate(localTransform, chain.translations[jointIdx]);

					chain.updateGlobalTransforms();
					distance = (chain.jointGlobalTransforms[endEffectorIdx].getTranslation() - chain.endEffectorTargets[i]).length();
				}

				if (jointIdx == parentIdx)
					break;
				jointIdx = parentIdx;
				parentIdx = chain.parents[jointIdx];
			}

			if (distance <= 0.0001f)
			{
				nconverged++;
			}
		}

		converged = nconverged == neffectors;
	}
}

void detail::solveIK_FABRIK(IKChain& chain, apex::uint32 max_iterations)
{
	const size_t N = chain.length();
	const int neffectors = chain.endEffectors.size();
	bool converged = false;

	for (apex::uint32 iter = 0; iter < max_iterations && !converged; iter++)
	{
		int nconverged = 0;

		for (size_t i = 0; i < chain.endEffectors.size(); i++)
		{
			const math::Vector3 target = chain.endEffectorTargets[i];
			if ((chain.jointGlobalTransforms[chain.endEffectors[i]].getTranslation() - target).lengthSquared() < 0.0001f)
			{
				nconverged++;
				continue;
			}

			const apex::uint32 endEffectorIdx = chain.endEffectors[i];
			apex::AxList<apex::uint32> chainIdxs;
			for (apex::uint32 idx = endEffectorIdx; ; idx = chain.parents[idx])
			{
				chainIdxs.append(idx);
				if (chain.parents[idx] == idx)
					break;
			}
			const apex::uint32 subrootIdx = chainIdxs.back();
			const math::Vector3 subroot = chain.jointGlobalTransforms[subrootIdx].getTranslation();

			math::Vector3 t = target;

			// forward-reaching pass
			for (auto idx : chainIdxs)
			{
				apex::uint32 parentIdx = chain.parents[idx];
				if (idx == subrootIdx)
				{
					chain.jointGlobalTransforms[idx].m_columns[3] = math::Vector4{ t, 1.f };
				}
				else
				{
					chain.jointGlobalTransforms[idx].m_columns[3] = math::Vector4{ t, 1.f };
					apex::float32 d = chain.translations[idx].length();
					apex::float32 r = (chain.jointGlobalTransforms[idx].getTranslation() - chain.jointGlobalTransforms[parentIdx].getTranslation()).length();
					apex::float32 lambda = d / r;

					t = math::lerp(chain.jointGlobalTransforms[idx].getTranslation(), chain.jointGlobalTransforms[parentIdx].getTranslation(), lambda);
				}
			}

			t = subroot;

			// backward-reaching pass
			for (auto it = chainIdxs.rbegin(); it != chainIdxs.rend(); ++it)
			{
				apex::uint32 idx = *it;
				apex::uint32 childIdx = it.next() != chainIdxs.rend() ? *it.next() : endEffectorIdx;
				if (idx == endEffectorIdx)
				{
					chain.jointGlobalTransforms[idx].m_columns[3] = math::Vector4{ t, 1.f };
				}
				else
				{
					chain.jointGlobalTransforms[idx].m_columns[3] = math::Vector4{ t, 1.f };
					apex::float32 d = chain.translations[childIdx].length();
					apex::float32 r = (chain.jointGlobalTransforms[idx].getTranslation() - chain.jointGlobalTransforms[childIdx].getTranslation()).length();
					apex::float32 lambda = d / r;

					t = math::lerp(chain.jointGlobalTransforms[idx].getTranslation(), chain.jointGlobalTransforms[childIdx].getTranslation(), lambda);
				}
			}
		}

		converged = nconverged == neffectors;
	}

	// compute the global transforms from the positions
	for (auto idx : chain.bfsOrder)
	{
		if (idx == chain.parents[idx])
			continue;

		apex::uint32 parentIdx = chain.parents[idx];

		math::Matrix4x4 globalTransform = chain.jointGlobalTransforms[parentIdx];
		globalTransform.m_columns[3] = math::Vector4::unitW(); // remove translation component
		math::Vector3 jointToNext = (globalTransform.transpose() * (chain.jointGlobalTransforms[idx].m_columns[3] - chain.jointGlobalTransforms[parentIdx].m_columns[3])).xyz();
		apex::float32 cosAngle = math::dot(jointToNext, math::Vector3::unitY()); // cos(angle) = a . b when a and b are unit vectors

		if (cosAngle < 0.9999f)
		{
			math::Vector3 crossProd = math::cross(jointToNext, math::Vector3::unitY());
			apex::float32 sinAngle = crossProd.length(); // sin(angle) = |a x b| when a and b are unit vectors
			// apex::float32 angle = atan2(sinAngle, cosAngle);

			// rotate the joint by the angle
			math::Matrix4x4 localTransform = chain.jointLocalTransforms[parentIdx];
			localTransform.m_columns[3] = math::Vector4::unitW(); // remove translation component
			localTransform = math::rotateAxisAngle(localTransform, crossProd, cosAngle, sinAngle);
			math::Vector3 eulerAngles = math::decomposeRotation(localTransform);

			chain.eulerAngles[parentIdx] = eulerAngles;
		}
	}

	chain.updateLocalTransforms();
	chain.updateGlobalTransforms();
}
