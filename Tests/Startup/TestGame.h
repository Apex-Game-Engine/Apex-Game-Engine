#pragma once

#include "Apex/Game.h"
#include "Apex/Application.h"
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
		m_cameraTransform = math::translate(math::Matrix4x4::identity(), { 0, 0, -5 });

		auto pyramidMeshCpu = apex::gfx::Pyramid::getMesh();
		meshes[0].create(renderer.getContext().m_device, &pyramidMeshCpu, nullptr);

		auto quadMeshCpu = apex::gfx::Quad::getMesh();
		meshes[1].create(renderer.getContext().m_device, &quadMeshCpu, nullptr);

		auto& commandList = renderer.getCurrentCommandList();
		commandList.getCommands().reserve(100);
	}

	void update(float deltaTimeMs) override
	{
		auto app = apex::Application::Instance();
		auto& commandList = app->getRenderer()->getCurrentCommandList();

		commandList.clear();

		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		
		Vector3 cameraPos = Vector3{ 2 * sinf(time * math::radians(30.f)), 2 * cosf(time * math::radians(30.f)), 5 };
		m_cameraTransform = math::translate(Matrix4x4::identity(), cameraPos);

		// m_camera.view = math::inverse(m_cameraTransform);
		m_camera.view = math::lookAt(cameraPos, { 0, 0, 0 }, Vector3::unitY());

		int width, height;
		apex::Application::Instance()->getWindow()->getFramebufferSize(width, height);
		apex::float32 aspect = static_cast<apex::float32>(width) / static_cast<apex::float32>(height);
		apex::float32 fov = math::radians(60.f);
		m_camera.projection = math::perspective(fov, aspect, 0.1f, 1000.f);
		m_camera.projection[1][1] *= -1;

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
			drawCommand.transform = math::translate(transform, { 2.f * ((2*i - r + 1)/2.f), 0, -2.f * j });
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}

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
