#include "LuaEcsExample.h"

#include "IpcConnection.h"
#include "Apex/Application.h"
#include "Apex/ECS/Registry.h"
#include "Apex/ECS/Transform.h"
#include "Graphics/ForwardRenderer.h"

#include <unordered_map>

#define nameof(type) STR(type)

using namespace apex;

namespace apex::internal
{
    constexpr uint32_t fnv_prime_32 = 16777619;
    constexpr uint64_t fnv_prime_64 = 1099511628211U;

    constexpr uint32_t fnv_offset_basis_32 = 2166136261;
    constexpr uint64_t fnv_offset_basis_64 = 14695981039346656037U;

    constexpr uint64_t hash_fnv1a_64(const char* const str, const uint64_t hash = fnv_offset_basis_64)
    {
        // hash = fnv_offset_basis_64
        // hash = hash ^ str[idx]
        // hash = hash * fnv_prime_64
        return (str[0] == '\0') ? hash : hash_fnv1a_64(&str[1], (hash ^ str[0]) * fnv_prime_64);
    }

	constexpr uint64_t hash_fnv1a_64_sz(const char* const str, const size_t size, const uint64_t hash = fnv_offset_basis_64)
    {
	    return (str[0] == '\0' || size == 0) ? hash : hash_fnv1a_64_sz(&str[1], size - 1, (hash ^ str[0]) * fnv_prime_64);
    }

	template <typename T>
	struct AxHash {};

	template <>
	struct AxHash<const char*>
	{
		uint64 operator()(const char* str) const
		{
			return hash_fnv1a_64(str);
		}
	};

	template <>
	struct AxHash<AxStringView>
	{
		uint64 operator()(AxStringView const& sv) const
		{
			return hash_fnv1a_64_sz(sv.m_str, sv.m_size);
		}
	};
}

template <typename T>
using ResourceStorage = std::unordered_map<const char*, T, internal::AxHash<const char*>>;

ResourceStorage<gfx::Mesh*> g_meshes;

namespace lua_setup {

	using namespace apex::lua;

