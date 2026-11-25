#pragma once

#include "Graphics/GraphicsContext.h"
#include "Mount/Mount.h"
#include "String/AxHashString.h"

namespace apex {

typedef AxHashString AssetId;

template <typename Type>
struct AssetStore
{
	Type* TryGet(AssetId id)
	{
		auto it = m_data.find(id);
		if (it == m_data.end())
		{
			return nullptr;
		}
		return it->second;
	}

	Type* Get(AssetId id)
	{
		auto it = m_data.find(id);
		axCriticalAssertFmt(it != m_data.end() && it->second != nullptr, "Asset '{}' is not loaded!", id.GetStr());
		return it->second;
	}

	void Add(AssetId id, Type* asset)
	{
		m_data.emplace(id, asset);
	}

	AxDenseHashMap<AssetId, Type*> m_data;
};

template <typename Type>
struct AssetStoreInstance
{
	inline static AssetStore<Type>* m_store{ nullptr };

	static AssetStore<Type>* Get()
	{
		return m_store;
	}
	
	static void Set(AssetStore<Type>* store)
	{
		m_store = store;
	}
};

struct AssetStoreFactory
{
	template <typename Type>
	static void Register()
	{
		AssetStore<Type>* store = apex_new AssetStore<Type>;
		AssetStoreInstance<Type>::Set(store);
	}

	template <typename Type>
	static void Unregister()
	{
		AssetStore<Type>* store = AssetStoreInstance<Type>::Get();
		delete store;
		AssetStoreInstance<Type>::Set(nullptr);
	}

	template <typename Type>
	static AssetStore<Type>* Get()
	{
		return AssetStoreInstance<Type>::Get();
	}
};

template <typename Type>
struct AssetRef
{
	Type* TryGet()
	{
		AssetStore<Type>* store = AssetStoreFactory::Get<Type>();
		if (!axVerifyFmt(store, "Asset store not registered!"))
		{
			return nullptr;
		}
		return store->TryGet(id);
	}

	Type* Get()
	{
		AssetStore<Type>* store = AssetStoreFactory::Get<Type>();
		if (!axVerifyFmt(store, "Asset store not registered!"))
		{
			return nullptr;
		}
		return store->Get(id);
	}

	Type* operator->() { return Get(); }
	const Type* operator->() const { return TryGet(); }

	AssetId id;
};

#define PSO_STORE (*AssetStoreFactory::Get<gfx::GraphicsPipeline>())
#define MESH_STORE (*AssetStoreFactory::Get<gfx::Mesh>())
#define TEX_STORE (*AssetStoreFactory::Get<gfx::Image>())

inline gfx::GraphicsPipeline* LoadPipeline(AssetId id, const char* vertex_src, const char* fragment_src, gfx::VertexFormat const& vertexFormat, AxSpan<gfx::GraphicsPipelineAttachmentDesc> const& colorAttachments, gfx::Format depthFormat, gfx::DepthTestDesc const& depthTest)
{
	gfx::Context& ctx = gfx::Factory::GetContext();
	mnt::Path vertexShaderPath { vertex_src };
	mnt::Path fragmentShaderPath { fragment_src };
	AxStringView vertexShaderName = vertexShaderPath.GetFileBaseName();
	AxStringView fragmentShaderName = fragmentShaderPath.GetFileBaseName();
	char pathBuf[mnt::kMaxPathLength];
	gfx::ShaderModuleRef vertexShader, fragmentShader;
	if (mnt::MountManager::ResolveDevicePath(vertexShaderPath, pathBuf, sizeof(pathBuf)))
	{
		vertexShader = ctx.GetDevice()->CreateShaderModule(vertexShaderName.data(), pathBuf);
	}
	if (mnt::MountManager::ResolveDevicePath(fragmentShaderPath, pathBuf, sizeof(pathBuf)))
	{
		fragmentShader = ctx.GetDevice()->CreateShaderModule(fragmentShaderName.data(), pathBuf);
	}
	if (!vertexShader || !fragmentShader)
	{
		return nullptr;
	}
	const gfx::GraphicsPipelineCreateDesc pipelineCreateDesc {
		.shaderStages = {
			.vertexShader = vertexShader.Get(),
			.fragmentShader = fragmentShader.Get()
		},
		.topology = gfx::PrimitiveTopology::TriangleList,
		.vertexFormat = vertexFormat,
		.colorAttachments = colorAttachments,
		.depthAttachmentFormat = depthFormat,
		.depthTest = depthTest
	};

	gfx::GraphicsPipeline* pipeline = ctx.GetDevice()->CreateGraphicsPipeline(vertexShaderName.data(), pipelineCreateDesc);

	PSO_STORE.Add(id, pipeline);

	return pipeline;
}


}
