#include "LuaSimpleExample.h"

#include "Apex/Application.h"
#include "Graphics/ForwardRenderer.h"

#define XCONCAT(a, b) a ## b
#define CONCAT(a, b) XCONCAT(a, b)

#define LUA_INTERNAL_FUNCTION(name) \
	static const char * const l_##name##_Name = XSTR(CONCAT(apex_,name)); \
	int l_##name(lua_State* L)

#define LUA_CFUNCTION(name) \
	static const char * const l_##name##_Name = XSTR(name); \
	int l_##name(lua_State* L)

namespace detail
{
	LUA_CFUNCTION(submitVertices)
	{
		using VertexArray = apex::AxArray<apex::gfx::Vertex_P0_C0>;
		VertexArray* pVertices = static_cast<VertexArray*>(lua_touserdata(L, lua_upvalueindex(1)));
		axAssertMsg(pVertices, "Invalid upvalue");
		VertexArray& vertices = *pVertices;

		size_t nverts = lua_rawlen(L, 1);

		vertices.reserve(nverts);

		for (size_t i = 1; i <= nverts; i++)
		{
			lua_pushnumber(L, i);
			lua_gettable(L, -2);

			if (lua_istable(L, -1))
			{
				apex::gfx::Vertex_P0_C0 vertex;
				vertex.color = { 1.f, 0.18f, 0.11f, 1.f };

				lua_pushnumber(L, 1);
				lua_gettable(L, -2);
				vertex.position.x = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushnumber(L, 2);
				lua_gettable(L, -2);
				vertex.position.y = (static_cast<float>(lua_tonumber(L, -1)));
				lua_pop(L, 1);

				lua_pushnumber(L, 3);
				lua_gettable(L, -2);
				vertex.position.z = (static_cast<float>(lua_tonumber(L, -1)));
				lua_pop(L, 1);

				lua_pop(L, 1);

				vertices.append(vertex);
			}
		}

		return 0;
	}

	LUA_CFUNCTION(submitIndices)
	{
		using IndexArray = apex::AxArray<apex::uint32>;
		IndexArray* pIndices = static_cast<IndexArray*>(lua_touserdata(L, lua_upvalueindex(1)));
		axAssertMsg(pIndices, "Invalid upvalue");
		IndexArray& indices = *pIndices;

		size_t nindices = lua_rawlen(L, 1);

		indices.reserve(nindices);

		for (size_t i = 1; i <= nindices; i++)
		{
			lua_pushnumber(L, i);
			lua_gettable(L, -2);

			if (lua_isnumber(L, -1))
			{
				indices.append(static_cast<apex::uint32>(lua_tonumber(L, -1)));
			}

			lua_pop(L, 1);
		}

		return 0;
	}

	LUA_INTERNAL_FUNCTION(updateCameraTransform)
	{
		apex::math::Matrix4x4* pTransform = static_cast<apex::math::Matrix4x4*>(lua_touserdata(L, lua_upvalueindex(1)));
		axAssertMsg(pTransform, "Invalid upvalue");
		apex::math::Matrix4x4& transform = *pTransform;

		transform = apex::math::Matrix4x4::identity();

		if (lua_istable(L, -1))
		{
			lua_pushstring(L, "translation");
			lua_gettable(L, -2);
			axAssertMsg(lua_istable(L, -1), "Invalid translation table");

			{
				lua_pushnumber(L, 1);
				lua_gettable(L, -2);
				transform[3][0] = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushnumber(L, 2);
				lua_gettable(L, -2);
				transform[3][1] = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushnumber(L, 3);
				lua_gettable(L, -2);
				transform[3][2] = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);
			}

			lua_pop(L, 1);

			lua_pushstring(L, "lookAtTarget");
			lua_gettable(L, -2);
			if (lua_istable(L, -1))
			{
				lua_pushnumber(L, 1);
				lua_gettable(L, -2);
				float x = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushnumber(L, 2);
				lua_gettable(L, -2);
				float y = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushnumber(L, 3);
				lua_gettable(L, -2);
				float z = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);

				transform = apex::math::lookAt(transform.getTranslation(), { x, y, z }, apex::math::Vector3::unitY());
			}
		}

		return 0;
	}

}

#define UNUSED_VAL(...) (void)(__VA_ARGS__)

