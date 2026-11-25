#pragma once

#include "Factory.h"
#include "Containers/AxArray.h"
#include "String/AxHashString.h"

namespace apex {
namespace gfx {

namespace rendergraph {

	// Resource bits

	constexpr u32 kResourceIndexBits = 20;
	constexpr u32 kResourceVersionBits = 8;
	constexpr u32 kResourceUsageBits = 3; // gfx::ResourceUsage
	constexpr u32 kResourceTypeBits = 1; // gfx::rendergraph::Type
	
	static_assert((u32)ResourceUsage::MAX_COUNT <= (1 << kResourceUsageBits) - 1);

	constexpr u32 kResourceInvalidIndex = (1 << kResourceIndexBits) - 1;
	constexpr u32 kResourceMaxIndex = kResourceInvalidIndex - 1;
	
	static_assert(kResourceIndexBits + kResourceVersionBits + kResourceUsageBits + kResourceTypeBits == 32);

	// ResourceGraphPass bits
	constexpr u32 kPassIndexBits = 20;
	constexpr u32 kPassQueueBits = 2;	// gfx::QueueType

	struct ResourceHandle
	{
		u32 index : kResourceIndexBits;
		u32 _ : (32 - kResourceIndexBits);

		explicit constexpr ResourceHandle(u32 index) : index(index), _(0) {}
		explicit constexpr operator u32() const { return index; }
		constexpr bool IsValid() const { return index != INVALID_INDEX; }
	};

	using ResourceVersion = u8;

	enum ResourceAccess : u8 { eRead, eWrite, eReadWrite };

	enum ResourceType : u8 { eImage, eBuffer };

	struct ResourceVersionedHandle
	{
		u32 index : kResourceIndexBits;
		u32 version : kResourceVersionBits;
		u32 _ : (32 - kResourceIndexBits - kResourceVersionBits);

		//explicit constexpr ResourceVersionedHandle(ResourceHandle handle, ResourceVersion)
	};

	struct PassHandle
	{
		u32 index;

		explicit constexpr PassHandle(u32 index) : index(index) {}
		explicit constexpr operator u32() const { return index; }
		constexpr bool IsValid() const { return index != INVALID_INDEX; }
	};

	constexpr ResourceAccess GetAccessFromResourceUsageAndType(ResourceUsage usage, ResourceType type)
	{
		constexpr ResourceAccess resourceAccessTable[2][(u32)ResourceUsage::MAX_COUNT] = {
			// ResourceType::eImage
			{
				eRead, // TransferSrc,
				eWrite, // TransferDst,
				eRead, // SampledImage,
				eReadWrite, // StorageImage,
				eWrite, // ColorAttachment,
				eWrite, // DepthStencilAttachment,
				eReadWrite // DepthStencilReadOnly,
			},
			// ResourceType::eBuffer
			{
				eRead, // TransferSrc,
				eWrite, // TransferDst,
				eRead, // UniformBuffer,
				eReadWrite, // StorageBuffer,
				eRead, // VertexBuffer,
				eRead, // IndexBuffer,
				eRead, // _,
			}
		};
		return resourceAccessTable[type][(u32)usage];
	}

	constexpr ImageLayout GetImageLayoutFromResourceUsage(ResourceUsage usage)
	{
		constexpr ImageLayout imageLayoutTable[(u32)ResourceUsage::MAX_COUNT] = {
			ImageLayout::TransferSrcOptimal, // TransferSrc
			ImageLayout::TransferDstOptimal, // TransferDst
			ImageLayout::ShaderReadOnlyOptimal, // SampledImage
			ImageLayout::General, // StorageImage
			ImageLayout::ColorAttachmentOptimal, // ColorAttachment
			ImageLayout::DepthStencilAttachmentOptimal, // DepthStencilAttachment
			ImageLayout::DepthStencilReadOnlyOptimal, // DepthStencilReadOnly
		};
		return imageLayoutTable[(u32)usage];
	}

	class Resource
	{
	public:

		Resource(AxHashString id, ResourceHandle handle, ResourceType type)
			: m_id(id), m_handle(handle), m_type(type)
		{
		}

		AxHashString GetId() const { return m_id; }
		ResourceHandle GetHandle() const { return m_handle; }
		ResourceType GetType() const { return m_type; }
		
