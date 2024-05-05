#pragma once
#include <lua.h>

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

		LuaResult doString(char const* code) const;

		LuaResult registerFunction(PFN_LuaCFunction pfunc, const char* lua_name) const;

		template <typename T>
		LuaResult registerFunction(T pfunc, const char* lua_name) const
		{
			
		}

		lua_State* raw() const { return m_lua; }

	private:
		lua_State *m_lua;
	};

}
}
