#pragma once
#include "Apex/Application.h"
#include "Apex/Window.h"
#include "Apex/Game.h"
#include "Graphics/Camera.h"
#include "Graphics/ForwardRenderer.h"
#include "Graphics/Geometry/Mesh.h"
#include "Graphics/Primitives/Quad.h"
#include "Graphics/Vulkan/VulkanComputePipeline.h"

namespace math = apex::math;
using math::Vector3;
using math::Matrix4x4;
using apex::u32;
using apex::f32;

struct Cloth
{
	struct Spring { u32 p1, p2; f32 restLength; };

	apex::AxArray<Vector3> positions;
	apex::AxArray<Vector3> velocities;

	apex::AxArray<Spring> springs;

	static constexpr int rows = 1000;
	static constexpr int cols = 1000;
	static constexpr int nParticles = rows * cols;

	float mass = 0.0025f;

	float kElastic = 1000000.f;
	float kDamping = 50.f;

	float kLen = 4.f / rows;
	float kLenStructural = kLen;
	float kLenShear = kLen * sqrtf(2);
	float kLenBend = kLen * 2;

	size_t nStructural = (rows - 1) * cols + (cols - 1) * rows;
	size_t nShear = 2 * (rows - 1) * (cols - 1);
	size_t nBend = (rows - 2) * cols + (cols - 2) * rows;

	Vector3 wind = { 0, 0, 25.f };

	float timer = 0;
	const float dt = 0.0005f;

	enum setup
	{
		eFreezeTopRow,
		eFreezeOneCorner,
		eFreezeTwoCorners,
		eFreezeThreeCorners,
		eFreezeFourCorners,
		eNone
	} setup = eFreezeTopRow;

	inline int gridToIndex(int x, int y) { return x * cols + y; }

	void init()
	{
		timer = 0;
		positions.resize(nParticles);
		velocities.resize(nParticles);
		setup = eFreezeThreeCorners;

		for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++)
		{
			positions[gridToIndex(i, j)] = { (cols/2-j) * kLen, (rows/2-i) * kLen, 0 };
			velocities[gridToIndex(i, j)] = sinf(i * 0.25f) * cosf(j * 0.25f) * Vector3(0, 0, 0.5f);
			//velocities[gridToIndex(i, j)] = Vector3(0.f);
		}