		u32 GetFirstPass() const { return m_firstPass; }
		u32 GetLastPass() const { return m_lastPass; }

		void SetFirstPass(u32 index) { m_firstPass = index; }
		void SetLastPass(u32 index) { m_lastPass = index; }

	private:
		AxHashString m_id;
		ResourceHandle m_handle;
		ResourceType m_type;
		
		u32 m_firstPass = INVALID_INDEX;
		u32 m_lastPass = INVALID_INDEX;
	};

	class ResourceUse
	{
	public:
		ResourceUse(ResourceHandle handle, ResourceVersion version, ResourceUsage usage, ResourceType type)
			: m_handle(handle), m_version(version), m_usage((u32)usage), m_type(type)
		{
		}

		ResourceHandle GetHandle() const { return (ResourceHandle)m_handle; }
		ResourceVersion GetVersion() const { return m_version; }
		ResourceUsage GetUsage() const { return static_cast<ResourceUsage>(m_usage); }
		ImageLayout GetLayout() const { return GetImageLayoutFromResourceUsage(static_cast<ResourceUsage>(m_usage)); }
		ResourceAccess GetAccess() const { return GetAccessFromResourceUsageAndType(static_cast<ResourceUsage>(m_usage), static_cast<ResourceType>(m_type)); }
		
		void SetVersion(ResourceVersion new_version) { m_version = new_version; }

	private:
		u32 m_handle : kResourceIndexBits;
		u32 m_version : kResourceVersionBits;
		u32 m_usage: kResourceUsageBits;
		u32 m_type : kResourceTypeBits;
		u32 _ : 1;
	};

	class Pass
	{
	public:
		Pass(AxHashString id, u32 index, QueueType queue)
			: m_id(id), m_index(index), m_queue(queue)
		{
		}

		u32 GetIndex() const { return m_index; }
		QueueType GetQueue() const { return static_cast<QueueType>(m_queue); }

		void SetResourceCount(u32 count)
		{
			m_resourceUsages.reserve(count);
		}

		void AddResourceUse(ResourceUse rsc)
		{
			m_resourceUsages.append(rsc);
		}

		AxSpan<ResourceUse> GetResources()
		{
			return make_span(m_resourceUsages);
		}

		AxSpan<const ResourceUse> GetResource() const
		{
			return make_span(m_resourceUsages);
		}

	private:
		AxHashString m_id;
		u32 m_index : kPassIndexBits;
		u32 m_queue : kPassQueueBits;
		AxArray<ResourceUse> m_resourceUsages;
	};

	struct Description
	{
		AxArray<Pass> passes;
		AxArray<Resource> resources;
		AxDenseHashMap<AxHashString, u32> passTable;
		AxDenseHashMap<AxHashString, u32> resourceTable;
	};

	class Builder
	{
	public:
		Builder(Description& desc) : m_desc(desc), m_currentPass(INVALID_INDEX) {}

		Builder& AddGraphicsPass(AxHashString id)
		{
			u32 index = m_desc.passes.size();
			auto [it, res] = m_desc.passTable.try_emplace(id, index);
			axAssertFmt(res, "Pass '{}' already added to the Render Graph at index {}.", it->first.GetStr(), it->second);
			m_desc.passes.emplace_back(id, index, QueueType::Graphics);
			m_currentPass = index;
			return *this;
		}

		Builder& AddComputePass(AxHashString id)
		{
			u32 index = m_desc.passes.size();
			auto [it, res] = m_desc.passTable.try_emplace(id, index);
			axAssertFmt(res, "Pass '{}' already added to the Render Graph at index {}.", it->first.GetStr(), it->second);
			m_desc.passes.emplace_back(id, index, QueueType::Compute);
			m_currentPass = index;
			return *this;
		}

		ResourceHandle GetOrCreateImage(AxHashString id)
		{
			u32 index = m_desc.resources.size();
			auto [it, res] = m_desc.resourceTable.try_emplace(id, index);
			if (res)
			{
				m_desc.resources.emplace_back(id, ResourceHandle(index), eImage);
			}
			return ResourceHandle(it->second);
		}

		ResourceHandle GetOrCreateBuffer(AxHashString id)
		{
			u32 index = m_desc.resources.size();
			auto [it, res] = m_desc.resourceTable.try_emplace(id, index);
			if (res)
			{
				m_desc.resources.emplace_back(id, ResourceHandle(index), eBuffer);
			}
			return ResourceHandle(it->second);
		}

