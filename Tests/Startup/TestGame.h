#pragma once

#include "Apex/Game.h"
#include "Apex/Application.h"
#include "Apex/ModelLoader.h"
#include "Apex/Window.h"
#include "Graphics/ForwardRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Primitives/Pyramid.h"
#include "Graphics/Primitives/Quad.h"
#include "Math/Matrix4x4.h"

namespace math = apex::math;
using math::Vector3;
using math::Matrix4x4;


class MyGame : public apex::Game
{
public:
	MyGame() = default;

	void initialize() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		renderer.setActiveCamera(&m_camera);
		m_cameraTransform = math::translate(math::Matrix4x4::identity(), { 0, 0, 15 });

		auto pyramidMeshCpu = apex::gfx::Pyramid::getMesh();
		meshes[0].create(renderer.getContext().m_device, &pyramidMeshCpu, nullptr);

		auto quadMeshCpu = apex::gfx::Quad::getMesh();
		meshes[1].create(renderer.getContext().m_device, &quadMeshCpu, nullptr);

		{
			apex::ModelLoader loader;
			(void)loader.readMetadata("X:/ApexGameEngine-Vulkan/assets/Suzanne/Suzanne.gltf");
		}

		{
			apex::gfx::MeshCPU meshCpu;
			apex::float32 X = 0.525731112119133606f;
			apex::float32 Z = 0.850650808352039932f;
			apex::float32 N = 0.f;
			apex::AxArray<apex::gfx::Vertex_P0_C0> vertices = {
				{ .position = { -X, N, Z }, .color = { 1, 0, 0, 1 } },
				{ .position = { X, N, Z }, .color = { 1, 0, 0, 1 } },
				{ .position = { -X, N, -Z }, .color = { 1, 0, 0, 1 } },
				{ .position = { X, N, -Z }, .color = { 1, 0, 0, 1 } },
				{ .position = { N, Z, X }, .color = { 1, 0, 0, 1 } },
				{ .position = { N, Z, -X }, .color = { 1, 0, 0, 1 } },
				{ .position = { N, -Z, X }, .color = { 1, 0, 0, 1 } },
				{ .position = { N, -Z, -X }, .color = { 1, 0, 0, 1 } },
				{ .position = { Z, X, N }, .color = { 1, 0, 0, 1 } },
				{ .position = { -Z, X, N }, .color = { 1, 0, 0, 1 } },
				{ .position = { Z, -X, N }, .color = { 1, 0, 0, 1 } },
				{ .position = { -Z, -X, N }, .color = { 1, 0, 0, 1 } }
			};
			apex::AxArray<apex::uint32> indices = {
				0, 1, 4,
				0, 4, 9,
				9, 4, 5,
				4, 8, 5,
				4, 1, 8,
				8, 1, 10,
				8, 10, 3,
				5, 8, 3,
				5, 3, 2,
				2, 3, 7,

				7, 3, 10,
				7, 10, 6,
				7, 6, 11,
				11, 6, 0,
				0, 6, 1,
				6, 10, 1,
				9, 11, 0,
				9, 2, 11,
				9, 5, 2,
				7, 11, 2
			};
			meshCpu.m_vertexBufferCPU.create(vertices);
			meshCpu.m_indexBufferCPU.create(indices);
			meshCpu.m_subMeshes.resize(2);
			meshCpu.m_subMeshes[0] = { 0, 30 };
			meshCpu.m_subMeshes[1] = { 30, 30 };

			meshes[2].create(renderer.getContext().m_device, &meshCpu, nullptr);
		}

