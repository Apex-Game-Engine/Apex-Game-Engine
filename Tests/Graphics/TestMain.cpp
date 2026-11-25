#define NOMINMAX
#include <Windows.h>
#include <Xinput.h>

#include "Containers/AxArray.h"
#include "Core/Console.h"
#include "Core/Logging.h"
#include "Core/Files.h"
#include "Graphics/Factory.h"
#include "Graphics/GraphicsContext.h"
#include "Memory/MemoryManager.h"
#include "Math/Vector3.h"

#include <charconv>

#include "Asset/Mesh.h"
#include "Graphics/DebugRenderer.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"
#include "Platform/Timer.h"
#include "Graphics/AxImGui.h"

#include "imgui.h"
#include "Asset/Pipeline.h"
#include "Containers/AxHashMap.h"
#include "Memory/UniquePtr.h"
#include "String/AxHashString.h"

#include "tracy/Tracy.hpp"

#define WNDCLASSNAME "Graphics_Test"
#define APPNAME      "ApexGraphics-Test"


bool g_running = true;
bool g_resized = false;

float clamp(float val, float min_val, float max_val)
{
	return val < min_val ? min_val : val > max_val ? max_val : val;
}

/*void ProcessGamepadRumble(float rumble_left, float rumble_right)
{
	rumble_left = clamp(rumble_left, 0.0f, 1.0f);
	rumble_right = clamp(rumble_right, 0.0f, 1.0f);

	XINPUT_VIBRATION vibration {
		.wLeftMotorSpeed = static_cast<WORD>(rumble_left * 65535u),
		.wRightMotorSpeed = static_cast<WORD>(rumble_right * 65535u),
	};

	DWORD dwResult = XInputSetState(0, &vibration);
	axAssertFmt(ERROR_SUCCESS == dwResult, "Failed to set gamepad vibration : {}", dwResult);
}*/

struct AABB
{
	apex::math::Vector3 bmin { +apex::Constants::f32_MAX }, bmax { -apex::Constants::f32_MAX };
	apex::math::Vector3 GetCenter() const { return (bmin + bmax) / 2.f; }
	apex::math::Vector3 GetExtent() const { return bmax - bmin; }
};
struct Frustum
{
	apex::math::Vector4 planes[6];
	//apex::math::Vector3 points[8];
};

AABB operator*(apex::math::Matrix4x4 const& m, AABB const& aabb)
{
	ZoneScoped;
	apex::math::Vector3 bmin(apex::Constants::f32_MAX);
	apex::math::Vector3 bmax(-apex::Constants::f32_MAX);

	for (int i = 0; i < 3; i++)
	{
		ZoneScopedN("AABB Transform - 1st for loop");
		bmin[i] = bmax[i] = m[3][i];
		for (int j = 0; j < 3; j++)
		{
			ZoneScopedN("AABB Transform - 2nd for loop");
			float e = m[i][j] * aabb.bmin[j];
			float f = m[i][j] * aabb.bmax[j];
			if (e < f)
			{
				bmin[i] += e;
				bmax[i] += f;
			}
			else
			{
				bmin[i] += f;
				bmax[i] += e;
			}
		}
	}

	return { bmin, bmax };
}

// This function assumes that the position is the first attribute in the first stream of the mesh
AABB CalculateBoundingBox(apex::asset::ReadOnlyMeshData const& mesh)
{
	AABB bbox;
	const apex::u32 strideInBytes = mesh.GetVertexFormat().CalcVertexStride();
	float* vertices = static_cast<float*>(mesh.GetVertices());
	apex::u32 stride = strideInBytes / sizeof(float);
	for (apex::u32 i = 0; i < mesh.GetVertexCount(); i++)
	{
		const apex::u32 idx = stride * i;
		float x = vertices[idx];
		float y = vertices[idx + 1];
		float z = vertices[idx + 2];
		apex::math::Vector3 vertex { x, y, z };

		bbox.bmin = min(bbox.bmin, vertex);
		bbox.bmax = max(bbox.bmax, vertex);
	}
	return bbox;
}

