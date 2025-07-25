﻿#pragma once

#include <type_traits>

#include "Containers/AxArray.h"
#include "Core/Types.h"

namespace apex {
namespace gfx {
	class CommandBuffer;
	class Fence;
	class ShaderModule;

	enum class MemoryPropertyFlagBits : u16
	{
		None            = 0x0000,
		DeviceLocal     = 0x0001,
		HostVisible     = 0x0002,
		HostCoherent    = 0x0004,
		HostCached      = 0x0008,
		LazilyAllocated = 0x0010,
	};
	using MemoryPropertyFlags = Flags<MemoryPropertyFlagBits>;
	DEFINE_BITWISE_OPERATORS(MemoryPropertyFlagBits, MemoryPropertyFlags);

	enum class MemoryAllocateFlagBits : u16
	{
		None                       = 0x0000,
		AllocateDedicated          = 0x0001,
		NeverAllocate              = 0x0002,
		CanAlias                   = 0x0004,
		HostAccessSequential       = 0x0010,
		HostAccessRandom           = 0x0020,
		HostAccessFallbackTransfer = 0x0040,
	};
	using MemoryAllocateFlags = Flags<MemoryAllocateFlagBits>;
	DEFINE_BITWISE_OPERATORS(MemoryAllocateFlagBits, MemoryAllocateFlags);

	enum class BufferUsageFlagBits : u16
	{
		Vertex      = 0x0001,
		Index       = 0x0002,
		Uniform     = 0x0004,
		Storage     = 0x0008,
		Indirect    = 0x0010,
		TransferSrc = 0x0020,
		TransferDst = 0x0040,
	};
	using BufferUsageFlags = Flags<BufferUsageFlagBits>;
	DEFINE_BITWISE_OPERATORS(BufferUsageFlagBits, BufferUsageFlags);

	enum class ImageUsageFlagBits : u16
	{
		TransferSrc = 0x00000001,
		TransferDst = 0x00000002,
		Sampled = 0x00000004,
		Storage = 0x00000008,
		ColorAttachment = 0x00000010,
		DepthStencilAttachment = 0x00000020,
		TransientAttachment = 0x00000040,
		InputAttachment = 0x00000080,
	};
	using ImageUsageFlags = Flags<ImageUsageFlagBits>;
	DEFINE_BITWISE_OPERATORS(ImageUsageFlagBits, ImageUsageFlags);

	enum class ImageLayout : u32
	{
		Undefined = 0,
	    General = 1,
	    ColorAttachmentOptimal = 2,
	    DepthStencilAttachmentOptimal = 3,
	    DepthStencilReadOnlyOptimal = 4,
	    ShaderReadOnlyOptimal = 5,
	    TransferSrcOptimal = 6,
	    TransferDstOptimal = 7,
		PresentSrcOptimal = 8,
	};

	enum class ImageType : u32
	{
		Image1D,
		Image2D,
		Image3D,
		ImageCube,
		Image1DArray,
		Image2DArray,
		ImageCubeArray,
	};

	enum class ImageFormat : u32
	{
		UNDEFINED,
		R8_UNORM,
		R8_SNORM,
		R8_UINT,
		R8_SINT,
		R8_SRGB,
		R8G8_UNORM,
		R8G8_SNORM,
		R8G8_UINT,
		R8G8_SINT,
		R8G8_SRGB,
		R8G8B8_UNORM,
		R8G8B8_SNORM,
		R8G8B8_UINT,
		R8G8B8_SINT,
		R8G8B8_SRGB,
		B8G8R8_UNORM,
		B8G8R8_SNORM,
		B8G8R8_UINT,
		B8G8R8_SINT,
		B8G8R8_SRGB,
		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R8G8B8A8_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_SRGB,
		B8G8R8A8_UNORM,
		B8G8R8A8_SNORM,
		B8G8R8A8_UINT,
		B8G8R8A8_SINT,
		B8G8R8A8_SRGB,

		R16_FLOAT,
		R32_FLOAT,
		
		D16_UNORM,
	    D32_FLOAT,
	    S8_UINT,
	    D16_UNORM_S8_UINT,
	    D24_UNORM_S8_UINT,
	};

	enum class AccessFlagBits : u32
	{
	    None                        = 0,
		IndirectCommandRead         = 0x00000001,
	    IndexRead                   = 0x00000002,
	    VertexAttributeRead         = 0x00000004,
	    UniformRead                 = 0x00000008,
	    InputAttachmentRead         = 0x00000010,
	    ShaderRead                  = 0x00000020,
	    ShaderWrite                 = 0x00000040,
	    ColorAttachmentRead         = 0x00000080,
	    ColorAttachmentWrite        = 0x00000100,
	    DepthStencilAttachmentRead  = 0x00000200,
	    DepthStencilAttachmentWrite = 0x00000400,
	    TransferRead                = 0x00000800,
	    TransferWrite               = 0x00001000,
	    HostRead                    = 0x00002000,
	    HostWrite                   = 0x00004000,
	    MemoryRead                  = 0x00008000,
	    MemoryWrite                 = 0x00010000,
	};
	using AccessFlags = Flags<AccessFlagBits>;
	DEFINE_BITWISE_OPERATORS(AccessFlagBits, AccessFlags);

