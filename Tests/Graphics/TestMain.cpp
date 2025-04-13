#define NOMINMAX
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

#include "Containers/AxArray.h"
#include "Containers/AxStringView.h"
#include "Core/Console.h"
#include "Core/Logging.h"
#include "Core/Files.h"
#include "Graphics/Factory.h"
#include "Graphics/GraphicsContext.h"
#include "Memory/MemoryManager.h"
#include "Math/Vector3.h"

#include <charconv>
#include <vk_mem_alloc.h>

#include "ObjLoader.h"
#include "Math/Matrix4x4.h"

#define WNDCLASSNAME "Graphics_Test"
#define APPNAME      "ApexGraphics-Test"


bool g_running = true;
bool g_resized = false;

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		g_running = false;
		break;

	case WM_SIZE:
		g_resized = true;
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void OpenWindow(HINSTANCE hInstance, HWND& hwnd, int nCmdShow)
{
	WNDCLASS wc{};
	wc.lpfnWndProc   = WndProc;
	wc.lpszClassName = WNDCLASSNAME;
	wc.hInstance     = hInstance;

	RegisterClass(&wc);

	hwnd = CreateWindowEx(
		0,                               // Optional window styles
		WNDCLASSNAME,	                 // Window class
		APPNAME,                         // Window text
		WS_OVERLAPPEDWINDOW,             // Window style
		CW_USEDEFAULT, CW_USEDEFAULT,    // Position
		1366, 768,                       // Size
		NULL,                            // Parent window
		NULL,                            // Menu
		hInstance,
		NULL 
	);
	ShowWindow(hwnd, nCmdShow);
}