bool CheckFrustumVisibility(Frustum const& frustum, AABB const& bbox)
{
	apex::math::Vector4 corners[] = {
		{ bbox.bmin.x, bbox.bmin.y, bbox.bmin.z, 1.0f },
		{ bbox.bmin.x, bbox.bmin.y, bbox.bmax.z, 1.0f },
		{ bbox.bmin.x, bbox.bmax.y, bbox.bmin.z, 1.0f },
		{ bbox.bmin.x, bbox.bmax.y, bbox.bmax.z, 1.0f },
		{ bbox.bmax.x, bbox.bmin.y, bbox.bmin.z, 1.0f },
		{ bbox.bmax.x, bbox.bmin.y, bbox.bmax.z, 1.0f },
		{ bbox.bmax.x, bbox.bmax.y, bbox.bmin.z, 1.0f },
		{ bbox.bmax.x, bbox.bmax.y, bbox.bmax.z, 1.0f },
	};

	bool visible = false;
	for (int i = 0; i < 6; i++)
	{
		for (int c = 0; c < 8; c++)
		{
			visible = visible || (dot(frustum.planes[i], corners[c]) > 0.0f);
		}
		if (!visible) return false;
	}
	return true;
}

struct Camera
{
	apex::math::Vector3 position;
	apex::math::Quat rotation;
	float fov = apex::math::radians(60.f);
	float nearZ = 0.1f;
	float farZ = 10000.f;
};

struct CameraMatrices
{
	apex::math::Matrix4x4 view;
	apex::math::Matrix4x4 projection;
	apex::math::Matrix4x4 inverseView;
};

void UpdateCamera(CameraMatrices &camera_data, apex::Dim2D dims, Camera const& camera)
{
	apex::math::Matrix4x4 cameraTransform = camera.rotation.matrix();
	cameraTransform.setTranslation(camera.position);
	camera_data.inverseView = cameraTransform;
	camera_data.view = cameraTransform.inverse();
	// Can precompute the projection matrix. But its okay for the demo.
	camera_data.projection = apex::math::perspective(camera.fov, static_cast<float>(dims.width) / static_cast<float>(dims.height), camera.farZ, camera.nearZ);
	camera_data.projection[1][1] *= -1;
}