	LuaResult RegisterEcsTransform(LuaState& lua)
	{
		luaL_newmetatable(lua.raw(), "Transform");

		lua_pushliteral(lua.raw(), "__index"); // meta-method to index into table
		lua_pushcfunction(lua.raw(), [](lua_State* L) -> int
		{
			apex::ecs::Transform const* transform = static_cast<apex::ecs::Transform const*>(luaL_checkudata(L, 1, "Transform"));
			const char* key = luaL_checkstring(L, 2);
			if (_strcmpi(key, "position") == 0)
			{
				lua_createtable(L, 3, 0);
				lua_pushnumber(L, transform->position.x);
				lua_rawseti(L, -2, 1);
				lua_pushnumber(L, transform->position.y);
				lua_rawseti(L, -2, 2);
				lua_pushnumber(L, transform->position.z);
				lua_rawseti(L, -2, 3);
			}
			else if (_strcmpi(key, "rotation") == 0)
			{
				lua_createtable(L, 4, 0);
				lua_pushnumber(L, transform->rotation.x);
				lua_rawseti(L, -2, 1);
				lua_pushnumber(L, transform->rotation.y);
				lua_rawseti(L, -2, 2);
				lua_pushnumber(L, transform->rotation.z);
				lua_rawseti(L, -2, 3);
				lua_pushnumber(L, transform->rotation.w);
				lua_rawseti(L, -2, 4);
			}
			return 1;
		});
		lua_settable(lua.raw(), -3);

		lua_pushliteral(lua.raw(), "__newindex"); // meta-method to add or set elements to the table
		lua_pushcfunction(lua.raw(), [](lua_State* L) -> int
		{
			apex::ecs::Transform* transform = static_cast<apex::ecs::Transform*>(luaL_checkudata(L, 1, "Transform"));
			const char* key = luaL_checkstring(L, 2);
			if (strcmp(key, "position") == 0)
			{
				//axAssertMsg(lua_istable(L, 3), "Expected a table for argument #3");
				luaL_checktype(L, 3, LUA_TTABLE);
				if (lua_rawgeti(L, 3, 1) == LUA_TNUMBER) // x
					transform->position.x = static_cast<float>(lua_tonumber(L, -3));
				if (lua_rawgeti(L, 3, 2) == LUA_TNUMBER) // y
					transform->position.y = static_cast<float>(lua_tonumber(L, -2));
				if (lua_rawgeti(L, 3, 3) == LUA_TNUMBER) // z
					transform->position.z = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 3);
			}
			else if (strcmp(key, "rotation") == 0)
			{
				// axAssertMsg(lua_istable(L, 3), "Expected a table for argument #3");
				luaL_checktype(L, 3, LUA_TTABLE);
				if (lua_rawgeti(L, 3, 1) == LUA_TNUMBER) // x
					transform->rotation.x = static_cast<float>(lua_tonumber(L, -1));
				if (lua_rawgeti(L, 3, 2) == LUA_TNUMBER) // y
					transform->rotation.y = static_cast<float>(lua_tonumber(L, -1));
				if (lua_rawgeti(L, 3, 3) == LUA_TNUMBER) // z
					transform->rotation.z = static_cast<float>(lua_tonumber(L, -1));
				if (lua_rawgeti(L, 3, 4) == LUA_TNUMBER) // w
					transform->rotation.w = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 4);
			}
			return 0;
		});
		lua_settable(lua.raw(), -3);

		lua.registerCFunction([](lua_State* L) -> int
		{
			apex::ecs::Transform* transform = static_cast<apex::ecs::Transform*>(lua_newuserdatauv(L, sizeof(apex::ecs::Transform), 0));
			transform->position = apex::math::Vector3{};
			transform->rotation = apex::math::Quat{};
			luaL_setmetatable(L, "Transform");

			if (lua_gettop(L) > 1)
			{
				luaL_checktype(L, 1, LUA_TTABLE);

				lua_pushliteral(L, "position");
				if (lua_rawget(L, 1) == LUA_TTABLE)
				{
					if (lua_rawgeti(L, 3, 1) == LUA_TNUMBER) // x
						transform->position.x = static_cast<float>(lua_tonumber(L, -1));
					if (lua_rawgeti(L, 3, 2) == LUA_TNUMBER) // y
						transform->position.y = static_cast<float>(lua_tonumber(L, -1));
					if (lua_rawgeti(L, 3, 3) == LUA_TNUMBER) // z
						transform->position.z = static_cast<float>(lua_tonumber(L, -1));
					lua_pop(L, 3);
				}
				lua_pop(L, 1);

				lua_pushliteral(L, "rotation");
				if (lua_rawget(L, 1) == LUA_TTABLE)
				{
					if (lua_rawgeti(L, 3, 1) == LUA_TNUMBER) // x
						transform->rotation.x = static_cast<float>(lua_tonumber(L, -1));
					if (lua_rawgeti(L, 3, 2) == LUA_TNUMBER) // y
						transform->rotation.y = static_cast<float>(lua_tonumber(L, -1));
					if (lua_rawgeti(L, 3, 3) == LUA_TNUMBER) // z
						transform->rotation.z = static_cast<float>(lua_tonumber(L, -1));
					if (lua_rawgeti(L, 3, 4) == LUA_TNUMBER) // w
						transform->rotation.w = static_cast<float>(lua_tonumber(L, -1));
					lua_pop(L, 4);
				}
				lua_pop(L, 1);
			}

			return 1;
		}, "CreateTransform");

		lua_pop(lua.raw(), 1);

		return LuaResult::Ok;
	}

	LuaResult RegisterEcsComponents(LuaState& lua)
	{
		RegisterEcsTransform(lua);

		return LuaResult::Ok;
	}

	LuaResult RegisterEcsFunctions(LuaState& lua)
	{
		lua.registerCFunctionToTable([](lua_State* L) -> int
		{
			apex::ecs::Registry& reg = *static_cast<apex::ecs::Registry*>(luaL_checkudata(L, 1, nameof(apex::ecs::Registry)));

			apex::ecs::Entity e = reg.createEntity();

			const char* prototype = luaL_checkstring(L, 2);
			(void)prototype; // use the prototype to construct the components of the entity
			if (ResourceStorage<gfx::Mesh*>::iterator it = g_meshes.find(prototype); it != g_meshes.end())
			{
				ecs::MeshRenderer& meshComp = reg.add<ecs::MeshRenderer>(e);
				meshComp.pMesh = it->second;
			}

			apex::ecs::Transform const& transform = *static_cast<apex::ecs::Transform const*>(luaL_checkudata(L, 3, "Transform"));

			reg.add<apex::ecs::Transform>(e) = transform;

			lua_pushinteger(L, e.uint);

			return 1;
		}, "SpawnEntity");

		return LuaResult::Ok;
	}

	LuaResult RegisterEcs(apex::lua::LuaState& lua)
	{
		luaL_newmetatable(lua.raw(), nameof(apex::ecs::Registry)); // -3
		lua_pushliteral(lua.raw(), "__index"); // -2
		lua_pushvalue(lua.raw(), -2); // -1
		lua_rawset(lua.raw(), -3);

		RegisterEcsFunctions(lua);

		return LuaResult::Ok;
	}

	LuaResult SetEcsRegistry(LuaState& lua, apex::ecs::Registry& registry)
	{
		lua_pushlightuserdata(lua.raw(), &registry);
		luaL_setmetatable(lua.raw(), nameof(apex::ecs::Registry));
		return lua.setGlobal("Scene");
	}
}