		const u32 numSprings = (rows - 1 + rows - 2) * cols + (cols - 1 + cols - 2) * rows + 2 * (rows - 1) * (cols - 1);
		springs.reserve(numSprings);
		u32 k = 0;
		for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++)
		{
			//printf("i: %d j: %d\n", i, j);
			if (j < rows - 1) springs.emplace_back(gridToIndex(i, j), gridToIndex(i, j + 1), kLenStructural);
			if (i < cols - 1) springs.emplace_back(gridToIndex(i, j), gridToIndex(i + 1, j), kLenStructural);
		}
		for (int i = 0; i < rows - 1; i++) for (int j = 0; j < cols - 1; j++)
		{
			springs.emplace_back(gridToIndex(i, j), gridToIndex(i + 1, j + 1), kLenShear);
			springs.emplace_back(gridToIndex(i, j + 1), gridToIndex(i + 1, j), kLenShear);
		}
		for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++)
		{
			if (i < rows - 2) springs.emplace_back(gridToIndex(i, j), gridToIndex(i + 2, j), kLenBend);
			if (j < cols - 2) springs.emplace_back(gridToIndex(i, j), gridToIndex(i, j + 2), kLenBend);
		}

		/*for (int i = 0; i < springs.size(); i++)
		{
			printf("spring: %d %d %f\n", springs[i].p1, springs[i].p2, springs[i].restLength);
		}*/
		
		axAssertFmt(springs.size() == numSprings, "Invalid number of springs : {}", springs.size());
	}

	void simulate(float deltaTime)
	{
		timer += deltaTime;
		timer = apex::min(timer, 0.01f); // clamp to 10ms

		while (timer > 0)
		{
			timer -= dt;
			//euler();
			rk4();
		}
	}

	void computeSpringForce(apex::AxArray<Vector3>& p, apex::AxArray<Vector3>& v, Spring const& s, Vector3 &force) const
	{
		Vector3 ab = p[s.p2] - p[s.p1];
		float length = ab.length();
		float diff = length - s.restLength;
		ab /= length;

		force = ab * diff * kElastic;
		force -= ab * (dot(v[s.p1] - v[s.p2], ab) * kDamping);
	}

	void computeAcceleration(apex::AxArray<Vector3>& p, apex::AxArray<Vector3>& v, apex::AxArray<Vector3>& a)
	{
		memset(a.data(), 0, a.size() * sizeof(Vector3));

		// spring forces
		for (int i = 0; i < springs.size(); i++)
		{
			Vector3 f;
			computeSpringForce(p, v, springs[i], f);
			a[springs[i].p1] += f;
			a[springs[i].p2] -= f;
		}

		// external forces
		for (int i = 0; i < p.size(); i++)
		{
			a[i] += Vector3(0, -9.81f, 0);
		}

		for (int i = 0; i < rows - 1; i++) for (int j = 0; j < cols - 1; j++) 
		{
			Vector3 e1 = p[gridToIndex(i, j + 1)] - p[gridToIndex(i, j)];
			Vector3 e2 = p[gridToIndex(i + 1, j)] - p[gridToIndex(i, j)];
			Vector3 normal = normalize(cross(e1, e2));
			Vector3 force = wind * abs(dot(normal, wind)) / wind.length();
			a[gridToIndex(i, j)] += force;
			if (j == rows - 2) a[gridToIndex(i, j + 1)] += force;
			if (i == cols - 2) a[gridToIndex(i + 1, j)] += force;
		}
	}

	bool isFixed(int i) const
	{
		int TL = 0, TR = cols - 1, BL = (rows - 1) * cols, BR = rows * cols - 1;

		bool shouldNotDisplace = true;
		switch (setup)
		{
		case eFreezeTopRow: shouldNotDisplace = i <= TR; break;
		case eFreezeOneCorner: shouldNotDisplace = i == TL; break;
		case eFreezeTwoCorners: shouldNotDisplace = i == TL || i == TR; break;
		case eFreezeThreeCorners: shouldNotDisplace = i == TL || i == TR || i == BR; break;
		case eFreezeFourCorners: shouldNotDisplace = i == TL || i == TR || i == BL || i == BR; break;
		case eNone: break;
		}

		return shouldNotDisplace;
	}

	void euler()
	{
		apex::AxArray<Vector3> acceleration;
		acceleration.resize(positions.size());

		computeAcceleration(positions, velocities, acceleration);

		for (int i = 0; i < positions.size(); i++)
		{
			bool shouldNotDisplace = isFixed(i);
			if (shouldNotDisplace) continue;

			velocities[i] += acceleration[i] * dt;
			positions[i] += velocities[i] * dt;
		}
	}

	void rk4()
	{
		// f1 = f(y, t)
		// f2 = f(y + f1 * dt/2, t + dt/2)
		// f3 = f(y + f2 * dt/2, t + dt/2)
		// f4 = f(y + f3 * dt, t + dt)
		// y = y + (f1 + 2*f2 + 2*f3 + f4) * dt / 6

		const auto n = positions.size();

		apex::AxArray<Vector3> p, v;
		p.resize(n);
		v.resize(velocities.size());

		apex::memcpy_s<Vector3>(p.data(), p.size(), positions.data(), n);
		apex::memcpy_s<Vector3>(v.data(), v.size(), velocities.data(), velocities.size());

		apex::AxArray<Vector3> f1p, f2p, f3p, f4p, f1v, f2v, f3v, f4v;
		f1p.resize(n); f1v.resize(n);
		f2p.resize(n); f2v.resize(n);
		f3p.resize(n); f3v.resize(n);
		f4p.resize(n); f4v.resize(n);

		apex::AxArray<Vector3> a;
		a.resize(n);

		computeAcceleration(p, v, a);

		for (int i = 0; i < n; i++)
		{
			f1p[i] = v[i] * dt;
			f1v[i] = a[i] * dt;
			p[i] = positions[i] + f1p[i] / 2;
			v[i] = velocities[i] + f1v[i] / 2;
		}

		computeAcceleration(p, v, a);

		for (int i = 0; i < n; i++)
		{
			f2p[i] = v[i] * dt;
			f2v[i] = a[i] * dt;
			p[i] = positions[i] + f2p[i] / 2;
			v[i] = velocities[i] + f2v[i] / 2;
		}

		computeAcceleration(p, v, a);

		for (int i = 0; i < n; i++)
		{
			f3p[i] = v[i] * dt;
			f3v[i] = a[i] * dt;
			p[i] = positions[i] + f3p[i];
			v[i] = velocities[i] + f3v[i];
		}

		computeAcceleration(p, v, a);

		for (int i = 0; i < n; i++)
		{
			f4p[i] = v[i] * dt;
			f4v[i] = a[i] * dt;

			bool shouldNotDisplace = isFixed(i);
			if (shouldNotDisplace) continue;

			positions[i] += (f1p[i] + 2*f2p[i] + 2*f3p[i] + f4p[i]) / 6.f;
			velocities[i] += (f1v[i] + 2*f2v[i] + 2*f3v[i] + f4v[i]) / 6.f;
		}
	}
};