void LuaSimpleExample::initialize()
{
	auto& renderer = *apex::Application::Instance()->getRenderer();

	renderer.setActiveCamera(&m_camera);

	m_cameraTransform = apex::math::translate(apex::math::Matrix4x4::identity(), { 0, 5, 5 });

	apex::lua::LuaResult luaRes;
	apex::AxArray<apex::gfx::Vertex_P0_C0> vertices;
	apex::AxArray<apex::uint32> indices;

	UNUSED_VAL(m_lua.createTable(0, 1));
	UNUSED_VAL(m_lua.pushLightUserData(&vertices));
	UNUSED_VAL(m_lua.registerCClosure(detail::l_submitVertices, 1, detail::l_submitVertices_Name));
	UNUSED_VAL(m_lua.pushLightUserData(&indices));
	UNUSED_VAL(m_lua.registerCClosure(detail::l_submitIndices, 1, detail::l_submitIndices_Name));
	UNUSED_VAL(m_lua.setGlobal("apex"));

	UNUSED_VAL(m_lua.pushLightUserData(&m_cameraTransform));
	UNUSED_VAL(m_lua.registerCClosureGlobal(detail::l_updateCameraTransform, 1, detail::l_updateCameraTransform_Name));

	luaRes = m_lua.loadFile(R"(X:\ApexGameEngine-Vulkan\Tests\Scripting\scripts\test_geometry.lua)");
	if (apex::lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Error loading test_geometry.lua\n[LUA_ERROR]: {0}\n", m_lua.getError());
		DEBUG_BREAK();
	}

	luaRes = m_lua.doFile(R"(X:\ApexGameEngine-Vulkan\Tests\Scripting\scripts\test_cameratransform.lua)");
	if (apex::lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Error loading test_camera.lua\n[LUA_ERROR]: {0}\n", m_lua.getError());
		DEBUG_BREAK();
	}

	lua_rawsetp(m_lua.raw(), LUA_REGISTRYINDEX, detail::l_updateCameraTransform_Name);

	luaRes = static_cast<apex::lua::LuaResult>(lua_pcall(m_lua.raw(), 0, 0, 0));
	if (apex::lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Error running test_geometry.lua\n[LUA_ERROR]: {0}\n", m_lua.getError());
		DEBUG_BREAK();
	}

	apex::gfx::MeshCPU meshCpu;
	meshCpu.m_vertexBufferCPU.create(vertices);
	meshCpu.m_indexBufferCPU.create(indices);
	m_mesh.create(renderer.getContext().m_device, &meshCpu, nullptr);

	renderer.getCurrentCommandList().getCommands().reserve(2);
}

void LuaSimpleExample::update(float deltaTimeMs)
{
	auto& renderer = *apex::Application::Instance()->getRenderer();
	auto& commandList = renderer.getCurrentCommandList();

	commandList.clear();

	lua_getglobal(m_lua.raw(), detail::l_updateCameraTransform_Name);
	axAssert(lua_iscfunction(m_lua.raw(), -1));
	
	lua_rawgetp(m_lua.raw(), LUA_REGISTRYINDEX, detail::l_updateCameraTransform_Name);
	auto luaRes = static_cast<apex::lua::LuaResult>(lua_pcall(m_lua.raw(), 0, 1, 0));
	if (apex::lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Error running test_camera.lua\n[LUA_ERROR]: {0}\n", m_lua.getError());
		DEBUG_BREAK();
	}
	luaRes = static_cast<apex::lua::LuaResult>(lua_pcall(m_lua.raw(), 1, 0, 0));
	if (apex::lua::LuaResult::Ok != luaRes)
	{
		axErrorFmt("Error calling apex_updateCameraTransform\n[LUA_ERROR]: {0}\n", m_lua.getError());
		DEBUG_BREAK();
	}

	m_camera.view = apex::math::lookAt(m_cameraTransform.getTranslation(), { 0, 0, 0 }, apex::math::Vector3::unitY());
	//m_camera.view = m_cameraTransform.inverse();
	m_camera.projection = apex::math::perspective(apex::math::radians(60.f), 16.f / 9.f, 0.1f, 100.f);
	m_camera.projection[1][1] *= -1;

	apex::gfx::DrawCommand drawCmd;

	drawCmd.pMesh = &m_mesh;
	drawCmd.transform = apex::math::Matrix4x4::identity();
	commandList.addCommand(drawCmd);

	commandList.sortCommands();
}

void LuaSimpleExample::stop()
{
	m_mesh.destroy(apex::Application::Instance()->getRenderer()->getContext().m_device, nullptr);
}