void MakeViewSpaceFrustum(Frustum& frustum, CameraMatrices const& camera)
{
	apex::math::Matrix4x4 VP = (camera.projection * camera.view).transpose();
	frustum.planes[0] = VP[3] + VP[0];
	frustum.planes[1] = VP[3] - VP[0];
	frustum.planes[2] = VP[3] + VP[1];
	frustum.planes[3] = VP[3] - VP[1];
	frustum.planes[4] = VP[3] + VP[2];
	frustum.planes[5] = VP[3] - VP[2];

	// Normalize the planes
	for (int i = 0; i < 6; i++)
	{
		frustum.planes[i] /= frustum.planes[i].xyz().length();
	}
}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
int main(int argc, char* argv[])
{
	using namespace apex;

	logs::Logger::Init(true);

	/*Console console(APPNAME);
	logging::Logger::AttachConsole(&console);*/

	mem::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });

	plat::PlatformManager::Init({ 1366, 768, "Apex Platform Test" });
	plat::PlatformWindow& mainWindow = plat::PlatformManager::GetMainWindow();
	mainWindow.Show();
	gfx::Factory::InitContext(gfx::Api::Vulkan, mainWindow);
	{		
		gfx::Context& gfx = gfx::Factory::GetContext();

		// Setup ImGui
		AxImGui::Init(mainWindow, gfx);

		// Rendering code
		{
			// MeshCPU meshCpu = LoadMesh(R"(X:\ApexGameEngine-Vulkan\Templates\Assets\cube.axmesh)");

			// ObjLoader objLoader(R"(X:\ApexGameEngine-Vulkan\Templates\Assets\FinalBaseMesh.obj)");
			// MeshCPU meshCpu { .vertices = objLoader.GetVertexBufferData(), .indices = objLoader.GetIndexBufferData() };

			asset::MeshExporterFlags meshExporterFlags = { asset::MeshExporterFlagBits::Triangulate | asset::MeshExporterFlagBits::CalculateNormals };
			asset::ReadOnlyMeshData meshData = asset::MeshLoader::ReadFromFile(R"(X:\Tests\Assets\Meshes\exported\Alpha_Surfac.axmesh)", meshExporterFlags);
			AABB aabb = CalculateBoundingBox(meshData);

			// Create mesh buffers
			auto verts = meshData.GetVertices();
			// MeshCPU meshCpu { .vertices = { (float*)verts.data, verts.count * verts.stride / sizeof(float) }, .indices = mesh.GetIndices() };
			// MeshGPU meshGpu = UploadMeshToGpu(gfx, meshCpu);
			gfx::Mesh* mesh = gfx::Mesh::Create(meshData.GetVertexFormat(), meshData.GetVertexCount(), meshData.GetVertices(), meshData.GetIndexCount(), meshData.GetIndices());

			// Create uniform buffers
			gfx::Buffer* cameraBuffer = gfx::Factory::CreateUniformBuffer("CameraMatrices Uniform Buffer", sizeof(CameraMatrices));
			gfx::Buffer* transformBuffer = gfx::Factory::CreateStorageBuffer("Model Transforms Buffer", sizeof(math::Matrix4x4) * 1000);

			gfx::Image* depthImages[3] = {
				gfx::Factory::CreateDepthImage("Depth #0", gfx.GetDevice()->GetSurfaceDim()),
				gfx::Factory::CreateDepthImage("Depth #1", gfx.GetDevice()->GetSurfaceDim()),
				gfx::Factory::CreateDepthImage("Depth #2", gfx.GetDevice()->GetSurfaceDim()),
			};

			gfx::Image* texture = gfx::Factory::CreateTexture2D("Texture", {2, 2});

			u32 visible = 0;
			AxArray<math::Vector3> positions(1000); positions.resize(1000);
			{
				u32 textureData[4] = { 0, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
				size_t textureDataSize = sizeof(textureData);
				gfx::Buffer* stagingTexture = gfx::Factory::CreateStagingBuffer("Staging-Texture", textureDataSize);
				memcpy(stagingTexture->GetMappedPointer(), textureData, textureDataSize);

				AxSpan transforms { (math::Matrix4x4*)transformBuffer->GetMappedPointer(), 1000 };
				for (int i = 0; i < 10; i++) for (int j = 0; j < 10; j++) for (int k = 0; k < 10; k++)
				{
					int idx = i * 100 + j * 10 + k;
					math::Vector3 position = { ((float)i / 10.f - 0.5f) * 40.f, ((float)j / 10.f - 0.5f) * 40.f, ((float)k / 10.f) * 40.f };
					positions[idx] = position;
					transforms[idx] = translate(math::Matrix4x4::identity(), position);
				}

				if (auto autoSubmit = gfx::AutoSubmitCommandBuffer(graphicsQueue->AllocateCommandBuffer(), graphicsQueue))
				{
					gfx::CommandBuffer* commands = autoSubmit.GetCommandBuffer();
					commands->Begin();
					for (gfx::Image* depthImage : depthImages) {
						commands->TransitionImage(depthImage,
						                          gfx::ImageLayout::Undefined,						gfx::PipelineStageFlagBits::None,				gfx::AccessFlagBits::None,							gfx::QueueType::Graphics,
						                          gfx::ImageLayout::DepthStencilAttachmentOptimal,	gfx::PipelineStageFlagBits::EarlyFragmentTests, gfx::AccessFlagBits::DepthStencilAttachmentWrite,	gfx::QueueType::Graphics);
					}
					//commands->TransitionImage(texture,
					//                          gfx::ImageLayout::Undefined,			gfx::PipelineStageFlagBits::TopOfPipe,	gfx::AccessFlagBits::None,			gfx::QueueType::Graphics,
					//                          gfx::ImageLayout::TransferDstOptimal, gfx::PipelineStageFlagBits::Transfer,	gfx::AccessFlagBits::TransferWrite, gfx::QueueType::Graphics);
					//
					//commands->CopyBufferToImage(stagingTexture, texture, gfx::ImageLayout::TransferDstOptimal);
					//
					//commands->TransitionImage(texture,
					//                          gfx::ImageLayout::TransferDstOptimal,		gfx::PipelineStageFlagBits::Transfer,		gfx::AccessFlagBits::TransferWrite, gfx::QueueType::Graphics,
					//                          gfx::ImageLayout::ShaderReadOnlyOptimal,	gfx::PipelineStageFlagBits::FragmentShader, gfx::AccessFlagBits::ShaderRead,	gfx::QueueType::Graphics);
					commands->End();
				}

				delete stagingTexture;
			}

			gfx.GetDevice()->BindUniformBuffer(cameraBuffer);
			gfx.GetDevice()->BindStorageBuffer(transformBuffer);

			gfx::DebugRenderer* debugRenderer = gfx::DebugRenderer::Create(gfx.GetDevice(), { .maxVertexCount = 16 * 1024 });

			AxDenseHashMap<AxString, UniquePtr<gfx::GraphicsPipeline>> pipelines;
			AxArray<asset::PipelineDesc> pipelineDescs = asset::PipelineLoader{}.ReadFromFile(R"(X:\ApexGameEngine-Vulkan\Tests\Graphics\Pipelines.txt)");
			for (auto& pipelineDesc : pipelineDescs)
			{
				gfx::ShaderModule* vertexShader = gfx.GetDevice()->CreateShaderModule(pipelineDesc.shaders[asset::PipelineDesc::Vertex].c_str(), pipelineDesc.shaders[asset::PipelineDesc::Vertex].c_str());
				gfx::ShaderModule* fragmentShader = gfx.GetDevice()->CreateShaderModule(pipelineDesc.shaders[asset::PipelineDesc::Fragment].c_str(), pipelineDesc.shaders[asset::PipelineDesc::Fragment].c_str());
				gfx::GraphicsPipeline* pipeline = gfx.GetDevice()->CreateGraphicsPipeline(pipelineDesc.name.c_str(),
				{
					.shaderStages = {
						.vertexShader = vertexShader,
						.fragmentShader = fragmentShader
					},
				});
				pipelines.emplace(std::move(pipelineDesc.name), pipeline);

				delete vertexShader;
				delete fragmentShader;
				break;
			}

			Camera camera { .position = { 0, 0, 5 } };
			
			const u32 framesInFlight = gfx.GetDevice()->GetMaxFrameCount();

			AxArray<gfx::CommandBuffer*> cmdbufs;
			cmdbufs.resize(framesInFlight);

			for (u32 i = 0; i < framesInFlight; i++)
				cmdbufs[i] = graphicsQueue->AllocateCommandBuffer();

			plat::Timer globalTimer;
			plat::Timer frameTimer;
			globalTimer.Start();
			frameTimer.Start();

			while (g_running)
			{
				ZoneScopedN("Game Loop");

				globalTimer.End();
				frameTimer.End();

				f32 elapsedTime = globalTimer.GetElapsedSeconds();
				f32 deltaTime = frameTimer.GetElapsedSeconds();

				frameTimer.Restart();

				plat::PlatformManager::PollEvents();

				g_running = !mainWindow.ShouldQuit();

				//if (frameCount++ % 1000) axDebugFmt("DeltaTime (us) : {}", deltaTime);

				if (!g_running)
				{
					gfx.GetDevice()->WaitForIdle();
					break;
				}

				if (g_resized)
				{
					u32 width, height;
					mainWindow.GetDimensions(&width, &height);
					gfx.ResizeSurface(width, height);
					g_resized = false;
					continue;
				}

				debugRenderer->NewFrame();
				AxImGui::BeginFrame();

				CameraMatrices cameraMatrices;
				{
					ZoneScopedN("Update Camera");
					// CameraMovement cam { .targetRotation = camera.rotation };
					// ProcessGamepadInput(cam, deltaTime);
					math::Vector3 deltaPosition;
					math::Quat targetRotation = camera.rotation;

					auto& inputManager = plat::PlatformManager::GetInputManager();
					auto& gamepad = inputManager.GetGamepad(0);
					
					deltaPosition.x += gamepad.leftStick.x * deltaTime * 5;
					deltaPosition.z -= gamepad.leftStick.y * deltaTime * 5;
					targetRotation *= math::Quat::fromAxisAngle(math::Vector3::unitY(), gamepad.rightStick.x * deltaTime).normalized();
					targetRotation = math::Quat::fromAxisAngle(math::Vector3::unitX(), -gamepad.rightStick.y * deltaTime).normalized() * targetRotation;

					camera.rotation = targetRotation;
					camera.position += camera.rotation.applyToVector(deltaPosition);
					UpdateCamera(cameraMatrices, gfx.GetDevice()->GetSurfaceDim(), camera);
					memcpy_s(cameraBuffer->GetMappedPointer(), cameraBuffer->GetSize(), &cameraMatrices, sizeof(CameraMatrices));
				}

				{
					ZoneScopedN("Update Transforms");
					visible = 0;
					Frustum frustum;
					MakeViewSpaceFrustum(frustum, cameraMatrices);
					math::Quat q = math::Quat::fromAxisAngle(math::Vector3{ 1.0, 1.0, 0.0 }.normalize(), elapsedTime * 2.f);
					math::Matrix4x4 r = q.matrix();

					AxSpan transforms { (math::Matrix4x4*)transformBuffer->GetMappedPointer(), 1000 };
					for (int i = 0; i < 1000; i++)
					{
						ZoneScopedN("Update Transforms - for loop");
						math::Matrix4x4 transform;
						{
							ZoneScopedN("Update Single Transform");
							transform = r;
							transform.setTranslation(positions[i]);
						}
						AABB bbox;
						{
							ZoneScopedN("Transform AABB");
							bbox = transform * aabb;
						}
						if (CheckFrustumVisibility(frustum, bbox))
						{
							{
								ZoneScopedN("Debug Render - Cube");
								debugRenderer->Cube(bbox.GetCenter(), bbox.GetExtent());
							}
							ZoneScopedN("Store Single Transform");
							transforms[visible++] = transform;
						}
					}
				}

				if (ImGui::Begin("Frame Stats"))
				{
					ImGui::Text("Frame Time:    %0.3f", deltaTime);
					ImGui::Text("No. Objects:    %u", visible);
					ImGui::End();
				}

				AxImGui::EndFrame();

				{
					ZoneScopedN("Render");
					const Dim2D dims = gfx.GetDevice()->GetSurfaceDim();

					const gfx::Image* swapchainImage;
					{
						ZoneScopedN("AcquireNextImage");
						swapchainImage = gfx.GetDevice()->AcquireNextImage();
					}

					const u32 currentFrameIndex = gfx.GetDevice()->GetCurrentFrameIndex();
					ZoneValue(currentFrameIndex);

					gfx::CommandBuffer* cmdbuf = cmdbufs[currentFrameIndex];
					{
						ZoneScopedN("Create Command Buffer");
						cmdbuf->Begin();
						cmdbuf->BindGlobalDescriptorSets();

						cmdbuf->TransitionImage(swapchainImage,
												gfx::ImageLayout::Undefined,				gfx::PipelineStageFlagBits::ColorAttachmentOutput,	gfx::AccessFlagBits::None,					gfx::QueueType::Graphics,
												gfx::ImageLayout::ColorAttachmentOptimal,	gfx::PipelineStageFlagBits::ColorAttachmentOutput,	gfx::AccessFlagBits::ColorAttachmentWrite,	gfx::QueueType::Graphics);

						cmdbuf->BeginRendering(swapchainImage->GetView(), depthImages[currentFrameIndex]->GetView());
						//cmdbuf->Clear();
						cmdbuf->BindGraphicsPipeline(pipelines["StandardPhongPipeline"].get());
						cmdbuf->SetViewport({ 0, 0, static_cast<float>(dims.width), static_cast<float>(dims.height), 0.0f, 1.0f });
						cmdbuf->SetScissor({ 0, 0, dims.width, dims.height });

						gfx::VertexBuffer vbuf { mesh->GetVertexBuffer().Get(), mesh->GetVertexFormat(), mesh->GetVertexCount() };
						cmdbuf->BindVertexBuffer(&vbuf);
						cmdbuf->BindIndexBuffer(mesh->GetIndexBuffer().Get());
						for (uint32_t modelId = 0; modelId < visible; modelId++)
						{
							cmdbuf->PushConstants(modelId);
							cmdbuf->DrawIndexed(mesh->GetIndexCount());
						}
						debugRenderer->Draw(cmdbuf);

						cmdbuf->EndRendering();

						cmdbuf->BeginRendering(swapchainImage->GetView(), nullptr, false);
						AxImGui::Render(cmdbuf);
						cmdbuf->EndRendering();

						cmdbuf->TransitionImage(swapchainImage, 
												gfx::ImageLayout::ColorAttachmentOptimal,	gfx::PipelineStageFlagBits::ColorAttachmentOutput,	gfx::AccessFlagBits::ColorAttachmentWrite,	gfx::QueueType::Graphics,
												gfx::ImageLayout::PresentSrcOptimal,		gfx::PipelineStageFlagBits::BottomOfPipe,			gfx::AccessFlagBits::None,					gfx::QueueType::Graphics);

						cmdbuf->End();
					}
					{
						ZoneScopedN("Submit");
						graphicsQueue->Submit(cmdbuf, true, gfx::PipelineStageFlagBits::ColorAttachmentOutput, true);
					}
					{
						ZoneScopedN("Present");
						graphicsQueue->Present();
					}
				}

				TracyPlot("Visible Objects", static_cast<s64>(visible));

				FrameMark;
			}

			for (auto cmdbuf : cmdbufs)
			{
				delete cmdbuf;
			}

			for (auto depthImage : depthImages)
			{
				delete depthImage;
			}

			AxImGui::Shutdown(gfx);

			delete debugRenderer;
			delete texture;
			delete transformBuffer;
			delete cameraBuffer;
		}
	}
	gfx::Factory::ShutdownContext();
	mem::MemoryManager::shutdown();

	return 0;
}