void PollEvents(HWND hwnd)
{
	MSG msg{nullptr};
	while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

struct CameraMovement
{
	apex::math::Vector3 targetPosition;
	apex::math::Vector3 targetRotation;
};

void ProcessGamepadInput(CameraMovement& camera_movement, float dt)
{
	XINPUT_STATE state {};
	DWORD dwResult = XInputGetState(0, &state);

	if (ERROR_SUCCESS == dwResult)
	{
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
		{
			//axInfo("Gamepad : A");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
		{
			//axInfo("Gamepad : B");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
		{
			//axInfo("Gamepad : X");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		{
			//axInfo("Gamepad : Y");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		{
			//axInfo("Gamepad : LB");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		{
			//axInfo("Gamepad : RB");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		{
			//axInfo("Gamepad : Left Thumb");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			//axInfo("Gamepad : Right Thumb");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
		{
			//axInfo("Gamepad : Back");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
		{
			//axInfo("Gamepad : Start");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
			//axInfo("Gamepad : DPad Up");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			//axInfo("Gamepad : DPad Down");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
			//axInfo("Gamepad : DPad Left");
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			//axInfo("Gamepad : DPad Right");
		}
		if (state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			//axInfoFmt("Gamepad : Left Thumb X : {}", state.Gamepad.sThumbLX);
			float fThumbLX = static_cast<float>(state.Gamepad.sThumbLX) / static_cast<float>(32767);
			camera_movement.targetPosition.x += fThumbLX * dt * 0.001f;
		}
		if (state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			// axInfoFmt("Gamepad : Left Thumb Y : {}", state.Gamepad.sThumbLY);
			float fThumbLY = static_cast<float>(state.Gamepad.sThumbLY) / static_cast<float>(32767);
			camera_movement.targetPosition.z += fThumbLY * dt * 0.001f;
		}
		if (state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			axInfoFmt("Gamepad : Right Thumb X : {}", state.Gamepad.sThumbRX);
		}
		if (state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			axInfoFmt("Gamepad : Right Thumb Y : {}", state.Gamepad.sThumbRY);
		}
		if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		{
			axInfoFmt("Gamepad : Left Trigger : {}", state.Gamepad.bLeftTrigger);
		}
		if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		{
			axInfoFmt("Gamepad : Right Trigger : {}", state.Gamepad.bRightTrigger);
		}
	}
}

float clamp(float val, float min_val, float max_val)
{
	return val < min_val ? min_val : val > max_val ? max_val : val;
}

void ProcessGamepadRumble(float rumble_left, float rumble_right)
{
	rumble_left = clamp(rumble_left, 0.0f, 1.0f);
	rumble_right = clamp(rumble_right, 0.0f, 1.0f);

	XINPUT_VIBRATION vibration {
		.wLeftMotorSpeed = static_cast<WORD>(rumble_left * 65535u),
		.wRightMotorSpeed = static_cast<WORD>(rumble_right * 65535u),
	};

	DWORD dwResult = XInputSetState(0, &vibration);
	axAssertFmt(ERROR_SUCCESS == dwResult, "Failed to set gamepad vibration : {}", dwResult);
}

struct Vertex { apex::math::Vector3 position; };
struct MeshCPU { apex::AxArrayRef<float> vertices; apex::AxArrayRef<apex::u32> indices; };
struct MeshGPU { apex::gfx::BufferRef vertexBuffer; apex::gfx::BufferRef indexBuffer; };

MeshCPU LoadMesh(const char* filename, apex::AxArray<float>& vertices, apex::AxArray<apex::u32>& indices)
{
	using namespace std::string_view_literals;
	apex::File file = apex::File::OpenExisting(filename);
	apex::AxArray<char> data = file.Read();
	std::string_view meshStr { data.data(), data.size() };

	size_t nvertices = 0, nfaces = 0, npvert = 3, npface = 3;
	size_t offset = 0, newOffset;
	while (offset != std::string_view::npos && offset < meshStr.size())
	{
		newOffset = meshStr.find_first_of("\r\n", offset);
		if (newOffset > offset)
		{
			std::string_view line = meshStr.substr(offset, newOffset - offset);
			if (line[0] == ':')
			{
				line.remove_prefix(1);
				size_t space = line.find_first_of(" \t");
				std::string_view key = line.substr(0, space);
				std::string_view val = line.substr(space + 1);
				if (key == "NVERTICES"sv)
				{
					auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), nvertices);
					axAssertFmt(err == std::errc(), "Could not read NVERTICES from mesh file : {}. Expected a u32, got {}", filename, val);
				}
				else if (key == "NFACES"sv)
				{
					auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), nfaces);
					axAssertFmt(err == std::errc(), "Could not read NFACES from mesh file : {}. Expected a u32, got {}", filename, val);
				}
				else if (key == "VERTEX"sv)
				{
					auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), npvert);
					axAssertFmt(err == std::errc(), "Could not read VERTEX from mesh file : {}. Expected a u32, got {}", filename, val);
				}
				else if (key == "FACE"sv)
				{
					auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), npface);
					axAssertFmt(err == std::errc(), "Could not read FACE from mesh file : {}. Expected a u32, got {}", filename, val);
				}
				else
				{
					axWarnFmt("Ignoring unknown option : {}", key);
				}
			}
			else if (line[0] == 'v')
			{
				axAssert(nvertices && nfaces && npvert && npface);
				if (vertices.capacity() == 0)
					vertices.reserve(nvertices * npvert);

				line.remove_prefix(1);
				size_t lineOffset = 0;
				size_t vi;
				for (vi = 0; vi < npvert; )
				{
					size_t spaceOffset = line.find_first_of(" \t", lineOffset);
					if (axVerifyFmt(spaceOffset != line.npos, "Unexpected end of line reached. Vertex expects {} values, got {}.", npvert, vi))
					{
						break;
					}
					if (spaceOffset > lineOffset)
					{
						std::string_view val = line.substr(lineOffset, spaceOffset-lineOffset);
						float v;
						auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), v);
						axAssertFmt(err == std::errc(), "Could not read vertex value from mesh file : {}. Expected a float, got {}", filename, val);

						vertices.append(v);
						vi++;
					}
					lineOffset = spaceOffset == std::string_view::npos ? spaceOffset : spaceOffset + 1;
				}
				axAssert(vi == npvert);
			}
			else if (line[0] == 'f')
			{
				axAssert(nvertices && nfaces && npvert && npface);
				if (indices.empty())
					indices.reserve(nfaces * npface);

				line.remove_prefix(1);
				size_t lineOffset = 0;
				size_t fi;
				for (fi = 0; fi < npface; )
				{
					size_t spaceOffset = line.find_first_of(" \t", lineOffset);
					if (spaceOffset > lineOffset)
					{
						std::string_view val = line.substr(lineOffset, spaceOffset-lineOffset);
						apex::u32 f;
						auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), f);
						axAssertFmt(err == std::errc(), "Could not read vertex value from mesh file : {}. Expected a u32, got {}", filename, val);

						indices.append(f);
						fi++;
					}
					lineOffset = spaceOffset == std::string_view::npos ? spaceOffset : spaceOffset + 1;
				}
				axAssert(fi == npface);
			}
			else
			{
				
			}
		}
		offset = newOffset == std::string_view::npos ? newOffset : newOffset + 1;
	}

	return { make_array_ref(vertices), make_array_ref(indices) };
}

