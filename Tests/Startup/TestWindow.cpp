#include <apex_pch.h>
#include <Apex/Game.h>
#include <Apex/Application.h>

#include "Graphics/ForwardRenderer.h"
#include "Graphics/Primitives/Quad.h"

class MyGame : public apex::Game
{
public:
	MyGame() = default;

	void initialize() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		auto pyramidMeshCpu = apex::gfx::Pyramid::getMesh();
		meshes[0].create(renderer.getContext().m_device, &pyramidMeshCpu, nullptr);

		auto quadMeshCpu = apex::gfx::Quad::getMesh();
		meshes[1].create(renderer.getContext().m_device, &quadMeshCpu, nullptr);

		auto& commandList = renderer.getCurrentCommandList();
		commandList.getCommands().reserve(8);
	}

	void run() override
	{
		auto app = apex::Application::Instance();
		auto& commandList = app->getRenderer()->getCurrentCommandList();

		commandList.clear();

		apex::gfx::DrawCommand drawCommand;
		drawCommand.pMesh = (apex::gfx::Mesh*)&meshes[0];
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));

		drawCommand.pMesh = (apex::gfx::Mesh*)&meshes[1];
		commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
	}

	void stop() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		for (auto& mesh : meshes)
			if (mesh)
				mesh.destroy(renderer.getContext().m_device.logicalDevice, nullptr);
	}

private:
	apex::gfx::Mesh meshes[3];
};

apex::UniquePtr<apex::Game> apex::Game::Construct()
{
	return apex::static_unique_cast<Game>(apex::make_unique<MyGame>());
}

#include <Startup.h>
