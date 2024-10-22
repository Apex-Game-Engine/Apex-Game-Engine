#pragma once
#include <lua.hpp>

struct lua_State;

namespace apex {
namespace lua {
	class LuaState;

	struct LuaStateCreateOptions
	{
		bool loadStdLibs = true;
		bool loadApexLib = true;
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

	class LuaTable
	{
	public:
		LuaTable& setField(const char* key);

		template <typename T>
		LuaTable& setField(const char* key, const T& value);

	private:
		LuaState* m_lua;
	};

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
		[[maybe_unused]] LuaResult registerCFunctionToTable(PFN_LuaCFunction pfunc, const char* lua_name) const;
		LuaResult registerCClosureToTable(PFN_LuaCFunction pfunc, int n, const char* lua_name) const;
		LuaResult registerCFunction(PFN_LuaCFunction pfunc, const char* lua_name) const;
		LuaResult registerCClosure(PFN_LuaCFunction pfunc, int n, const char* lua_name) const;

		// TODO: This only calls global functions. We want to be able to call member functions.
		LuaResult callFunction(const char* lua_name, int nargs, int nresults) const;

		[[maybe_unused]] LuaResult createTable(int narr, int nrec) const;

		// TODO: This only creates tables in the global scope. We want to be able to create tables in other tables.
		[[maybe_unused]] LuaResult createTable(const char* name) const;
		[[maybe_unused]] LuaResult createTable(const char* name, int narr, int nrec) const;

		LuaResult pushLightUserData(void* lightuserdata);
		LuaResult pushNumber(double number);
		LuaResult pushInteger(int number);
		LuaResult pushString(const char* string);
		LuaResult pushNamespace(const char* name);

		LuaResult setGlobal(const char* name) const;
		LuaResult setField(int idx, const char* name) const;

		lua_State* raw() const { return m_lua; }

		const char* getError() const;

	private:
		lua_State *m_lua;
	};



	template <typename Func, typename RetFn, typename ...ArgFns>
	void registerFunction(lua_State * lua, Func&& func)
	{
		lua_pushlightuserdata(lua, new Func(std::forward<Func>(func)));
		lua_pushcclosure(lua, [](lua_State * lua) -> int
		{
			auto func = reinterpret_cast<Func*>(lua_touserdata(lua, lua_upvalueindex(1)));
			return RetFn::call(lua, *func, ArgFns::call(lua)...);
		}, 1);
	}


}
}