MeshGPU UploadMeshToGpu(apex::gfx::Context& gfx, MeshCPU const& mesh)
{
	apex::gfx::Buffer* vertexBuffer = gfx.GetDevice()->CreateVertexBuffer("meshVB", sizeof(float) * mesh.vertices.size(), nullptr);
	apex::gfx::Buffer* indexBuffer = gfx.GetDevice()->CreateIndexBuffer("meshIB", sizeof(apex::u32) * mesh.indices.size(), nullptr);
	{
		apex::gfx::BufferRef vertexStagingBuffer = gfx.GetDevice()->CreateStagingBuffer("Staging-Vertex", sizeof(float) * mesh.vertices.size());
		apex::gfx::BufferRef indexStagingBuffer = gfx.GetDevice()->CreateStagingBuffer("Staging-Index", sizeof(apex::u32) * mesh.indices.size());

		apex::memcpy_s<float>(vertexStagingBuffer->GetMappedPointer(), mesh.vertices.size(), mesh.vertices.data(), mesh.vertices.size());
		apex::memcpy_s<apex::u32>(indexStagingBuffer->GetMappedPointer(), mesh.indices.size(), mesh.indices.data(), mesh.indices.size());

		apex::gfx::CommandBuffer* commands = gfx.GetDevice()->AllocateCommandBuffer(apex::gfx::DeviceQueue::Transfer, 0, 0);
		commands->Begin();
		commands->CopyBuffer(vertexBuffer, vertexStagingBuffer.GetPointer());
		commands->CopyBuffer(indexBuffer, indexStagingBuffer.GetPointer());
		commands->End();

		gfx.GetDevice()->SubmitImmediateCommandBuffer(apex::gfx::DeviceQueue::Transfer, commands);
		gfx.GetDevice()->WaitForQueueIdle(apex::gfx::DeviceQueue::Transfer);
		delete commands;
	}

	return { vertexBuffer, indexBuffer };
}


struct Camera
{
	apex::math::Matrix4x4 view;
	apex::math::Matrix4x4 projection;
};

