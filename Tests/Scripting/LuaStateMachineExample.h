#pragma once
#include "Apex/Game.h"
#include "Apex/Lua/LuaManager.h"
#include "Graphics/Geometry/Mesh.h"

namespace apex {
namespace test {

	enum class State
	{
		Invalid,
		Idle,
		Walk,
		Run,
	};

	struct StateMachine
	{
		State curState = State::Invalid;
		State nextState = State::Invalid;
		State prevState = State::Invalid;

		void update();
	};

}
}

class LuaStateMachineExample : public apex::Game
{
public:
	void initialize() override;
	void update(float deltaTimeMs) override;
	void stop() override {}

protected:
	apex::lua::LuaState m_lua;
	apex::test::StateMachine sm;

	apex::gfx::StaticMesh mesh;
};