struct Timer
{
	using timepoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using duration = std::chrono::duration<float, std::chrono::seconds::period>;

	timepoint currentTime;
	timepoint newTime;
	float deltaTime;

	void init()
	{
		currentTime = std::chrono::high_resolution_clock::now();
	}

	void update()
	{
		newTime = std::chrono::high_resolution_clock::now();
		deltaTime = duration(newTime - currentTime).count();
		currentTime = newTime;
	}

	[[nodiscard]] f32 getCurrentTime() const { return duration(currentTime.time_since_epoch()).count(); }
	[[nodiscard]] f32 getDeltaTime() const { return deltaTime; }
};

class ClothSim : public apex::Game
{
public:
	void initialize() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		renderer.setActiveCamera(&m_camera);
		m_cameraTransform = translate(Matrix4x4::identity(), { -4, 3, 6 });

		// m_camera.view = math::inverse(m_cameraTransform);
		m_camera.view = lookAt(m_cameraTransform.m_columns[3].xyz(), { 0, 0, 0 }, Vector3::unitY());

		int width, height;
		apex::Application::Instance()->getWindow()->getFramebufferSize(width, height);
		f32 aspect = static_cast<f32>(width) / static_cast<f32>(height);
		f32 fov = math::radians(60.f);
		m_camera.projection = math::perspective(fov, aspect, 0.1f, 1000.f);
		m_camera.projection[1][1] *= -1;

		//{ // Create mesh
		//	auto meshCpu = apex::gfx::Cube::getMesh();
		//	auto vertices = reinterpret_cast<apex::gfx::Vertex_P0_C0*>(meshCpu.m_vertexBufferCPU.dataMutable());
		//	for (int i = 0; i < meshCpu.m_vertexBufferCPU.count(); i++)
		//	{
		//		vertices[i].color = { 0, 0, 0.8f, 1 };
		//	}
		//	mesh.create(renderer.getContext().m_device, &meshCpu, nullptr);
		//}

		cloth.init();

		{ // Create cloth mesh
			int numIndices = 6 * (Cloth::rows-1) * (Cloth::cols-1) * 2;
			apex::AxArray<u32> indices(numIndices);
			indices.resize(numIndices);
			for (int i = 0; i < Cloth::rows-1; i++) for (int j = 0; j < Cloth::cols-1; j++)
			{
				int indicesPerQuad[6] = { 0, 1, Cloth::cols, 1, Cloth::cols + 1, Cloth::cols };
				for (int k = 0; k < 6; k++)
				{
					int index = i * (Cloth::cols-1) * 6 + j * 6 + k;

					indices[index] = cloth.gridToIndex(i, j) + indicesPerQuad[k];
					indices[6 * (Cloth::rows-1) * (Cloth::cols-1) + index] = cloth.gridToIndex(i, j) + indicesPerQuad[5-k];
				}
			}
			apex::gfx::IndexBufferCPU indexBufferCpu;
			indexBufferCpu.create(indices);
			dynamicMesh.create(renderer.getContext().m_device, apex::gfx::Vertex_P0_M0_C0::getVertexInfo(), cloth.nParticles, indexBufferCpu, nullptr);

			auto vertices = static_cast<apex::gfx::Vertex_P0_M0_C0*>(dynamicMesh.getVertexBuffer().m_buffer.getMappedMemory());
			auto lerp = [](auto const& a, auto const& b, float t) { return (1-t) * a + t * b; };
			for (int i = 0; i < Cloth::nParticles; i++)
			{
				const math::Vector4 startColor { 1, 0, 0, 1 }, endColor { 0, 1, 0, 1 };
				vertices[i].color = lerp(startColor, endColor, ((f32)i/(f32)Cloth::rows)/(f32)Cloth::cols);
				vertices[i].position = cloth.positions[i];
			}
		}