		Builder& ColorAttachment(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::ColorAttachment, eImage);
		}

		Builder& ReadOnlyDepthAttachment(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::DepthStencilReadOnly, eImage);
		}

		Builder& DepthAttachment(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::DepthStencilAttachment, eImage);
		}

		Builder& SampledImage(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::SampledImage, eImage);
		}

		Builder& StorageImage(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::StorageImage, eImage);
		}
		
		Builder& UniformBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::UniformBuffer, eBuffer);
		}

		Builder& StorageBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::StorageBuffer, eBuffer);
		}

		Builder& VertexBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::VertexBuffer, eBuffer);
		}

		Builder& IndexBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::IndexBuffer, eBuffer);
		}

		Builder& TransferSrcImage(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::TransferSrc, eImage);
		}

		Builder& TransferDstImage(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::TransferDst, eImage);
		}

		Builder& TransferSrcBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::TransferSrc, eBuffer);
		}

		Builder& TransferDstBuffer(ResourceHandle handle)
		{
			return Access(handle, ResourceUsage::TransferDst, eBuffer);
		}

	protected:
		Builder& Access(ResourceHandle handle, ResourceUsage usage, ResourceType type)
		{
			axAssert(ValidateResourceHandle(handle));
			Pass& pass = m_desc.passes[m_currentPass];
			Resource& rsc = m_desc.resources[handle.index];
			axAssert(rsc.GetType() == type);
			const ResourceUse ref(handle, 0, usage, type);
			pass.AddResourceUse(ref);
			return *this;
		}

#if APEX_ENABLE_ASSERTS
		bool ValidateResourceHandle(ResourceHandle handle) const
		{
			const u32 index = handle.index;
			if (index >= m_desc.resources.size())
			{
				return false;
			}
			if (m_desc.resources[index].GetHandle().index != index)
			{
				return false;
			}
			if (m_desc.resourceTable.count(m_desc.resources[index].GetId()) == 0)
			{
				return false;
			}
			return true;
		}