	enum class PipelineStageFlagBits : u32
	{
	    None                         = 0,
		TopOfPipe                    = 0x00000001,
	    DrawIndirect                 = 0x00000002,
	    VertexInput                  = 0x00000004,
	    VertexShader                 = 0x00000008,
	    TessellationControlShader    = 0x00000010,
	    TessellationEvaluationShader = 0x00000020,
	    GeometryShader               = 0x00000040,
	    FragmentShader               = 0x00000080,
	    EarlyFragmentTests           = 0x00000100,
	    LateFragmentTests            = 0x00000200,
	    ColorAttachmentOutput        = 0x00000400,
	    ComputeShader                = 0x00000800,
	    Transfer                     = 0x00001000,
	    BottomOfPipe                 = 0x00002000,
	    Host                         = 0x00004000,
	    AllGraphics                  = 0x00008000,
	    AllCommands                  = 0x00010000,
	};
	using PipelineStageFlags = Flags<PipelineStageFlagBits>;
	DEFINE_BITWISE_OPERATORS(PipelineStageFlagBits, PipelineStageFlags);

	enum class PrimitiveTopology : u8
	{
		PointLists,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		LineListWithAdjacency,
		LineStripWithAdjacency,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency,
		PatchList,
	};

	struct QueueType
	{
		enum Value {
			Graphics,
			Compute,
			Transfer,
			COUNT
		};

		Value value;

		constexpr QueueType(Value val) : value(val) {}
		constexpr QueueType(u32 val) : value(static_cast<Value>(val)) {}

		constexpr operator Value() const { return value; }
		constexpr bool operator==(Value val) const { return value == val; }
	};

	struct BindlessDescriptorType
	{
		enum Value : u32
		{
			SampledImage,
			StorageImage,
			UniformBuffer,
			StorageBuffer,
			Sampler,

			COUNT
		} value;

		constexpr BindlessDescriptorType(Value val) : value(val) {}

		constexpr operator u32() const { return value; }
	};

	enum class DescriptorType : u8
	{
		eInputAttachment,
		eSampler,
		eCombinedImageSampler,
		eSampledImage,
		eStorageImage,
		eUniformBuffer,
		eStorageBuffer,

		// HLSL descriptor types
		eShaderResourceView = eStorageImage,
		eConstantBufferView = eUniformBuffer,
		eUnorderedAccessView = eStorageBuffer,
		eTexture = eSampledImage,
	};

	struct Viewport
	{
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

	struct Rect2D
	{
		s32 x;
		s32 y;
		u32 width;
		u32 height;
	};

	struct Dim2D
	{
		union { u32 width = 1;	u32 x; };
		union { u32 height = 1;	u32 y; };
	};

	struct Dim3D
	{
		union { u32 width = 1;	u32 x;};
		union { u32 height = 1;	u32 y;};
		union { u32 depth = 1;	u32 z;};
	};

	struct CommandBufferCreateDesc;

	struct BufferCreateDesc
	{
		size_t              size;
		BufferUsageFlags    usageFlags;
		MemoryPropertyFlags requiredFlags;
		MemoryPropertyFlags preferredFlags;
		MemoryAllocateFlags memoryFlags;
		QueueType			ownerQueue;
		bool                createMapped;
		u32                 alignment;
		const void*         pInitialData;
	};

	struct ImageCreateDesc
	{
		ImageType           imageType;
		ImageFormat         format;
		ImageUsageFlags     usageFlags;
		Dim3D               dimensions;
		MemoryPropertyFlags requiredFlags;
		MemoryPropertyFlags preferredFlags;
		MemoryAllocateFlags memoryFlags;
		QueueType			ownerQueue;
		bool                createMapped;
		const void*         pInitialData;
	};

	struct ImageViewCreateDesc
	{
		
	};

	struct GraphicsPipelineShaderStagesDesc
	{
		ShaderModule* vertexShader;
		ShaderModule* fragmentShader;
		// TODO: Add other shader stages as required
	};

	struct GraphicsPipelineCreateDesc
	{
		GraphicsPipelineShaderStagesDesc shaderStages;
		PrimitiveTopology                topology { PrimitiveTopology::TriangleList };
	};

	struct ComputePipelineCreateDesc
	{
		ShaderModule* computeShader;
	};

	struct QueueSubmitDesc
	{
		AxArrayRef<CommandBuffer*>			commandBuffers;
		Fence*								fence;
		PipelineStageFlags					fenceWaitStageMask;
		u64									fenceWaitValue;
		u64									fenceSignalValue;
		bool								waitImageAcquired;
		PipelineStageFlags					imageAcquiredWaitStageMask;
		bool								signalRenderComplete;
	};

} // namespace gfx
} // namespace apex