apex::ipc::IpcSocket g_serverSocket;
apex::ipc::IpcSocket g_clientSocket;
char g_socketBuffer[1024];

void LuaEcsExample::initialize()
{
	auto& renderer = *Application::Instance()->getRenderer();

	renderer.setActiveCamera(&m_camera);

	m_cameraTransform = math::translate(math::Matrix4x4::identity(), { 0, 5, 5 });

	// Icosphere
	{
		gfx::MeshCPU meshCpu;
		float32 X = 0.525731112119133606f;
		float32 Z = 0.850650808352039932f;
		float32 N = 0.f;
		AxArray<gfx::Vertex_P0_C0> vertices = {
			{ .position = { -X, N, Z }, .color = { 1, 0, 0, 1 } },
			{ .position = { X, N, Z }, .color = { 1, 0, 0, 1 } },
			{ .position = { -X, N, -Z }, .color = { 1, 0, 0, 1 } },
			{ .position = { X, N, -Z }, .color = { 1, 0, 0, 1 } },
			{ .position = { N, Z, X }, .color = { 1, 0, 0, 1 } },
			{ .position = { N, Z, -X }, .color = { 1, 0, 0, 1 } },
			{ .position = { N, -Z, X }, .color = { 1, 0, 0, 1 } },
			{ .position = { N, -Z, -X }, .color = { 1, 0, 0, 1 } },
			{ .position = { Z, X, N }, .color = { 1, 0, 0, 1 } },
			{ .position = { -Z, X, N }, .color = { 1, 0, 0, 1 } },
			{ .position = { Z, -X, N }, .color = { 1, 0, 0, 1 } },
			{ .position = { -Z, -X, N }, .color = { 1, 0, 0, 1 } }
		};
		AxArray<uint32> indices = {
			0, 1, 4,
			0, 4, 9,
			9, 4, 5,
			4, 8, 5,
			4, 1, 8,
			8, 1, 10,
			8, 10, 3,
			5, 8, 3,
			5, 3, 2,
			2, 3, 7,

			7, 3, 10,
			7, 10, 6,
			7, 6, 11,
			11, 6, 0,
			0, 6, 1,
			6, 10, 1,
			9, 11, 0,
			9, 2, 11,
			9, 5, 2,
			7, 11, 2
		};
		meshCpu.m_vertexBufferCPU.create(vertices);
		meshCpu.m_indexBufferCPU.create(indices);
		meshCpu.m_subMeshes.resize(2);
		meshCpu.m_subMeshes[0] = { 0, 30 };
		meshCpu.m_subMeshes[1] = { 30, 30 };

		m_meshes[0].create(renderer.getContext().m_device, &meshCpu, nullptr);
		g_meshes.emplace("Icosphere", &m_meshes[0]);
	}


	lua::LuaResult luaRes;

	lua_setup::RegisterEcs(m_lua);
	lua_setup::RegisterEcsComponents(m_lua);

	lua_setup::SetEcsRegistry(m_lua, m_registry);

	luaRes = m_lua.doString(R"(
Scene:SpawnEntity( "Icosphere", CreateTransform { position = { 0, 0, -10 }, rotation = { 34, 56, 78 } } )
)");
	if (lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Lua Error : {}", m_lua.getError());
		DEBUG_BREAK();
	}

	ipc::Initialize();
	g_serverSocket = ipc::CreateSocket({ ipc::IpcSocketType::eStream, ipc::IpcSocketProtocol::eTcp, true });
	g_serverSocket.Listen(9999);
}

void LuaEcsExample::update(float deltaTimeMs)
{
	if (!g_clientSocket.IsValid())
	{
		if (g_serverSocket.Accept(g_clientSocket))
		{
			axInfo("Connected to client session");
		}
	}
	else // g_clientSocket.IsValid()
	{
		uint32 result = g_clientSocket.Recv(make_array_ref(g_socketBuffer));
		if (result > 0)
		{
			axDebugFmt("Received string of {0} bytes : {1}", result, g_socketBuffer);

			lua::LuaResult luaRes = m_lua.doString(g_socketBuffer);
			if (lua::LuaResult::Ok != luaRes)
			{
				axAssertMsg(lua::LuaResult::Ok == luaRes, "Lua Error : {}", m_lua.getError());
			}
		}
	}


}

void LuaEcsExample::stop()
{
}
