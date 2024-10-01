#include "apex_pch.h"
#include "Apex/Lua/LuaManager.h"

#include <lua.hpp>

namespace apex {

	namespace generate
	{
		float perlinNoise(int x, int y)
		{
			
		}

		int l_perlinNoise(lua_State* L)
		{
			lua_Integer x = luaL_checkinteger(L, 1);
			lua_Integer y = luaL_checkinteger(L, 2);
			lua_pushnumber(L, perlinNoise(static_cast<int>(x), static_cast<int>(y)));
			return 1;
		}
	}


namespace lua {

	const luaL_Reg apex_lib[] = {
		{ "perlinNoise", apex::generate::perlinNoise },
		{ nullptr, nullptr }
	};

	LUAMOD_API int luaopen_apex(lua_State* L)
	{
		luaL_newlib(L, );
	}

	LuaTable& LuaTable::setField(const char* key)
	{
		lua_setfield(m_lua->raw(), -2, key);
		return *this;
	}

	LuaState::LuaState(LuaStateCreateOptions const& options)
	{
		m_lua = luaL_newstate();
		if (options.loadStdLibs)
			luaL_openlibs(m_lua);
		if (options.loadApexLib)
			luaL_requiref(m_lua, "apex", luaopen_apex, 1);
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

	LuaResult LuaState::registerCFunction(PFN_LuaCFunction pfunc, const char* lua_name) const
	{
		lua_pushcfunction(m_lua, pfunc);
		lua_setfield(m_lua, -2, lua_name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::registerCClosure(PFN_LuaCFunction pfunc, int n, const char* lua_name) const
	{
		lua_pushcclosure(m_lua, pfunc, n);
		lua_setfield(m_lua, -2, lua_name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::registerCClosureGlobal(PFN_LuaCFunction pfunc, int n, const char* lua_name)
	{
		lua_pushcclosure(m_lua, pfunc, n);
		lua_setglobal(m_lua, lua_name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::callFunction(const char* lua_name, int nargs, int nresults) const
	{
		lua_getglobal(m_lua, lua_name);
		lua_insert(m_lua, -nargs - 1);
		int result = lua_pcall(m_lua, nargs, nresults, 0);
		return static_cast<LuaResult>(result);
	}

	LuaResult LuaState::createTable(int narr, int nrec) const
	{
		lua_createtable(m_lua, narr, nrec);
		return LuaResult::Ok;
	}

	LuaResult LuaState::createTable(const char* name) const
	{
		lua_newtable(m_lua);
		lua_setglobal(m_lua, name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::createTable(const char* name, int narr, int nrec) const
	{
		lua_createtable(m_lua, narr, nrec);
		lua_setglobal(m_lua, name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::pushLightUserData(void* lightuserdata)
	{
		lua_pushlightuserdata(m_lua, lightuserdata);
		return LuaResult::Ok;
	}

	LuaResult LuaState::pushNumber(double number)
	{
		lua_pushnumber(m_lua, number);
		return LuaResult::Ok;
	}

	LuaResult LuaState::pushInteger(int number)
	{
		lua_pushinteger(m_lua, number);
		return LuaResult::Ok;
	}

	LuaResult LuaState::pushString(const char* string)
	{
		lua_pushstring(m_lua, string);
		return LuaResult::Ok;
	}

	LuaResult LuaState::pushNamespace(const char* name)
	{
		lua_getglobal(m_lua, name);
		return LuaResult::Ok;
	}

	LuaResult LuaState::setGlobal(const char* name) const
	{
		lua_setglobal(m_lua, name);
		return LuaResult::Ok;
	}

	const char* LuaState::getError() const
	{
		return lua_tostring(m_lua, -1);
	}
}
}
