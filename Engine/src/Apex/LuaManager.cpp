#include "apex_pch.h"
#include "Apex/Lua/LuaManager.h"

#include <lua.hpp>

namespace apex {
namespace lua {

	LuaState::LuaState(LuaStateCreateOptions const& options)
	{
		m_lua = luaL_newstate();
		luaL_openlibs(m_lua);
	}

	LuaState::~LuaState()
	{
		lua_close(m_lua);
	}

	LuaResult LuaState::doString(char const* code) const
	{
		int result = luaL_dostring(m_lua, code);
		return static_cast<LuaResult>(result);
	}

	LuaResult LuaState::registerFunction(PFN_LuaCFunction pfunc, const char* lua_name) const
	{
		lua_pushcfunction(m_lua, pfunc);
		lua_setfield(m_lua, -2, lua_name);
		return LuaResult::Ok;
	}
}
}
