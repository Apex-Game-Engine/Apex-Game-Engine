#include <Tracy.hpp>

#include "Asset/Mesh.h"
#include "Core/Asserts.h"
#include "Core/Logging.h"
#include "Core/Files.h"
#include "Graphics/Camera.h"
#include "Graphics/Renderer.h"
#include "Graphics/AxImGui.h"
#include "Math/Matrix4x4.h"
#include "Math/Random.h"
#include "Mount/Mount.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"
#include "Platform/Timer.h"
#include "AssetStore.h"

#include <ranges>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x) axAssertFmt(x, "STB_IMAGE_ASSERT")
#include "stb_image.h"
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include "../../../Apex/Entity/include/Entity/Entity.h"
#include "Core/Socket.h"
#include "Graphics/RenderGraph.h"

#define nameof(type) (sizeof(::type), STR(type))

using namespace apex;

unsigned long RenderThreadFunc(void*)
{
	return 0;
}

struct GradientStop
{
	Color3f color;
	float position;
};

bool operator<(const GradientStop& left, const GradientStop& right)
{
	return left.position < right.position;
}

class ColorGradient
{
public:
	ColorGradient(AxArray<GradientStop> gradient_stops, math::Vector2 direction)
		: m_gradientStops(std::move(gradient_stops)), m_direction(std::move(direction))
	{
		std::sort(m_gradientStops.begin(), m_gradientStops.end());
	}

	Color3f Pick(float position)
	{
		const auto it = std::lower_bound(m_gradientStops.begin(), m_gradientStops.end(), GradientStop { .position = position });
		if (it == m_gradientStops.begin())
		{
			return it->color;
		}
		const auto prevIt = it - 1;
		const float t = (position - prevIt->position) / (it->position - prevIt->position);
		return math::lerp(prevIt->color, it->color, t);
	}

private:
	AxArray<GradientStop> m_gradientStops;
	math::Vector2 m_direction;
};

enum class LightType : u8
{
	ePoint,
	eDirectional,
	eSpot,
	// eTube,
	// eSphere,
	// eQuad,
};

struct alignas(16) LightParams
{
	union
	{
		math::Vector3 position;
		math::Vector3 direction;
	};
	float intensity;
	u8 r, g, b;
	LightType type;
};

static_assert(sizeof(LightParams) == 32);
static_assert(offsetof(LightParams, intensity) == 12);
static_assert(offsetof(LightParams, r) == 16);
static_assert(offsetof(LightParams, type) == 19);

static constexpr u32 LIGHT_BUFFER_INDEX = 1;
static constexpr u32 kMaxLights = 1024;

struct SceneNode
{
	ecs::EntityId entity;
	AxString name;
	SceneNode *parent;
	AxArray<SceneNode*> children;
};


///////////////////////////////////////////////////////////////////////////////////////
///               Global Variables
///////////////////////////////////////////////////////////////////////////////////////
static gfx::CleanupQueue cleanupQueue;
static gfx::Camera mainCamera;
static gfx::Buffer* cameraBuf = nullptr;
static gfx::Buffer* transformBuf = nullptr;
static gfx::Buffer* constantsBuf = nullptr;
static gfx::Buffer* lightsBuf = nullptr;
//static gfx::GBuffer gbuf;
static SceneNode *sceneRoot = nullptr;

template <typename T>
void SetValue(T& dest, const T& src)
{
	dest = src;
}

template <typename T>
void* SetValueAndAdvance(void* buf, const T& value)
{
	SetValue<T>(*(T*)buf, value);
	return (char*)buf + sizeof(T);
}

static gfx::CameraDesc mainCameraDesc = {
	.position = { 0, 0, 4 },
	.rotation = math::Quat::fromAxisAngle({1, 0, 0}, 0)
};

// Debug ImGui Textures
static ImTextureID imguiTextureId;
static float albedoOverride[3] = {0.0965f, 0.4216f, 0.5220f};

