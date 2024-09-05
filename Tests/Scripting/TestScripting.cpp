#include <fmt/core.h>

#include "Apex/Lua/LuaManager.h"


namespace apex {
namespace anim {

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

		void update()
		{
			switch (curState)
			{
			case apex::anim::State::Idle:
				fmt::print("Idling...\n");
				break;
			case apex::anim::State::Walk:
				fmt::print("Walking...\n");
				break;
			case apex::anim::State::Run:
				fmt::print("Running...\n");
				break;
			default:
				fmt::print("Invalid state\n");
				break;
			}

			if (nextState != State::Invalid && nextState != curState)
			{
				prevState = curState;
				curState = nextState;
				nextState = State::Invalid;
			}
		}
	};

}

namespace lua 
{
	int l_anim_StateMachine_Walk(lua_State* L)
	{
		apex::anim::StateMachine* sm = static_cast<apex::anim::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::anim::State::Walk;
		lua_pop(L, 1);
		return 0;
	}

	int l_anim_StateMachine_Run(lua_State* L)
	{
		apex::anim::StateMachine* sm = static_cast<apex::anim::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::anim::State::Run;
		lua_pop(L, 1);
		return 0;
	}

	int l_anim_StateMachine_Idle(lua_State* L)
	{
		apex::anim::StateMachine* sm = static_cast<apex::anim::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::anim::State::Idle;
		lua_pop(L, 1);
		return 0;
	}
}
}


int main(int argc, char **argv)
{
	apex::lua::LuaState lua;
	apex::lua::LuaResult luaRes;

	{
		lua_createtable(lua.raw(), 0, 1);

		lua_pushstring(lua.raw(), "anim");
		lua_createtable(lua.raw(), 0, 1);

		{
			lua_pushstring(lua.raw(), "StateMachine");
			lua_createtable(lua.raw(), 0, 3);

			{
				lua_pushstring(lua.raw(), "Walk");
				lua_pushcfunction(lua.raw(), apex::lua::l_anim_StateMachine_Walk);
				lua_settable(lua.raw(), -3);

				lua_pushstring(lua.raw(), "Run");
				lua_pushcfunction(lua.raw(), apex::lua::l_anim_StateMachine_Run);
				lua_settable(lua.raw(), -3);

				lua_pushstring(lua.raw(), "Idle");
				lua_pushcfunction(lua.raw(), apex::lua::l_anim_StateMachine_Idle);
				lua_settable(lua.raw(), -3);
			}
			fmt::print("[StateMachine] top: {0}\n", lua_gettop(lua.raw()));
			lua_settable(lua.raw(), -3);
		}
		fmt::print("[anim] top: {0}\n", lua_gettop(lua.raw()));
		lua_settable(lua.raw(), -3);

		fmt::print("[apex] top: {0}\n", lua_gettop(lua.raw()));
		lua_setglobal(lua.raw(), "apex");
	}

	luaRes = lua.doFile("X:\\ApexGameEngine-Vulkan\\Tests\\Scripting\\scripts\\test.lua");
	if (apex::lua::Ok != luaRes)
	{
		fmt::print("Error loading test.lua\n[LUA_ERROR]: {0}\n", lua.getError());
		return 1;
	}

	apex::anim::StateMachine sm;
	sm.curState = apex::anim::State::Idle;

	lua_getglobal(lua.raw(), "apex_start");
	lua_pushlightuserdata(lua.raw(), &sm);
	lua_pcall(lua.raw(), 1, 0, 0);

	while (true)
	{
		lua_getglobal(lua.raw(), "apex_update");
		lua_pcall(lua.raw(), 0, 0, 0);

		sm.update();

		for (int i = 0; i < 1000000000; i++);
	}

	return 0;
}