#endif // RENDER_GRAPH_RESOURCE_VALIDATION

	private:
		u32 m_currentPass;
		Description& m_desc;
	};

	class Compiler
	{
	public:
		Compiler(Description& desc) : m_desc(desc) {}

		void Stage_GatherResourceVersions()
		{
			struct ResourceState { ResourceVersion latestVersion = 0; };
			ResourceState* resourceStates = apex_alloca_new(ResourceState, m_desc.resources.size());
			
			for (Pass& pass : m_desc.passes)
			{
				for (ResourceUse& use : pass.GetResources())
				{
					ResourceState& resourceState = resourceStates[(u32)use.GetHandle()];
					if (use.GetAccess() == eRead)
					{
						use.SetVersion(resourceState.latestVersion);
					}
					else
					{
						use.SetVersion(++resourceState.latestVersion);
					}
				}
			}
		}

		void Stage_BuildDependencyGraph()
		{
			m_passDependencies.resize(m_desc.passes.size());
			for (u32 passIdx = 0; passIdx < m_desc.passes.size(); passIdx++)
			{
				PassDependency& dep = m_passDependencies[passIdx];
				
			}
		}

	private:
		Description& m_desc;
		
		struct PassDependency
		{
			u16 predecessorBegin;
			u16 predecessorEnd;
			u16 successorStart;
			u16 successorEnd;
		};
		AxArray<PassDependency> m_passDependencies;

		struct ResourceVersionInfo
		{
			u32 handle : kResourceIndexBits;
		};
	};

}

	using RenderGraphBuilder = rendergraph::Builder;
	using RenderGraphDescription = rendergraph::Description;
	using RenderGraphCompiler = rendergraph::Compiler;

	class RenderGraph
	{
	public:

	private:

	};

	inline void TestRenderGraphBuilder_DepthPrePass(RenderGraphBuilder& builder)
	{
		rendergraph::ResourceHandle skinningBuffer = builder.GetOrCreateBuffer("SkinningBuffer"_hs);
		rendergraph::ResourceHandle cameraBuffer = builder.GetOrCreateBuffer("CameraBuffer"_hs);
		rendergraph::ResourceHandle vertexBuffer = builder.GetOrCreateBuffer("VertexBuffer"_hs);
		rendergraph::ResourceHandle depthAttachment = builder.GetOrCreateImage("DepthPrePassBuffer"_hs);
		builder.AddGraphicsPass("DepthPrePass"_hs)
		       .DepthAttachment(depthAttachment)
			   .StorageBuffer(vertexBuffer)
			   .StorageBuffer(skinningBuffer)
			   .UniformBuffer(cameraBuffer);
	}

	inline void TestRenderGraphBuilder_ShadowMapPass(RenderGraphBuilder& builder)
	{
		rendergraph::ResourceHandle skinningBuffer = builder.GetOrCreateBuffer("SkinningBuffer"_hs);
		rendergraph::ResourceHandle cameraBuffer = builder.GetOrCreateBuffer("CameraBuffer"_hs);
		rendergraph::ResourceHandle vertexBuffer = builder.GetOrCreateBuffer("VertexBuffer"_hs);
		rendergraph::ResourceHandle shadowCascadesBuffer = builder.GetOrCreateBuffer("ShadowCascadesBuffer"_hs);
		rendergraph::ResourceHandle shadowMapCascade0 = builder.GetOrCreateImage("ShadowMap_Cascade0"_hs);
		rendergraph::ResourceHandle shadowMapCascade1 = builder.GetOrCreateImage("ShadowMap_Cascade1"_hs);
		rendergraph::ResourceHandle shadowMapCascade2 = builder.GetOrCreateImage("ShadowMap_Cascade2"_hs);
		builder.AddGraphicsPass("ShadowMapPass_Cascade0"_hs)
			   .DepthAttachment(shadowMapCascade0)
			   .StorageBuffer(vertexBuffer)
			   .StorageBuffer(skinningBuffer)
			   .StorageBuffer(shadowCascadesBuffer)
			   .UniformBuffer(cameraBuffer);
		builder.AddGraphicsPass("ShadowMapPass_Cascade1"_hs)
			   .DepthAttachment(shadowMapCascade1)
			   .StorageBuffer(vertexBuffer)
			   .StorageBuffer(skinningBuffer)
			   .StorageBuffer(shadowCascadesBuffer)
			   .UniformBuffer(cameraBuffer);
		builder.AddGraphicsPass("ShadowMapPass_Cascade2"_hs)
			   .DepthAttachment(shadowMapCascade2)
			   .StorageBuffer(vertexBuffer)
			   .StorageBuffer(skinningBuffer)
			   .StorageBuffer(shadowCascadesBuffer)
			   .UniformBuffer(cameraBuffer);
	}

	inline void TestRenderGraphBuilder_GbufferPass(RenderGraphBuilder& builder)
	{
		rendergraph::ResourceHandle skinningBuffer = builder.GetOrCreateBuffer("SkinningBuffer"_hs);
		rendergraph::ResourceHandle cameraBuffer = builder.GetOrCreateBuffer("CameraBuffer"_hs);
		rendergraph::ResourceHandle vertexBuffer = builder.GetOrCreateBuffer("VertexBuffer"_hs);
		rendergraph::ResourceHandle depth = builder.GetOrCreateImage("DepthPrePassAttachment"_hs);
		rendergraph::ResourceHandle albedo = builder.GetOrCreateImage("Gbuffer_Albedo"_hs);
		rendergraph::ResourceHandle normal = builder.GetOrCreateImage("Gbuffer_Normal"_hs);
		rendergraph::ResourceHandle uv01 = builder.GetOrCreateImage("Gbuffer_UV01"_hs);
		rendergraph::ResourceHandle uv23 = builder.GetOrCreateImage("Gbuffer_UV23"_hs);
		rendergraph::ResourceHandle mro = builder.GetOrCreateImage("Gbuffer_MetRofOcc"_hs);
		builder.AddGraphicsPass("GbufferPass"_hs)
		       .DepthAttachment(depth)
			   .StorageBuffer(vertexBuffer)
			   .StorageBuffer(skinningBuffer)
			   .UniformBuffer(cameraBuffer)
			   .ColorAttachment(albedo)
			   .ColorAttachment(normal)
			   .ColorAttachment(uv01)
			   .ColorAttachment(uv23)
			   .ColorAttachment(mro);
	}

}
}