void RefreshLights()
{
	ZoneScoped;
	LightParams lights[8] = {
		/*{
			math::Vector3{ 1, -1, -1 },
			1000.0f,
			38, 46, 31,
			LightType::eDirectional
		},
		{
			math::Vector3{ -0.5, 0.5, 0.5 },
			1000.f,
			255, 255, 255,
			LightType::ePoint
		},
		{
			math::Vector3{ 0.5, 0.5, 0.5 },
			1000.f,
			0, 255, 0,
			LightType::ePoint
		}*/
	};
	for (u32 i = 0; i < std::size(lights); i++)
	{
		LightParams& light = lights[i];
		light.position = math::Random::randomUnitSphereVector3() + math::Vector3(0, 1, 1);
		light.r = math::Random::randomInt32(0, 0xFF);
		light.g = math::Random::randomInt32(0, 0xFF);
		light.b = math::Random::randomInt32(0, 0xFF);
		light.type = LightType::ePoint;
	}

	void* lightsGpuPointer = lightsBuf->GetMappedPointer();
	SetValueAndAdvance<u32>(lightsGpuPointer, std::size(lights));
	lightsGpuPointer = (char*)lightsGpuPointer + 4 * sizeof(u32);
	apex::memcpy_s<LightParams>(lightsGpuPointer, kMaxLights, lights, std::size(lights));
}

void DeleteSceneNode(SceneNode* node)
{
	for (auto& child : node->children)
	{
		DeleteSceneNode(child);
		child = nullptr;
	}
	// g_world->RemoveEntity(node->entity);
	delete node;
}

struct MeshArrays
{
	u32 vertexCount;
	f32* positionData {};
	f32* normalData {};
	f32* tangentData {};
	f32* bitangentData {};
	f32* texcoord0Data {};
	f32* texcoord1Data {};
	f32* texcoord2Data {};
	f32* texcoord3Data {};
	u16* boneIndices0Data {};
	f32* boneWeights0Data {};
	u16* boneIndices1Data {};
	f32* boneWeights2Data {};
	f32* colorData {};
};