		auto& commandList = renderer.getCurrentCommandList();
		commandList.getCommands().reserve(100);
	}

	void update(float deltaTimeMs) override
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		static auto prevTime = startTime;
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
		prevTime = currentTime;

		//Vector3 cameraPos = Vector3{ 2 * sinf(time * math::radians(30.f)), 2 * cosf(time * math::radians(30.f)), 5 };
		auto& inputManager = *apex::Application::Instance()->getInputManager();
		if (inputManager.getKeyState(apex::KeyCode::KeyUp) == apex::InputState::Pressed)
		{
			m_cameraTransform = math::translate(m_cameraTransform, { 0, 2.f * deltaTime, 0 });
		}
		if (inputManager.getKeyState(apex::KeyCode::KeyDown) == apex::InputState::Pressed)
		{
			m_cameraTransform = math::translate(m_cameraTransform, { 0, -2.f * deltaTime, 0 });
		}
		if (inputManager.getKeyState(apex::KeyCode::KeyLeft) == apex::InputState::Pressed)
		{
			m_cameraTransform = math::translate(m_cameraTransform, (m_camera.view.inverse() * math::Vector4{ -2.f * deltaTime, 0, 0, 0 }).xyz());
		}
		if (inputManager.getKeyState(apex::KeyCode::KeyRight) == apex::InputState::Pressed)
		{
			m_cameraTransform = math::translate(m_cameraTransform, (m_camera.view.inverse() * math::Vector4{ 2.f * deltaTime, 0, 0, 0 }).xyz());
		}
		//m_cameraTransform = math::translate(Matrix4x4::identity(), cameraPos);

		// m_camera.view = math::inverse(m_cameraTransform);
		m_camera.view = math::lookAt(m_cameraTransform.getTranslation(), { 0, 0, 0 }, Vector3::unitY());

		int width, height;
		apex::Application::Instance()->getWindow()->getFramebufferSize(width, height);
		apex::float32 aspect = static_cast<apex::float32>(width) / static_cast<apex::float32>(height);
		apex::float32 fov = math::radians(60.f);
		m_camera.projection = math::perspective(fov, aspect, 0.1f, 1000.f);
		m_camera.projection[1][1] *= -1;

		auto app = apex::Application::Instance();
		auto& commandList = app->getRenderer()->getCurrentCommandList();

		commandList.clear();

		apex::gfx::DrawCommand drawCommand;
		/*drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[0];
		drawCommand.transform = math::rotateY(math::Matrix4x4::identity(), time * math::radians(90.f));
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));*/

		Matrix4x4 transform = math::rotateY(math::Matrix4x4::identity(), time * math::radians(90.f));

		//drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[0];
		//drawCommand.transform = math::translate(transform, { 2 * sinf(time), -2 * cosf(time), 0 });
		//commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		//drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[1];
		//drawCommand.transform = math::translate(transform, {2 * sinf(time + 0.5f * apex::constants::float32_PI), -1 * cosf(time + 0.5f * apex::constants::float32_PI), 0});
		//commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		//drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[0];
		//drawCommand.transform = math::translate(transform, {1 * sinf(time + 1.f * apex::constants::float32_PI), -2 * cosf(time + 1.0f * apex::constants::float32_PI), 0});
		//commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		//drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[1];
		//drawCommand.transform = math::translate(transform, {1 * sinf(time + 1.5f * apex::constants::float32_PI), -1 * cosf(time + 1.5f * apex::constants::float32_PI), 0});
		//commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		int r = 10, c = 10;
		for (int i = 0; i < r; i++) for (int j = 0; j < c; j++)
		{
			drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[0];
			drawCommand.transform = math::translate(transform, { 2.f * ((2*i - r + 1)/2.f), 0, -2.f * ((2*j - c + 1)/2.f) });
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}

		transform = math::rotateY(math::Matrix4x4::identity(), std::sin(time * 2.f) * math::radians(90.f));

		drawCommand.pMesh = (apex::gfx::StaticMesh*)&meshes[2];
		drawCommand.transform = math::translate(transform, { 0, 3, 0 });
		drawCommand.subMeshIdx = 0;
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		drawCommand.transform = math::translate(transform, { 0, 2 + 0.5f * std::sin(time * 3.f), 0 });
		drawCommand.subMeshIdx = 1;
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		commandList.sortCommands();
	}

	void stop() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		for (auto& mesh : meshes)
			if (mesh)
				mesh.destroy(renderer.getContext().m_device, nullptr);
	}

private:
	apex::gfx::Camera m_camera;
	apex::gfx::StaticMesh meshes[3]; // TODO: move to resource manager
	Matrix4x4 m_cameraTransform;
};
