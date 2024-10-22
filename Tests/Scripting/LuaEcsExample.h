#pragma once
#include "Apex/Game.h"
#include "Apex/ECS/Registry.h"
#include "Apex/Lua/LuaManager.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry/Mesh.h"

class LuaEcsExample : public apex::Game
{
public:
	void initialize() override;
	void update(float deltaTimeMs) override;
	void stop() override;

private:
	apex::ecs::Registry m_registry{};
	apex::lua::LuaState m_lua{};
	apex::gfx::StaticMesh m_meshes[8]{};
	apex::gfx::Camera m_camera{};
	apex::math::Matrix4x4 m_cameraTransform{};
};