		auto& commandList = renderer.getCurrentCommandList();
		commandList.getCommands().reserve(100);

		timer.init();

		initializeVulkanObjects();
	}

	void update(float deltaTimeMs) override
	{
		timer.update();
		frames++;

		static float averageDeltaTime = 0;
		averageDeltaTime += timer.deltaTime;
		if (frames % 1000 == 0) 
		{
			averageDeltaTime /= 1000;
			axInfoFmt("Avg. Frame Time: {}\n\t\tFPS: {}", averageDeltaTime, 1. / averageDeltaTime);
			averageDeltaTime = 0;
		}

		dispatchComputeShader();

		auto app = apex::Application::Instance();
		auto& commandList = app->getRenderer()->getCurrentCommandList();

		commandList.clear();

		apex::gfx::DrawCommand drawCommand;
		Matrix4x4 transform = Matrix4x4::identity();
		transform = scale(transform, 0.1f);

		/*for (auto& p : cloth.positions)
		{
			drawCommand.transform = translate(transform, p);
			drawCommand.pMesh = &mesh;
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}*/
		
		{
			/*auto vertices = static_cast<apex::gfx::Vertex_P0_C0*>(dynamicMesh.getVertexBuffer().m_buffer.getMappedMemory());
			for (size_t i = 0; i < cloth.positions.size(); i++)
			{
				vertices[i].position = cloth.positions[i];
			}*/
			
			drawCommand.transform = Matrix4x4::identity();
			drawCommand.pMesh = &dynamicMesh;
			commandList.addCommand<apex::gfx::DrawCommand>(apex::make_unique<apex::gfx::DrawCommand>(drawCommand));
		}

		commandList.sortCommands();
	}

	void stop() override
	{
		auto& renderer = *apex::Application::Instance()->getRenderer();

		//if (mesh) mesh.destroy(renderer.getContext().m_device, nullptr);
		if (dynamicMesh) dynamicMesh.destroy(renderer.getContext().m_device, nullptr);

		auto& device = renderer.getContext().m_device;

		clothBuffer.destroy(device, VK_NULL_HANDLE);

		vkDestroyFence(device.logicalDevice, m_computeFence, VK_NULL_HANDLE);
		vkDestroyDescriptorPool(device.logicalDevice, m_computeDescriptorPool, VK_NULL_HANDLE);
		m_computeDescriptorSetLayout.destroy(device.logicalDevice, VK_NULL_HANDLE);
		vkFreeCommandBuffers(device.logicalDevice, m_computeCommandPool, 1, &m_computeCommandBuffer);
		vkDestroyCommandPool(device.logicalDevice, m_computeCommandPool, VK_NULL_HANDLE);
		m_comp_initAccelerations.destroy(device.logicalDevice, VK_NULL_HANDLE);
		m_comp_springForces.destroy(device.logicalDevice, VK_NULL_HANDLE);
		m_comp_applyForces.destroy(device.logicalDevice, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(device.logicalDevice, m_comp_initAccelerations.pipelineLayout, VK_NULL_HANDLE);
	}

protected:

	struct PushConstants
	{
		u32 rows, cols, springs;
		f32 currentTime, deltaTime;
		f32 kElastic, kDamping;
	};

	void initializeVulkanObjects()
	{
		apex::vk::VulkanDevice& device = apex::Application::Instance()->getRenderer()->getContext().m_device;

		#pragma region Create compute pipeline
		{ // Create compute pipeline
			apex::vk::VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
			descriptorSetLayoutBuilder
				.setBindingCount(4)
				.addStorageBuffer(0, 1, VK_SHADER_STAGE_COMPUTE_BIT)
				.addStorageBuffer(1, 1, VK_SHADER_STAGE_COMPUTE_BIT)
				.addStorageBuffer(2, 1, VK_SHADER_STAGE_COMPUTE_BIT)
				.addStorageBuffer(3, 1, VK_SHADER_STAGE_COMPUTE_BIT)
			;
			m_computeDescriptorSetLayout = descriptorSetLayoutBuilder.build(device.logicalDevice, VK_NULL_HANDLE);

			VkPushConstantRange pushConstantRange
			{
				.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
				.offset = 0,
				.size = sizeof(PushConstants),
			};

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = 1,
				.pSetLayouts = &m_computeDescriptorSetLayout.layout,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &pushConstantRange,
			};

			VkPipelineLayout pipelineLayout;
			axVerifyFmt(VK_SUCCESS == vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout),
			            "Failed to create pipeline layout"
			);

			{
				apex::vk::VulkanComputeShader computeShader;
				computeShader.create(device.logicalDevice, "X:\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\cloth_init.comp.spv", VK_NULL_HANDLE);
				m_comp_initAccelerations.create(device, pipelineLayout, computeShader, VK_NULL_HANDLE);
				computeShader.destroy(device.logicalDevice, VK_NULL_HANDLE);
			}
			{
				apex::vk::VulkanComputeShader computeShader;
				computeShader.create(device.logicalDevice, "X:\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\cloth_spring.comp.spv", VK_NULL_HANDLE);
				m_comp_springForces.create(device, pipelineLayout, computeShader, VK_NULL_HANDLE);
				computeShader.destroy(device.logicalDevice, VK_NULL_HANDLE);
			}
			{
				apex::vk::VulkanComputeShader computeShader;
				computeShader.create(device.logicalDevice, "X:\\ApexGameEngine-Vulkan\\build-msvc\\Graphics\\spv\\cloth_applyForces.comp.spv", VK_NULL_HANDLE);
				m_comp_applyForces.create(device, pipelineLayout, computeShader, VK_NULL_HANDLE);
				computeShader.destroy(device.logicalDevice, VK_NULL_HANDLE);
			}


		}
		#pragma endregion

		#pragma region Create compute command pool
		{ // Create compute command pool
			VkCommandPoolCreateInfo commandPoolCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = device.queueFamilyIndices.computeFamily.value_or(0),
			};

			axVerifyFmt(VK_SUCCESS == vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &m_computeCommandPool),
			            "Failed to create compute command pool"
			);

			VkCommandBufferAllocateInfo commandBufferAllocateInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = m_computeCommandPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			axVerifyFmt(VK_SUCCESS == vkAllocateCommandBuffers(device.logicalDevice, &commandBufferAllocateInfo, &m_computeCommandBuffer),
			            "Failed to allocate compute command buffer"
			);
		}
		#pragma endregion

		#pragma region Create compute fence
		{ // Create compute fence
			VkFenceCreateInfo fenceCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};

			axVerifyFmt(VK_SUCCESS == vkCreateFence(device.logicalDevice, &fenceCreateInfo, VK_NULL_HANDLE, &m_computeFence),
			            "Failed to create compute fence"
			);
		}
		#pragma endregion


		#pragma region Create compute descriptor pool
		{ // Create compute descriptor pool
			VkDescriptorPoolSize poolSizes[]
			{
				{
					.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.descriptorCount = 4,
				},
				{
					.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					.descriptorCount = 1,
				}
			};

			VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.maxSets = 1,
				.poolSizeCount = static_cast<u32>(std::size(poolSizes)),
				.pPoolSizes = poolSizes,
			};

			axVerifyFmt(VK_SUCCESS == vkCreateDescriptorPool(device.logicalDevice, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &m_computeDescriptorPool),
			            "Failed to create compute descriptor pool"
			);
		}
		#pragma endregion

		const size_t clothSpringsSize = math::round_to(cloth.springs.size() * sizeof(Cloth::Spring), 16ui64);
		const size_t clothVelocitiesSize = cloth.nParticles * sizeof(math::Vector4);
		const size_t clothAccelerationsSize = cloth.nParticles * sizeof(math::Vector4);
		const size_t clothBufferSize = clothSpringsSize + clothVelocitiesSize + clothAccelerationsSize;
		#pragma region Create spring buffer
		{ // Create spring buffer
			u32 queueFamilyIndices[] = { device.queueFamilyIndices.computeFamily.value() };
			clothBuffer.create(device,
				clothBufferSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				apex::make_array_ref(queueFamilyIndices),
				VMA_MEMORY_USAGE_GPU_ONLY,
				0,
				VK_NULL_HANDLE);

			apex::vk::VulkanBuffer stagingBuffer;
			stagingBuffer.createStagingBuffer(device, clothBufferSize, VK_NULL_HANDLE);

			apex::memcpy_s<Cloth::Spring>(stagingBuffer.getMappedMemory(), stagingBuffer.allocation_info.size / sizeof(Cloth::Spring), cloth.springs.data(), cloth.springs.size());

			{ // Copy velocities
				void *pData = static_cast<apex::u8*>(stagingBuffer.getMappedMemory()) + clothSpringsSize;
				for (auto vel : cloth.velocities)
				{
					*static_cast<math::Vector4*>(pData) = { vel.x, vel.y, vel.z, 0 };
					pData = static_cast<apex::u8*>(pData) + sizeof(math::Vector4);
				}
			}

			{ // Copy accelerations
				void *pData = static_cast<apex::u8*>(stagingBuffer.getMappedMemory()) + clothSpringsSize + clothVelocitiesSize;
				::memset(pData, 0, clothAccelerationsSize);
			}

			apex::vk::VulkanBuffer::CopyBufferData(device, clothBuffer, stagingBuffer, clothBufferSize);

			stagingBuffer.destroy(device, VK_NULL_HANDLE);
		}
		#pragma endregion

		#pragma region Create compute descriptor set
		{ // Create compute descriptor set
			VkDescriptorSetAllocateInfo alloc_info
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = m_computeDescriptorPool,
				.descriptorSetCount = 1,
				.pSetLayouts = &m_computeDescriptorSetLayout.layout,
			};

			axVerifyFmt(VK_SUCCESS == vkAllocateDescriptorSets(device.logicalDevice, &alloc_info, &m_computeDescriptorSet),
			            "Failed to allocate descriptor set"
			);

			VkDescriptorBufferInfo bufferInfos[]
			{
				{
					.buffer = dynamicMesh.getVertexBuffer().m_buffer.buffer,
					.offset = 0,
					.range = VK_WHOLE_SIZE,
				},
				{
					.buffer = clothBuffer.buffer,
					.offset = 0,
					.range = sizeof(Cloth::Spring) * cloth.springs.size(),
				},
				{
					.buffer = clothBuffer.buffer,
					.offset = clothSpringsSize,
					.range = clothVelocitiesSize,
				},
				{
					.buffer = clothBuffer.buffer,
					.offset = clothSpringsSize + clothVelocitiesSize,
					.range = clothAccelerationsSize,
				}
			};

			VkWriteDescriptorSet descriptorSetWrites[]
			{
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_computeDescriptorSet,
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &bufferInfos[0],
				},
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_computeDescriptorSet,
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &bufferInfos[1],
				},
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_computeDescriptorSet,
					.dstBinding = 2,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &bufferInfos[2],
				},
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = m_computeDescriptorSet,
					.dstBinding = 3,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.pBufferInfo = &bufferInfos[3],
				}
			};

			vkUpdateDescriptorSets(device.logicalDevice, std::size(descriptorSetWrites), descriptorSetWrites, 0, nullptr);
		}
		#pragma endregion
	}

	void dispatchComputeShader()
	{
		const auto& device = apex::Application::Instance()->getRenderer()->getContext().m_device;
		auto& computeQueue = device.computeQueue;
		vkWaitForFences(device.logicalDevice, 1, &m_computeFence, VK_TRUE, 1'000'000'000 /* ns */);
		vkResetFences(device.logicalDevice, 1, &m_computeFence);

		//cloth.simulate(timer.getDeltaTime());

		vkResetCommandBuffer(m_computeCommandBuffer, 0);

		VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr,
		};

		vkBeginCommandBuffer(m_computeCommandBuffer, &commandBufferBeginInfo);

		const PushConstants pushConstants
		{
			Cloth::rows, Cloth::cols, (u32)cloth.springs.size() /*(cloth.nStructural + cloth.nShear)*/,
			timer.getCurrentTime(), timer.getDeltaTime(),
			cloth.kElastic, cloth.kDamping
		};

		VkMemoryBarrier2 memoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		};

		VkDependencyInfo dependencyInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = 1,
			.pMemoryBarriers = &memoryBarrier,
		};

		static float acc = 0;
		acc += timer.deltaTime;
		acc = apex::min(acc, 0.004f);
		float dt = 0.0005f;

		// Compute: CreateContext accelerations
		cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_initAccelerations, m_computeDescriptorSet, pushConstants, math::ceil(cloth.nParticles, 16));
		vkCmdPipelineBarrier2(m_computeCommandBuffer, &dependencyInfo);

		// Compute: Compute spring forces
		cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_springForces, m_computeDescriptorSet, pushConstants, math::ceil(cloth.springs.size(), 16ui64));
		vkCmdPipelineBarrier2(m_computeCommandBuffer, &dependencyInfo);

		// Compute: Apply forces
		cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_applyForces, m_computeDescriptorSet, pushConstants, math::ceil(cloth.nParticles, 16));
		acc -= dt;

		while (acc > dt + apex::Constants::float32_EPSILON)
		{
			// Compute: CreateContext accelerations
			vkCmdPipelineBarrier2(m_computeCommandBuffer, &dependencyInfo);
			cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_initAccelerations, m_computeDescriptorSet, pushConstants, math::ceil(cloth.nParticles, 16));

			// Compute: Compute spring forces
			vkCmdPipelineBarrier2(m_computeCommandBuffer, &dependencyInfo);
			cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_springForces, m_computeDescriptorSet, pushConstants, math::ceil(cloth.springs.size(), 16ui64));

			// Compute: Apply forces
			vkCmdPipelineBarrier2(m_computeCommandBuffer, &dependencyInfo);
			cmdDispatchComputeShader(m_computeCommandBuffer, m_comp_applyForces, m_computeDescriptorSet, pushConstants, math::ceil(cloth.nParticles, 16));

			acc -= dt;
		}

		vkEndCommandBuffer(m_computeCommandBuffer);

		VkCommandBufferSubmitInfo commandBufferSubmitInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
			.pNext = nullptr,
			.commandBuffer = m_computeCommandBuffer,
			.deviceMask = 0,
		};

		VkSubmitInfo2 submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
			.pNext = nullptr,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &commandBufferSubmitInfo,
		};

		VkResult result = vkQueueSubmit2(computeQueue, 1, &submitInfo, m_computeFence);
		axAssertFmt(result == VK_SUCCESS, "Failed to submit compute command buffer");
	}

	static void cmdDispatchComputeShader(VkCommandBuffer command_buffer, apex::vk::VulkanComputePipeline& compute, VkDescriptorSet descriptor_set, PushConstants const& push_constants, int groupCountX)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &descriptor_set, 0, nullptr);
		vkCmdPushConstants(command_buffer, compute.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push_constants);
		vkCmdDispatch(command_buffer, groupCountX, 1, 1);
	}

private:
	apex::gfx::Camera m_camera;
	// apex::gfx::StaticMesh mesh {};
	apex::gfx::DynamicMesh dynamicMesh {};

	apex::vk::VulkanBuffer clothBuffer;

	apex::vk::VulkanComputePipeline m_comp_initAccelerations;
	apex::vk::VulkanComputePipeline m_comp_springForces;
	apex::vk::VulkanComputePipeline m_comp_applyForces;
	apex::vk::VulkanDescriptorSetLayout m_computeDescriptorSetLayout;
	VkCommandPool m_computeCommandPool{};
	VkCommandBuffer m_computeCommandBuffer{};
	VkDescriptorPool m_computeDescriptorPool{};
	VkDescriptorSet m_computeDescriptorSet{};
	VkFence m_computeFence{};

	Matrix4x4 m_cameraTransform;

	int frames = 0;
	Cloth cloth;

	Timer timer;
};
