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
struct MeshCPU { apex::AxArray<float> vertices; apex::AxArray<apex::u32> indices; };
struct MeshGPU { apex::gfx::BufferRef vertexBuffer; apex::gfx::BufferRef indexBuffer; };

MeshCPU LoadMesh(const char* filename)
{
	using namespace std::string_view_literals;
	apex::File file = apex::File::OpenExisting(filename);
	apex::AxArray<char> data = file.Read();
	std::string_view meshStr { data.data(), data.size() };

	size_t nvertices = 0, nfaces = 0, npvert = 3, npface = 3;
	apex::AxArray<float> vertices;
	apex::AxArray<apex::u32> indices;

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

	return { vertices, indices };
}

MeshGPU UploadMeshToGpu(apex::gfx::Context& gfx, MeshCPU const& mesh)
{
	apex::gfx::Buffer* vertexBuffer = gfx.GetDevice()->CreateVertexBuffer("meshVB", sizeof(float) * mesh.vertices.size(), nullptr);
	apex::gfx::Buffer* indexBuffer = gfx.GetDevice()->CreateIndexBuffer("meshIB", sizeof(apex::u32) * mesh.indices.size(), nullptr);
	{
		apex::gfx::BufferRef vertexStagingBuffer = gfx.GetDevice()->CreateStagingBuffer(sizeof(float) * mesh.vertices.size());
		apex::gfx::BufferRef indexStagingBuffer = gfx.GetDevice()->CreateStagingBuffer(sizeof(apex::u32) * mesh.indices.size());

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
	camera.projection = apex::math::perspective(apex::math::radians(60.f), static_cast<float>(dims.width) / static_cast<float>(dims.height), 0.1f, 1000.f);
	camera.projection[1][1] *= -1;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	apex::logging::Logger::Init();

	apex::Console console(APPNAME);
	apex::logging::Logger::AttachConsole(&console);

	apex::mem::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });
	{
		HWND hwnd;
		OpenWindow(hInstance, hwnd, nCmdShow);

		apex::gfx::Context gfx = apex::gfx::Context::CreateContext(apex::gfx::ContextApi::Vulkan);
		gfx.Init(hInstance, hwnd);
		// Rendering code
		{
			MeshCPU meshCpu = LoadMesh(R"(X:\ApexGameEngine-Vulkan\Templates\Assets\cube.axmesh)");
			MeshGPU meshGpu = UploadMeshToGpu(gfx, meshCpu);

			apex::gfx::Buffer* cameraBuffer = gfx.GetDevice()->CreateBuffer("Camera Uniform Buffer",
			{
				.size = sizeof(Camera),
				.usageFlags = apex::gfx::BufferUsageFlagBits::Uniform,
				.requiredFlags = apex::gfx::MemoryPropertyFlagBits::DeviceLocal | apex::gfx::MemoryPropertyFlagBits::HostVisible | apex::gfx::MemoryPropertyFlagBits::HostCoherent,
				.memoryFlags = apex::gfx::MemoryAllocateFlagBits::HostAccessSequential,
				.createMapped = true,
			});

			apex::gfx::ShaderModule* vertexShader = gfx.GetDevice()->CreateShaderModule("standard_phong.vert", R"(X:\ApexGameEngine-Vulkan\build-msvc\Graphics\spv\standard_phong.vert.spv)");
			apex::gfx::ShaderModule* fragmentShader = gfx.GetDevice()->CreateShaderModule("standard_phong.frag", R"(X:\ApexGameEngine-Vulkan\build-msvc\Graphics\spv\standard_phong.frag.spv)");
			apex::gfx::GraphicsPipeline* pipeline = gfx.GetDevice()->CreateGraphicsPipeline("StandardPhongPipeline",
			{
				.shaderStages = {
					.vertexShader = vertexShader,
					.fragmentShader = fragmentShader
				},
			});

			apex::math::Matrix4x4 cameraTransform = translate(apex::math::Matrix4x4::identity(), { 0.0, 2.0, 4.0 });
			apex::AxArray<apex::gfx::DescriptorSet> descriptorSets = gfx.GetDevice()->AllocateDescriptorSets(pipeline);
			descriptorSets[0].Add({ apex::gfx::DescriptorType::eUniformBuffer, cameraBuffer });
			gfx.GetDevice()->UpdateDescriptorSet(descriptorSets[0]);
			UpdateCamera(*static_cast<Camera*>(cameraBuffer->GetMappedPointer()), gfx.GetDevice()->GetSurfaceDim(), cameraTransform);

			const apex::u32 framesInFlight = gfx.GetDevice()->GetFramesInFlight();

			apex::AxArray<apex::gfx::CommandBuffer*> cmdbufs;
			cmdbufs.resize(framesInFlight);

			for (apex::u32 i = 0; i < framesInFlight; i++)
				cmdbufs[i] = gfx.GetDevice()->AllocateCommandBuffer(apex::gfx::DeviceQueue::Graphics, i, 0);

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

					apex::math::Vector3 forward = normalize(-cameraTransform.getTranslation()); // target - eye
					apex::math::Vector3 up = apex::math::Vector3::unitY();
					apex::math::Vector3 right = normalize(cross(forward, up));

					apex::math::Vector3 displacement = cam.targetPosition.z * forward + cam.targetPosition.x * right;
					if (!displacement.isNearZero())
					{
						cameraTransform = translate(cameraTransform, displacement);
						UpdateCamera(*static_cast<Camera*>(cameraBuffer->GetMappedPointer()), gfx.GetDevice()->GetSurfaceDim(), cameraTransform);
					}
				}

				const apex::gfx::Dim2D dims = gfx.GetDevice()->GetSurfaceDim();

				const apex::gfx::Image* swapchainImage = gfx.GetDevice()->AcquireNextImage();

				const apex::u32 currentFrameIndex = gfx.GetDevice()->GetCurrentFrameIndex();

				apex::gfx::CommandBuffer* cmdbuf = cmdbufs[currentFrameIndex];
				cmdbuf->Begin();

				cmdbuf->TransitionImage(swapchainImage, apex::gfx::ImageLayout::Undefined, apex::gfx::ImageLayout::ColorAttachmentOptimal,
					apex::gfx::AccessFlagBits::None, apex::gfx::AccessFlagBits::ColorAttachmentWrite,
					apex::gfx::PipelineStageFlagBits::TopOfPipe, apex::gfx::PipelineStageFlagBits::ColorAttachmentOutput);

				cmdbuf->BeginRenderPass(swapchainImage->GetView());

				cmdbuf->Clear();
				cmdbuf->BindGraphicsPipeline(pipeline);
				cmdbuf->BindDescriptorSet(descriptorSets[0], pipeline);

				cmdbuf->SetViewport({ 0, 0, static_cast<float>(dims.width), static_cast<float>(dims.height), 0.0f, 1.0f });
				cmdbuf->SetScissor({ 0, 0, dims.width, dims.height });

				cmdbuf->BindVertexBuffer(meshGpu.vertexBuffer.GetPointer());
				cmdbuf->BindIndexBuffer(meshGpu.indexBuffer.GetPointer());
				cmdbuf->DrawIndexed(meshCpu.indices.size());

				cmdbuf->EndRenderPass();

				cmdbuf->TransitionImage(swapchainImage, apex::gfx::ImageLayout::ColorAttachmentOptimal, apex::gfx::ImageLayout::PresentSrcOptimal,
					apex::gfx::AccessFlagBits::ColorAttachmentWrite, apex::gfx::AccessFlagBits::None,
					apex::gfx::PipelineStageFlagBits::ColorAttachmentOutput, apex::gfx::PipelineStageFlagBits::BottomOfPipe);

				cmdbuf->End();

				gfx.GetDevice()->SubmitCommandBuffer(apex::gfx::DeviceQueue::Graphics, cmdbuf);
				gfx.GetDevice()->Present(apex::gfx::DeviceQueue::Graphics);
			}

			for (auto cmdbuf : cmdbufs)
			{
				delete cmdbuf;
			}

			delete cameraBuffer;
			delete vertexShader;
			delete fragmentShader;
			delete pipeline;
		}
	}

	apex::mem::MemoryManager::shutdown();

	system("pause");

	return 0;
}
