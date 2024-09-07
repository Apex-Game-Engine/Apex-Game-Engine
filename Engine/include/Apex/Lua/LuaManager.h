#pragma once
#include <lua.hpp>

struct lua_State;

namespace apex {
namespace lua {

	struct LuaStateCreateOptions
	{
	};

	enum class LuaType
	{
		Nil = 0,
		Boolean = 1,
		LightUserData = 2,
		Number = 3,
		String = 4,
		Table = 5,
		Function = 6,
		UserData = 7,
		Thread = 8,
	};

	enum LuaResult
	{
		Ok = 0,
		Error = 1,
	};

	using PFN_LuaCFunction = int(*)(lua_State*);

	class LuaState
	{
	public:
		LuaState(LuaStateCreateOptions const& options = {});
		~LuaState();

		LuaResult loadFile(const char* filename) const;
		[[maybe_unused]] LuaResult doFile(const char* filename) const;

		LuaResult loadString(char const* code) const;
		[[maybe_unused]] LuaResult doString(char const* code) const;

		// TODO: This only registers static or global functions to a table.
		// We want to be able to register functions as Lua globals and also allow registering C++ member functions.
		[[maybe_unused]] LuaResult registerCFunction(PFN_LuaCFunction pfunc, const char* lua_name) const;
		LuaResult registerCClosure(PFN_LuaCFunction pfunc, int n, const char* lua_name) const;
		LuaResult registerCClosureGlobal(PFN_LuaCFunction pfunc, int n, const char* lua_name);

		// TODO: This only calls global functions. We want to be able to call member functions.
		LuaResult callFunction(const char* lua_name, int nargs, int nresults) const;

		[[maybe_unused]] LuaResult createTable(int narr, int nrec) const;

		// TODO: This only creates tables in the global scope. We want to be able to create tables in other tables.
		[[maybe_unused]] LuaResult createTable(const char* name) const;
		[[maybe_unused]] LuaResult createTable(const char* name, int narr, int nrec) const;

		LuaResult pushLightUserData(void* lightuserdata);

		LuaResult setGlobal(const char* name) const;

		lua_State* raw() const { return m_lua; }

		const char* getError() const;

	private:
		lua_State *m_lua;
	};

}
}
