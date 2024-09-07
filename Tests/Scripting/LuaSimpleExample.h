#pragma once
#include "Apex/Game.h"
#include "Apex/Lua/LuaManager.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry/Mesh.h"

class LuaSimpleExample : public apex::Game
{
public:
	void initialize() override;
	void update(float deltaTimeMs) override;
	void stop() override;

protected:
	apex::lua::LuaState m_lua{};
	apex::gfx::StaticMesh m_mesh{};
	apex::gfx::Camera m_camera{};
	apex::math::Matrix4x4 m_cameraTransform{};
};
