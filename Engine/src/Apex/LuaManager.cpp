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

	LuaResult LuaState::loadFile(const char* filename) const
	{
		int result = luaL_loadfile(m_lua, filename);
		return static_cast<LuaResult>(result);
	}

	LuaResult LuaState::doFile(const char* filename) const
	{
		int result = luaL_dofile(m_lua, filename);
		return static_cast<LuaResult>(result);
	}

	LuaResult LuaState::loadString(char const* code) const
	{
		int result = luaL_loadstring(m_lua, code);
		return static_cast<LuaResult>(result);
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

	LuaResult LuaState::callFunction(const char* lua_name, int nargs, int nresults) const
	{
		lua_getglobal(m_lua, lua_name);
		int result = lua_pcall(m_lua, nargs, nresults, 0);
		return static_cast<LuaResult>(result);
	}

	const char* LuaState::getError() const
	{
		return lua_tostring(m_lua, -1);
	}
}
}