void LoadGltfMesh(const tinygltf::Model& gltf_model, const tinygltf::Mesh& gltf_mesh, const gfx::VertexFormat& vertex_format)
{
	const tinygltf::Primitive& prim = gltf_mesh.primitives[0];

	MeshArrays meshArrays {};
	// POSITION
	if (!axVerifyFmt(prim.attributes.contains("POSITION"), "GLTF: Mesh {} primitive does not have POSITION attribute", gltf_mesh.name))
	{
		return;
	}
	const tinygltf::Accessor& posAccessor = gltf_model.accessors[prim.attributes.find("POSITION")->second];
	const tinygltf::BufferView& posView = gltf_model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = gltf_model.buffers[posView.buffer];
	meshArrays.vertexCount = (u32)posAccessor.count;
	meshArrays.positionData = (float*)(posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset);
	for (u32 i = 0; i < posAccessor.count; i++)
	{
		meshArrays.positionData[3*i+0] *= 0.01f;
		meshArrays.positionData[3*i+1] *= 0.01f;
		meshArrays.positionData[3*i+2] *= 0.01f;
	}
	// NORMAL
	if (prim.attributes.contains("NORMAL"))
	{
		const tinygltf::Accessor& normAccessor = gltf_model.accessors[prim.attributes.find("NORMAL")->second];
		const tinygltf::BufferView& normView = gltf_model.bufferViews[normAccessor.bufferView];
		const tinygltf::Buffer& normBuffer = gltf_model.buffers[normView.buffer];
		meshArrays.normalData = (float*)(normBuffer.data.data() + normView.byteOffset + normAccessor.byteOffset);
	}
	// TANGENT
	if (prim.attributes.contains("TANGENT"))
	{
		const tinygltf::Accessor& tanAccessor = gltf_model.accessors[prim.attributes.find("TANGENT")->second];
		const tinygltf::BufferView& tanView = gltf_model.bufferViews[tanAccessor.bufferView];
		const tinygltf::Buffer& tanBuffer = gltf_model.buffers[tanView.buffer];
		meshArrays.tangentData = (float*)(tanBuffer.data.data() + tanView.byteOffset + tanAccessor.byteOffset);
	}
	// BITANGENT
	if (prim.attributes.contains("BITANGENT"))
	{
		const tinygltf::Accessor& bitanAccessor = gltf_model.accessors[prim.attributes.find("BITANGENT")->second];
		const tinygltf::BufferView& bitanView = gltf_model.bufferViews[bitanAccessor.bufferView];
		const tinygltf::Buffer& bitanBuffer = gltf_model.buffers[bitanView.buffer];
		meshArrays.bitangentData = (float*)(bitanBuffer.data.data() + bitanView.byteOffset + bitanAccessor.byteOffset);
	}
	// TEXCOORD_0
	if (prim.attributes.contains("TEXCOORD_0"))
	{
		const tinygltf::Accessor& tc0Accessor = gltf_model.accessors[prim.attributes.find("TEXCOORD_0")->second];
		const tinygltf::BufferView& tc0View = gltf_model.bufferViews[tc0Accessor.bufferView];
		const tinygltf::Buffer& tc0Buffer = gltf_model.buffers[tc0View.buffer];
		meshArrays.texcoord0Data = (float*)(tc0Buffer.data.data() + tc0View.byteOffset + tc0Accessor.byteOffset);
	}
	// TEXCOORD_1
	if (prim.attributes.contains("TEXCOORD_1"))
	{
		const tinygltf::Accessor& tc1Accessor = gltf_model.accessors[prim.attributes.find("TEXCOORD_1")->second];
		const tinygltf::BufferView& tc1View = gltf_model.bufferViews[tc1Accessor.bufferView];
		const tinygltf::Buffer& tc1Buffer = gltf_model.buffers[tc1View.buffer];
		meshArrays.texcoord1Data = (float*)(tc1Buffer.data.data() + tc1View.byteOffset + tc1Accessor.byteOffset);
	}
	// COLOR_0
	if (prim.attributes.contains("COLOR_0"))
	{
		const tinygltf::Accessor& colorAccessor = gltf_model.accessors[prim.attributes.find("COLOR_0")->second];
		const tinygltf::BufferView& colorView = gltf_model.bufferViews[colorAccessor.bufferView];
		const tinygltf::Buffer& colorBuffer = gltf_model.buffers[colorView.buffer];
		meshArrays.colorData = (float*)(colorBuffer.data.data() + colorView.byteOffset + colorAccessor.byteOffset);
	}
	// JOINTS_0
	if (prim.attributes.contains("JOINTS_0"))
	{
		const tinygltf::Accessor& joints0Accessor = gltf_model.accessors[prim.attributes.find("JOINTS_0")->second];
		const tinygltf::BufferView& joints0View = gltf_model.bufferViews[joints0Accessor.bufferView];
		const tinygltf::Buffer& joints0Buffer = gltf_model.buffers[joints0View.buffer];
		if (joints0Accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
		{
			meshArrays.boneIndices0Data = (u16*)(joints0Buffer.data.data() + joints0View.byteOffset + joints0Accessor.byteOffset);
		}
		else if (joints0Accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		{
			const u32* src = (const u32*)(joints0Buffer.data.data() + joints0View.byteOffset + joints0Accessor.byteOffset);
			
		}
	}
	// JOINTS_1
	if (prim.attributes.contains("JOINTS_1"))
	{
		const tinygltf::Accessor& joints1Accessor = gltf_model.accessors[prim.attributes.find("JOINTS_1")->second];
		const tinygltf::BufferView& joints1View = gltf_model.bufferViews[joints1Accessor.bufferView];
		const tinygltf::Buffer& joints1Buffer = gltf_model.buffers[joints1View.buffer];
		if (joints1Accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
		{
			meshArrays.boneIndices1Data = (u16*)(joints1Buffer.data.data() + joints1View.byteOffset + joints1Accessor.byteOffset);
		}
	}
}

SceneNode* LoadGltfScene(AxStringView filepath)
{
	tinygltf::TinyGLTF gltf;
	tinygltf::Model gltfModel;
	std::string err;
	std::string warn;
	AxStringView ext = PathUtils::GetExtension(filepath);
	axAssert(ext == ".gltf");
	
	bool ret = gltf.LoadASCIIFromFile(&gltfModel, &err, &warn, std::string(filepath));

	axAssertFmt(err.empty(), "GLTF error: {}", err);
	if (!axVerifyFmt(ret, "GLTF error: {}", err))
	{
		return nullptr;
	}

	for (const auto& gltfMesh : gltfModel.meshes)
	{
		if (gltfMesh.primitives.empty())
		{
			continue;
		}
		gfx::VertexFormat vtxFmt;
		LoadGltfMesh(gltfModel, gltfMesh, vtxFmt);
	}

	AxArray<SceneNode*> sceneNodes;
	sceneNodes.resize(gltfModel.nodes.size());
	for (u32 nodeIdx = 0; nodeIdx < gltfModel.nodes.size(); nodeIdx++)
	{
		tinygltf::Node& gltfNode = gltfModel.nodes[nodeIdx];
		SceneNode* node = apex_new SceneNode;
		node->name = AxStringView(gltfNode.name);
		node->children.resize(gltfNode.children.size());
		for (u32 i = 0; i < gltfNode.children.size(); i++)
		{
			int childIdx = gltfNode.children[i];
			axAssert(childIdx < nodeIdx);
			node->children[i] = sceneNodes[childIdx];
		}
		sceneNodes[nodeIdx] = node;
	}
	
	auto& gltfScene = gltfModel.scenes[gltfModel.defaultScene];
	SceneNode* root = apex_new SceneNode;
	root->name = AxStringView{ gltfScene.name };
	root->children.resize(gltfScene.nodes.size());
	for (u32 i = 0; i < gltfScene.nodes.size(); i++)
	{
		root->children[i] = sceneNodes[gltfScene.nodes[i]];
	}

	return root;
}

#define INDENT_FMT "{:.{}}"
static const char *INDENT_STR = "                                                                ";

static void DescribeSceneNode(const SceneNode& node, int indent = 0)
{
	axAssert(indent < 32);
	if (indent == 0)
	{
		axInfoFmt("{}", node.name.c_str());
	}
	else
	{
		axInfoFmt(INDENT_FMT "- {}", INDENT_STR, indent * 2, node.name.c_str());
	}
	for (const SceneNode* pChild : node.children)
	{
		if (pChild)
		{
			DescribeSceneNode(*pChild, indent+1);
		}
	}
}

std::thread socketThread;
atomic_bool socketThreadRunning = true;

static void SocketThreadFunc()
{
	ipc::Socket::Init();

	axInfoFmt("Opening socket connection...");
	{
		ipc::Socket server = ipc::CreateSocket(9001);
		axVerify(server.Bind());
		server.Listen();

		char recvbuf[4096];
		
		while (socketThreadRunning)
		{
			ipc::Socket client = server.AcceptPoll(500);
			if (!client)
			{
				continue;
			}
			axInfoFmt("Connected to client @ {}", client.GetAddr().c_str());

			while (true)
			{
				int recvlen = client.Recv(recvbuf, sizeof(recvbuf)-1);
				if (recvlen <= 0)
				{
					axDebug("Connection closed");
					break;
				}
				recvbuf[recvlen] = 0;
				axDebugFmt("Received: {}", recvbuf);
			}

			client.Close();
		}
	}

	ipc::Socket::Shutdown();
}


void Setup(gfx::FrameData const& frameData)
{
	Dim2D frameDims;
	plat::PlatformManager::GetMainWindow().GetDimensions(&frameDims.width, &frameDims.height);

	AssetStoreFactory::Register<gfx::GraphicsPipeline>();
	AssetStoreFactory::Register<gfx::Mesh>();
	AssetStoreFactory::Register<gfx::Image>();

	socketThread = std::thread{ SocketThreadFunc };

	// Load meshes
	{
		ZoneScopedN("Load Meshes");
		{
			asset::ReadOnlyMeshData meshRef = asset::MeshLoader::ReadFromFile("X:/Tests/Assets/Meshes/exported/Alpha_Surfac.axmesh");
			MESH_STORE.Add("y_bot-surface"_hs, gfx::CreateMesh(meshRef.GetVertexFormat(), meshRef.GetVertexCount(), meshRef.GetVertices(), meshRef.GetIndexCount(), meshRef.GetIndices()));
		}
		{
			asset::ReadOnlyMeshData meshRef = asset::MeshLoader::ReadFromFile("X:/Tests/Assets/Meshes/exported/Alpha_Joint.axmesh");
			MESH_STORE.Add("y_bot-joints"_hs, gfx::CreateMesh(meshRef.GetVertexFormat(), meshRef.GetVertexCount(), meshRef.GetVertices(), meshRef.GetIndexCount(), meshRef.GetIndices()));
		}
	}

	{
		sceneRoot = LoadGltfScene(R"(X:\Tests\Assets\gltf\Untitled.gltf)");
		if (sceneRoot)
		{
			DescribeSceneNode(*sceneRoot);
		}
	}

	// Load pipelines
	{
		ZoneScopedN("Load Pipelines")
		{
			gfx::GraphicsPipelineAttachmentDesc colorAttachments[] = {
				{ gfx::GBuffer::GetFormat(gfx::GBuffer::eAlbedo) },
				{ gfx::GBuffer::GetFormat(gfx::GBuffer::eNormal) },
				{ gfx::GBuffer::GetFormat(gfx::GBuffer::eEmissive) },
			};
			gfx::VertexFormat vertexFormat {
				.vaPOSITION = {
					.location = 0,
					.stream = 0,
					.offset = 0,
					.format = (u32)gfx::Format::R32G32B32_SFLOAT,
				},
				.vaNORMAL = {
					.location = 2,
					.stream = 1,
					.offset = 0,
					.format = (u32)gfx::Format::R32G32B32_SFLOAT,
				},
				.vaTANGENT = {
					.location = 3,
					.stream = 1,
					.offset = 3 * sizeof(float),
					.format = (u32)gfx::Format::R32G32B32_SFLOAT,
				},
				.vaBITANGENT = {
					.location = 4,
					.stream = 1,
					.offset = 6 * sizeof(float),
					.format = (u32)gfx::Format::R32G32B32_SFLOAT,
				},
				.vaTEXCOORD0 = {
					.location = 1,
					.stream = 0,
					.offset = 3 * sizeof(float),
					.format = (u32)gfx::Format::R32G32_SFLOAT,
				},
			};

			(void)LoadPipeline(
					"Gbuffer.Phong.Static"_hs,
					"apex_root://Graphics/shaders/MSH_static.vert.spv",
					"apex_root://Graphics/shaders/GBUF_phong.frag.spv",
					vertexFormat,
					make_span(colorAttachments),
					gfx::GBuffer::GetFormat(gfx::GBuffer::eDepth),
					{ true, true });
		}
		{
			gfx::GraphicsPipelineAttachmentDesc colorAttachments[] = {
				{ gfx::Format::B8G8R8A8_SRGB },
			};
			gfx::VertexFormat emptyVertexFormat {};

			(void)LoadPipeline(
					"Lighting.Phong"_hs,
					"apex_root://Graphics/shaders/SCR_uv.vert.spv",
					"apex_root://Graphics/shaders/LIT_phong.frag.spv",
					emptyVertexFormat,
					make_span(colorAttachments),
					gfx::Format::UNDEFINED,
					{ false, false });
		}
		{
			/*gfx::GraphicsPipelineAttachmentDesc colosAttachments[] = {
				
			};*/

			gfx::RenderGraphDescription description;
			gfx::RenderGraphBuilder builder(description);
			gfx::TestRenderGraphBuilder_DepthPrePass(builder);
			gfx::TestRenderGraphBuilder_ShadowMapPass(builder);
			gfx::TestRenderGraphBuilder_GbufferPass(builder);
			gfx::RenderGraphCompiler compiler(description);
			compiler.Stage_GatherResourceVersions();
		}
	}

	// Create images
	{
		ZoneScopedN("Create Textures");
		{
			gfx::Image* checkered = gfx::Factory::CreateTexture2D("checkered", { 8, 8 });
			u32 data[] = {
				0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080,
				0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404,
				0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080,
				0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404,
				0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080,
				0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404,
				0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080,
				0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404, 0xff808080, 0xff040404,
			};
			gfx::Factory::EnqueueTextureUpload(checkered, { (byte*)data, sizeof(data) });
			gfx::Factory::GetContext().GetDevice()->BindSampledImage(checkered->GetView());
			TEX_STORE.Add("checkered"_hs, checkered);
		}

		{
			File file = File::OpenExisting("X:\\Tests\\Assets\\Meshes\\Alduin\\alduin.png");
			stbi_uc* imageFile = (stbi_uc*)file.MapRange(0, file.GetSize());
			int x, y, nc, len = (int)file.GetSize();
			stbi_uc* imageData = stbi_load_from_memory(imageFile, len, &x, &y, &nc, 4);
			gfx::Image* albedo = gfx::Factory::CreateTexture2D("alduin-albedo", Dim2D{ (u32)x, (u32)y }, gfx::Format::R8G8B8A8_SRGB);
			gfx::Factory::EnqueueTextureUpload(albedo, { imageData, x * y * nc * u32(sizeof(u8)) });
			gfx::Factory::GetContext().GetDevice()->BindSampledImage(albedo->GetView());
			TEX_STORE.Add("alduin-albedo"_hs, albedo);
		}

		{
			File file = File::OpenExisting("X:\\Tests\\Assets\\Meshes\\Alduin\\alduin_normal_1.png");
			stbi_uc* imageFile = (stbi_uc*)file.MapRange(0, file.GetSize());
			int x, y, nc, len = (int)file.GetSize();
			stbi_uc* imageData = stbi_load_from_memory(imageFile, len, &x, &y, &nc, 4);
			gfx::Image* normal = gfx::Factory::CreateTexture2D("alduin-normal", Dim2D{ (u32)x, (u32)y }, gfx::Format::R8G8B8A8_UNORM);
			gfx::Factory::EnqueueTextureUpload(normal, { imageData, x * y * nc * u32(sizeof(u8)) });
			gfx::Factory::BindSampledImage(normal->GetView());
			TEX_STORE.Add("alduin-normal"_hs, normal);
		}
	}

	{
		gbuf.Init(gfx::Factory::GetContext().GetDevice(), frameDims);
		for (u32 i = 0; i < gbuf.GetFrameCount(); i++)
		{
			gfx::Image* albedoImage = gbuf.GetImage_Unsafe(i, gfx::GBuffer::eAlbedo);
			gfx::Image* normalImage = gbuf.GetImage_Unsafe(i, gfx::GBuffer::eNormal);
			gfx::Image* emissiveImage = gbuf.GetImage_Unsafe(i, gfx::GBuffer::eEmissive);
			gfx::Image* depthImage = gbuf.GetImage_Unsafe(i, gfx::GBuffer::eDepth);

			gfx::Factory::BindSampledImage(albedoImage->GetView());
			gfx::Factory::BindSampledImage(normalImage->GetView());
			gfx::Factory::BindSampledImage(emissiveImage->GetView());
			gfx::Factory::BindSampledImage(depthImage->GetView());
		}
	}

	// Create buffers
	{
		ZoneScopedN("Create Buffers");
		constantsBuf = gfx::Factory::CreateUniformBuffer("Engine Constants Buffer", sizeof(float));
		cameraBuf = gfx::Factory::CreateUniformBuffer("Camera Buffer", sizeof(gfx::Camera::Matrices));
		transformBuf = gfx::Factory::CreateUniformBuffer("Transform Buffer", sizeof(math::Matrix4x4));
		lightsBuf = gfx::Factory::CreateStorageBuffer("Lights Buffer", kMaxLights * sizeof(LightParams) + sizeof(math::Vector4));
		cleanupQueue.Add(constantsBuf);
		cleanupQueue.Add(cameraBuf);
		cleanupQueue.Add(transformBuf);
		cleanupQueue.Add(lightsBuf);
		gfx::Factory::BindUniformBuffer(constantsBuf);
		gfx::Factory::BindUniformBuffer(cameraBuf);
		gfx::Factory::BindUniformBuffer(transformBuf);
		gfx::Factory::BindStorageBuffer(lightsBuf, LIGHT_BUFFER_INDEX);
	}

	// Setup Lights
	RefreshLights();

	gfx::Factory::FlushUploadBatch();
}

void MoveCamera(float deltaTime)
{
	math::Vector3 deltaPosition{};
	math::Quat targetRotation = mainCameraDesc.rotation;

	const auto& inputManager = plat::PlatformManager::GetInputManager();
	auto& gamepad = inputManager.GetGamepad(0);
	
	deltaPosition.x += gamepad.leftStick.x * deltaTime * 5;
	deltaPosition.z -= gamepad.leftStick.y * deltaTime * 5;
	targetRotation *= math::Quat::fromAxisAngle(math::Vector3::unitY(), gamepad.rightStick.x * deltaTime).normalized();
	targetRotation = math::Quat::fromAxisAngle(math::Vector3::unitX(), -gamepad.rightStick.y * deltaTime).normalized() * targetRotation;

	mainCameraDesc.rotation = targetRotation;
	mainCameraDesc.position += mainCameraDesc.rotation.applyToVector(deltaPosition);	
}

void Update(plat::Time time)
{
	MoveCamera(time.FrameTime);
	{
		mainCamera.UpdateMatrices(mainCameraDesc);
		memcpy(cameraBuf->GetMappedPointer(), &mainCamera, sizeof(mainCamera));
	}
	{
		const math::Quat rotation = math::Quat::fromAxisAngle({ 1, 0, 0 }, math::radians(90)) * math::Quat::fromAxisAngle({ 0, 1, 0 }, time.TotalTime / 5.f * math::radians(90));
		const math::Matrix4x4 identity = math::Matrix4x4::identity() * rotation.matrix();
		memcpy(transformBuf->GetMappedPointer(), &identity, sizeof(identity));
	}
}

void Draw(gfx::FrameData const& frameData)
{
	Dim2D dim = plat::PlatformManager::GetMainWindow().GetDimensions();
	gfx::CommandBuffer* cmdbuf = frameData.GetCommandBuffer();

	// GBuffer
	gbuf.Begin(cmdbuf);
	const gfx::Image* albedoImage = gbuf.GetImage(gfx::GBuffer::eAlbedo);
	const gfx::Image* normalImage = gbuf.GetImage(gfx::GBuffer::eNormal);
	const gfx::Image* emissiveImage = gbuf.GetImage(gfx::GBuffer::eEmissive);
	const gfx::Image* depthImage = gbuf.GetImage(gfx::GBuffer::eDepth);
	const gfx::ImageView* imageViews[] = { 
		albedoImage->GetView(),
		normalImage->GetView(),
		emissiveImage->GetView(),
	};

	cmdbuf->BeginRendering(apex::make_span(imageViews), depthImage->GetView(), gfx::LoadOp::Clear);
	cmdbuf->SetViewport({0, 0, (float)dim.width, (float)dim.height, 0.f, 1.f });
	cmdbuf->SetScissor({ 0, 0, dim.width, dim.height });

	const auto gbufferPipeline = PSO_STORE.Get("Gbuffer.Phong.Static"_hv);

	cmdbuf->BindGraphicsPipeline(gbufferPipeline);

	struct MaterialProperties
	{
		math::Vector4 albedoValue;
		math::Vector4 roughnessMetallicAoValue;
		math::Vector4 emissiveValue;
		uint32_t albedoTex;
		uint32_t normalTex;
		uint32_t roughnessMetallicAoTex;
		uint32_t emissiveTex;
	} pcMatProps;
	{
		pcMatProps.albedoTex = -1;// TEX_STORE.Get("alduin-albedo"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.normalTex = -1;// TEX_STORE.Get("alduin-normal"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.emissiveTex = -1;// TEX_STORE.Get("alduin-normal"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.albedoValue = math::Vector4(albedoOverride[0], albedoOverride[1], albedoOverride[2], 1.f);
		pcMatProps.emissiveValue = math::Vector4(1.f);
		cmdbuf->PushConstants(pcMatProps);

		const auto mesh = MESH_STORE.Get("y_bot-surface"_hv);
		gfx::VertexBuffer vbuf(mesh->GetVertexBuffer().Get(), mesh->GetVertexFormat(), mesh->GetVertexCount());
		axAssertFmt(gbufferPipeline->GetVertexFormatHash() == mesh->GetVertexFormat().CalcHash(), "VertexBuffer is incompatible with GraphicsPipeline!");
		cmdbuf->BindVertexBuffer(&vbuf);
		cmdbuf->BindIndexBuffer(mesh->GetIndexBuffer().Get());
		cmdbuf->DrawIndexed(mesh->GetIndexCount());
	}
	{
		pcMatProps.albedoTex = -1; //TEX_STORE.Get("checkered"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.normalTex = -1; //TEX_STORE.Get("alduin-normal"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.emissiveTex = -1; //TEX_STORE.Get("alduin-normal"_hs)->GetView()->GetGlobalDescriptorIndex();
		pcMatProps.albedoValue = Color4f(0.1535f, 0.2128f, 0.2222f, 1.f);
		pcMatProps.emissiveValue = math::Vector4(0.f);
		cmdbuf->PushConstants(pcMatProps);

		const auto mesh = MESH_STORE.Get("y_bot-joints"_hv);
		gfx::VertexBuffer vbuf(mesh->GetVertexBuffer().Get(), mesh->GetVertexFormat(), mesh->GetVertexCount());
		axAssertFmt(gbufferPipeline->GetVertexFormatHash() == mesh->GetVertexFormat().CalcHash(), "VertexBuffer is incompatible with GraphicsPipeline!");
		cmdbuf->BindVertexBuffer(&vbuf);
		cmdbuf->BindIndexBuffer(mesh->GetIndexBuffer().Get());
		cmdbuf->DrawIndexed(mesh->GetIndexCount());
	}

	cmdbuf->EndRendering();
	gbuf.End(cmdbuf);

	// Lighting
	cmdbuf->BeginRendering(frameData.GetBackBufferImage()->GetView(), nullptr, gfx::LoadOp::Clear);

	const auto lightingPipeline = PSO_STORE.Get("Lighting.Phong"_hv);
	cmdbuf->BindGraphicsPipeline(lightingPipeline);
	axAssert(albedoImage->GetView()->GetGlobalDescriptorBinding() == gfx::GlobalDescriptorBinding::SampledImage);
	axAssert(normalImage->GetView()->GetGlobalDescriptorBinding() == gfx::GlobalDescriptorBinding::SampledImage);
	axAssert(depthImage->GetView()->GetGlobalDescriptorBinding() == gfx::GlobalDescriptorBinding::SampledImage);
	struct GBuffer
	{
		uint32_t albedo, normal, emissive, depth;
	} pcGBuffer;
	pcGBuffer.albedo = albedoImage->GetView()->GetGlobalDescriptorIndex();
	pcGBuffer.normal = normalImage->GetView()->GetGlobalDescriptorIndex();
	pcGBuffer.emissive = emissiveImage->GetView()->GetGlobalDescriptorIndex();
	pcGBuffer.depth = depthImage->GetView()->GetGlobalDescriptorIndex();
	cmdbuf->PushConstants(pcGBuffer);
	cmdbuf->SetViewport({0, 0, (float)dim.width, (float)dim.height, 0.f, 1.f });
	cmdbuf->SetScissor({ 0, 0, dim.width, dim.height });
	cmdbuf->Draw(3);

	cmdbuf->EndRendering();
}

void Teardown()
{
	gbuf.Shutdown();
	AxImGui::RemoveTexture(imguiTextureId);
	for (const auto mesh : MESH_STORE.m_data | std::views::values)
	{
		delete mesh;
	}
	AssetStoreFactory::Unregister<gfx::Mesh>();
	for (const auto pipeline : PSO_STORE.m_data | std::views::values)
	{
		delete pipeline;
	}
	AssetStoreFactory::Unregister<gfx::GraphicsPipeline>();
	for (const auto image : TEX_STORE.m_data | std::views::values)
	{
		delete image;
	}
	AssetStoreFactory::Unregister<gfx::Image>();
	cleanupQueue.Cleanup();
	socketThreadRunning = false;
	socketThread.join();
}

void DebugDraw_MeshSelector(bool& show)
{
	static bool shouldLoadMesh = false;
	static char meshFilePath[mnt::kMaxPathLength];
	static asset::ReadOnlyMeshData* pMeshData = nullptr;
	ImGui::Begin("Mesh Selector");
	if (ImGui::InputTextWithHint("Mesh", "Path/to/file (*.axmesh)", meshFilePath, sizeof(meshFilePath), ImGuiInputTextFlags_ElideLeft | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		shouldLoadMesh = File::Exists(meshFilePath);
	}
	if (ImGui::Button("Browse..."))
	{
		shouldLoadMesh = plat::PlatformManager::ShowOpenFileDialog(meshFilePath, sizeof(meshFilePath)) || shouldLoadMesh;
	}
	
	ImGui::BeginDisabled(!shouldLoadMesh);
	if (ImGui::Button("Open"))
	{
		delete pMeshData;
		asset::ReadOnlyMeshData meshData = asset::MeshLoader::ReadFromFile(meshFilePath);
		pMeshData = apex_new asset::ReadOnlyMeshData(std::move(meshData));
	}
	ImGui::EndDisabled();

	if (pMeshData)
	{
		static char buf[512];
		ImGui::Text("Mesh Details:");
		ImGui::Text("Vertex count: %u", pMeshData->GetVertexCount());
		ImGui::Text("Index count: %u", pMeshData->GetIndexCount());
		ImGui::Text("Vertex Format: %s", pMeshData->GetVertexFormat().ToString(buf, sizeof(buf)));
	}
	ImGui::End();
}

void DebugDraw_MaterialEditor(bool& show)
{
	// ImGui::Checkbox("Disable Albedo", (bool*)&options.disableAlbedo);
	// ImGui::Checkbox("Disable Normal", (bool*)&options.disableNormal);

	ImGui::ColorPicker3("Albedo", albedoOverride);
}

void DebugDraw(gfx::FrameData const& frameData)
{
	static bool bShowMeshSelector = false;
	static bool bShowMaterialEditor = false;

	if (ImGui::Button("Refresh Lights"))
	{
		RefreshLights();
	}

	DebugDraw_MeshSelector(bShowMeshSelector);
	DebugDraw_MaterialEditor(bShowMaterialEditor);

	// ImGui::Begin("Image");
	if (imguiTextureId)
	{
		ImGui::Image(imguiTextureId, { 400, 400 });
	}
	// ImGui::End();
}