void UpdateCamera(Camera &camera, apex::gfx::Dim2D dims, apex::math::Matrix4x4 const& camera_transform)
{
	camera.view = apex::math::lookAt(camera_transform.getTranslation(), { 0.f, 0.f, 0.f }, apex::math::Vector3::unitY());
	camera.projection = apex::math::perspective(apex::math::radians(60.f), static_cast<float>(dims.width) / static_cast<float>(dims.height), 10000.f, 0.1f);
	camera.projection[1][1] *= -1;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	using namespace apex;

	logging::Logger::Init();

	Console console(APPNAME);
	logging::Logger::AttachConsole(&console);

	mem::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });
	{
		HWND hwnd;
		OpenWindow(hInstance, hwnd, nCmdShow);

		gfx::Context gfx = gfx::Context::CreateContext(gfx::ContextApi::Vulkan);
		gfx.Init(hInstance, hwnd);
		// Rendering code
		{
			// MeshCPU meshCpu = LoadMesh(R"(X:\ApexGameEngine-Vulkan\Templates\Assets\cube.axmesh)");
			ObjLoader objLoader(R"(X:\ApexGameEngine-Vulkan\Templates\Assets\FinalBaseMesh.obj)");

			MeshCPU meshCpu { .vertices = objLoader.GetVertexBufferData(), .indices = objLoader.GetIndexBufferData() };
			MeshGPU meshGpu = UploadMeshToGpu(gfx, meshCpu);

			gfx::Buffer* cameraBuffer = gfx.GetDevice()->CreateBuffer("Camera Uniform Buffer",
			                                                          {
				                                                          .size = sizeof(Camera),
				                                                          .usageFlags = gfx::BufferUsageFlagBits::Uniform,
				                                                          .requiredFlags = gfx::MemoryPropertyFlagBits::DeviceLocal |
				                                                          gfx::MemoryPropertyFlagBits::HostVisible |
				                                                          gfx::MemoryPropertyFlagBits::HostCoherent,
				                                                          .memoryFlags = gfx::MemoryAllocateFlagBits::HostAccessSequential,
				                                                          .createMapped = true,
			                                                          });

			gfx::Image* depthTexture = gfx.GetDevice()->CreateImage("Depth",
			                                                        {
				                                                        .imageType = gfx::ImageType::Image2D,
				                                                        .format = gfx::ImageFormat::D32_SFLOAT,
				                                                        .usageFlags = gfx::ImageUsageFlagBits::DepthStencilAttachment,
				                                                        .dimensions = { gfx.GetDevice()->GetSurfaceDim().width, gfx.GetDevice()->GetSurfaceDim().height },
				                                                        .requiredFlags = gfx::MemoryPropertyFlagBits::DeviceLocal,
				                                                        .createMapped = false
			                                                        });

			{
				constexpr gfx::DeviceQueue queue = gfx::DeviceQueue::Graphics;
				gfx::CommandBuffer* commands = gfx.GetDevice()->AllocateCommandBuffer(queue, 0, 0);
				commands->Begin();
				commands->TransitionImage(depthTexture,
				                          gfx::ImageLayout::Undefined, gfx::ImageLayout::DepthStencilAttachmentOptimal,
				                          gfx::AccessFlagBits::None, gfx::AccessFlagBits::DepthStencilAttachmentWrite,
				                          gfx::PipelineStageFlagBits::None, gfx::PipelineStageFlagBits::EarlyFragmentTests
				                          );
				commands->End();
				gfx.GetDevice()->SubmitImmediateCommandBuffer(queue, commands);
				gfx.GetDevice()->WaitForQueueIdle(queue);
				delete commands;
			}

			gfx::Image* texture = gfx.GetDevice()->CreateImage("Texture",
			                                                   {
				                                                   .imageType = gfx::ImageType::Image2D,
				                                                   .format = gfx::ImageFormat::R8G8B8A8_UNORM,
				                                                   .usageFlags = gfx::ImageUsageFlagBits::Sampled |
				                                                   gfx::ImageUsageFlagBits::TransferDst,
				                                                   .dimensions = { 2, 2 },
				                                                   .requiredFlags = gfx::MemoryPropertyFlagBits::DeviceLocal,
				                                                   .memoryFlags = gfx::MemoryAllocateFlagBits::None,
				                                                   .createMapped = false,
			                                                   });

			{
				u32 data[4] = { 0, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
				size_t dataSize = sizeof(data);
				gfx::Buffer* uploadBuffer = gfx.GetDevice()->CreateStagingBuffer("Staging-Texture", dataSize);
				memcpy(uploadBuffer->GetMappedPointer(), data, dataSize);

				constexpr gfx::DeviceQueue queue = gfx::DeviceQueue::Graphics;

				gfx::CommandBuffer* commands = gfx.GetDevice()->AllocateCommandBuffer(queue, 0, 0);
				commands->Begin();
				commands->TransitionImage(texture,
				                          gfx::ImageLayout::Undefined, gfx::ImageLayout::TransferDstOptimal,
				                          gfx::AccessFlagBits::None, gfx::AccessFlagBits::TransferWrite,
				                          gfx::PipelineStageFlagBits::TopOfPipe, gfx::PipelineStageFlagBits::Transfer);
				commands->CopyBufferToImage(texture, uploadBuffer, gfx::ImageLayout::TransferDstOptimal);
				commands->TransitionImage(texture,
				                          gfx::ImageLayout::TransferDstOptimal, gfx::ImageLayout::ShaderReadOnlyOptimal,
				                          gfx::AccessFlagBits::TransferWrite, gfx::AccessFlagBits::ShaderRead,
				                          gfx::PipelineStageFlagBits::Transfer, gfx::PipelineStageFlagBits::FragmentShader);
				commands->End();

				gfx.GetDevice()->SubmitImmediateCommandBuffer(queue, commands);
				gfx.GetDevice()->WaitForQueueIdle(queue);

				delete uploadBuffer;
				delete commands;
			}

			gfx::ShaderModule* vertexShader = gfx.GetDevice()->CreateShaderModule("standard_phong.vert", R"(X:\ApexGameEngine-Vulkan\build-msvc\Graphics\spv\standard_phong.vert.spv)");
			gfx::ShaderModule* fragmentShader = gfx.GetDevice()->CreateShaderModule("standard_phong.frag", R"(X:\ApexGameEngine-Vulkan\build-msvc\Graphics\spv\standard_phong.frag.spv)");
			gfx::GraphicsPipeline* pipeline = gfx.GetDevice()->CreateGraphicsPipeline("StandardPhongPipeline",
			{
				.shaderStages = {
					.vertexShader = vertexShader,
					.fragmentShader = fragmentShader
				},
			});

			math::Matrix4x4 cameraTransform = translate(math::Matrix4x4::identity(), { 0.0, 2.0, 4.0 });
			UpdateCamera(*static_cast<Camera*>(cameraBuffer->GetMappedPointer()), gfx.GetDevice()->GetSurfaceDim(), cameraTransform);

			// apex::gfx::DescriptorSet* cameraDescriptorSet = gfx.GetDevice()->CreateDescriptorSet();

			const u32 framesInFlight = gfx.GetDevice()->GetFramesInFlight();

			AxArray<gfx::CommandBuffer*> cmdbufs;
			cmdbufs.resize(framesInFlight);

			for (u32 i = 0; i < framesInFlight; i++)
				cmdbufs[i] = gfx.GetDevice()->AllocateCommandBuffer(gfx::DeviceQueue::Graphics, i, 0);

			LARGE_INTEGER liFrequency;
			LARGE_INTEGER liStartTime, liPrevEndTime, liEndTime, liElapsedMicroseconds, liDeltaSeconds;

			QueryPerformanceFrequency(&liFrequency);
			QueryPerformanceCounter(&liStartTime);
			liPrevEndTime = liStartTime;

			while (g_running)
			{
				QueryPerformanceCounter(&liEndTime);
				liElapsedMicroseconds.QuadPart = liEndTime.QuadPart - liStartTime.QuadPart;
				liElapsedMicroseconds.QuadPart *= 1000000;
				liElapsedMicroseconds.QuadPart /= liFrequency.QuadPart;
				float elapsedTime = static_cast<float>(liElapsedMicroseconds.QuadPart);

				liDeltaSeconds.QuadPart = liEndTime.QuadPart - liPrevEndTime.QuadPart;
				liDeltaSeconds.QuadPart /= liFrequency.QuadPart;
				float deltaTime = static_cast<float>(liDeltaSeconds.QuadPart);

				PollEvents(hwnd);

				if (!g_running)
				{
					gfx.GetDevice()->WaitForIdle();
					break;
				}

				if (g_resized)
				{
					RECT rect;
					GetClientRect(hwnd, &rect);
					gfx.ResizeWindow(rect.right - rect.left, rect.bottom - rect.top);
					g_resized = false;
					continue;
				}

				{
					CameraMovement cam {};
					ProcessGamepadInput(cam, deltaTime);

					math::Vector3 forward = normalize(-cameraTransform.getTranslation()); // target - eye
					math::Vector3 up = math::Vector3::unitY();
					math::Vector3 right = normalize(cross(forward, up));

					math::Vector3 displacement = cam.targetPosition.z * forward + cam.targetPosition.x * right;
					if (!displacement.isNearZero())
					{
						cameraTransform = translate(cameraTransform, displacement);
						UpdateCamera(*static_cast<Camera*>(cameraBuffer->GetMappedPointer()), gfx.GetDevice()->GetSurfaceDim(), cameraTransform);
					}
				}

				const gfx::Dim2D dims = gfx.GetDevice()->GetSurfaceDim();

				const gfx::Image* swapchainImage = gfx.GetDevice()->AcquireNextImage();

				const u32 currentFrameIndex = gfx.GetDevice()->GetCurrentFrameIndex();

				gfx::CommandBuffer* cmdbuf = cmdbufs[currentFrameIndex];
				cmdbuf->Begin();

				cmdbuf->TransitionImage(swapchainImage, gfx::ImageLayout::Undefined, gfx::ImageLayout::ColorAttachmentOptimal,
				                        gfx::AccessFlagBits::None, gfx::AccessFlagBits::ColorAttachmentWrite,
				                        gfx::PipelineStageFlagBits::TopOfPipe, gfx::PipelineStageFlagBits::ColorAttachmentOutput);

				cmdbuf->BeginRendering(swapchainImage->GetView(), depthTexture->GetView());

				cmdbuf->Clear();
				cmdbuf->BindGraphicsPipeline(pipeline);

				cmdbuf->SetViewport({ 0, 0, static_cast<float>(dims.width), static_cast<float>(dims.height), 0.0f, 1.0f });
				cmdbuf->SetScissor({ 0, 0, dims.width, dims.height });

				cmdbuf->BindVertexBuffer(meshGpu.vertexBuffer.GetPointer());
				cmdbuf->BindIndexBuffer(meshGpu.indexBuffer.GetPointer());
				cmdbuf->DrawIndexed(meshCpu.indices.size());

				cmdbuf->EndRendering();

				cmdbuf->TransitionImage(swapchainImage, gfx::ImageLayout::ColorAttachmentOptimal, gfx::ImageLayout::PresentSrcOptimal,
				                        gfx::AccessFlagBits::ColorAttachmentWrite, gfx::AccessFlagBits::None,
				                        gfx::PipelineStageFlagBits::ColorAttachmentOutput, gfx::PipelineStageFlagBits::BottomOfPipe);

				cmdbuf->End();

				gfx.GetDevice()->SubmitCommandBuffer(gfx::DeviceQueue::Graphics, cmdbuf);
				gfx.GetDevice()->Present(gfx::DeviceQueue::Graphics);
			}

			for (auto cmdbuf : cmdbufs)
			{
				delete cmdbuf;
			}

			delete texture;
			delete depthTexture;
			delete cameraBuffer;
			delete vertexShader;
			delete fragmentShader;
			delete pipeline;
		}
	}

	mem::MemoryManager::shutdown();

	system("pause");

	return 0;
}
