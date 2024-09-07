#include "LuaStateMachineExample.h"

#include <fmt/core.h>

namespace detail 
{
	int l_test_StateMachine_Walk(lua_State* L)
	{
		apex::test::StateMachine* sm = static_cast<apex::test::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::test::State::Walk;
		lua_pop(L, 1);
		return 0;
	}

	int l_test_StateMachine_Run(lua_State* L)
	{
		apex::test::StateMachine* sm = static_cast<apex::test::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::test::State::Run;
		lua_pop(L, 1);
		return 0;
	}

	int l_test_StateMachine_Idle(lua_State* L)
	{
		apex::test::StateMachine* sm = static_cast<apex::test::StateMachine*>(lua_touserdata(L, 1));
		sm->nextState = apex::test::State::Idle;
		lua_pop(L, 1);
		return 0;
	}
}


void apex::test::StateMachine::update()
{
	switch (curState)
	{
	case State::Idle:
		axLog("Idling...\n");
		break;
	case State::Walk:
		axLog("Walking...\n");
		break;
	case State::Run:
		axLog("Running...\n");
		break;
	default:
		axLog("Invalid state\n");
		break;
	}

	if (nextState != State::Invalid && nextState != curState)
	{
		prevState = curState;
		curState = nextState;
		nextState = State::Invalid;
	}
}

void LuaStateMachineExample::initialize()
{
	apex::lua::LuaResult luaRes;

	{
		lua_createtable(m_lua.raw(), 0, 1);

		lua_pushstring(m_lua.raw(), "anim");
		lua_createtable(m_lua.raw(), 0, 1);

		{
			lua_pushstring(m_lua.raw(), "StateMachine");
			lua_createtable(m_lua.raw(), 0, 3);

			{
				lua_pushstring(m_lua.raw(), "Walk");
				lua_pushcfunction(m_lua.raw(), detail::l_test_StateMachine_Walk);
				lua_settable(m_lua.raw(), -3);

				lua_pushstring(m_lua.raw(), "Run");
				lua_pushcfunction(m_lua.raw(), detail::l_test_StateMachine_Run);
				lua_settable(m_lua.raw(), -3);

				lua_pushstring(m_lua.raw(), "Idle");
				lua_pushcfunction(m_lua.raw(), detail::l_test_StateMachine_Idle);
				lua_settable(m_lua.raw(), -3);
			}
			
			lua_settable(m_lua.raw(), -3);
		}
		
		lua_settable(m_lua.raw(), -3);

		lua_setglobal(m_lua.raw(), "apex");
	}

	luaRes = m_lua.doFile(R"(X:\ApexGameEngine-Vulkan\Tests\Scripting\scripts\test_state_machine.lua)");
	if (apex::lua::Ok != luaRes)
	{
		axErrorFmt("Error loading test.lua\n[LUA_ERROR]: {0}\n", m_lua.getError());
	}

	sm.curState = apex::test::State::Idle;

	lua_getglobal(m_lua.raw(), "apex_start");
	lua_pushlightuserdata(m_lua.raw(), &sm);
	lua_pcall(m_lua.raw(), 1, 0, 0);
}

void LuaStateMachineExample::update(float deltaTimeMs)
{
	lua_getglobal(m_lua.raw(), "apex_update");
	lua_pcall(m_lua.raw(), 0, 0, 0);

	sm.update();

	for (int i = 0; i < 1000000000; i++);
}
