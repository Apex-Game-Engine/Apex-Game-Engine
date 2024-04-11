#pragma once
#include "Containers/AxArray.h"
#include "Math/Matrix4x4.h"

namespace apex::vk
{
	struct VulkanComputePipeline;
}


namespace apex {
namespace gfx {
	struct Mesh;
	struct StaticMesh;

	struct Command : public AxManagedClass
	{
		enum Type
		{
			Draw,
			Compute,
			State
		};

		Type type;

		Command(Type type) : type(type) { }
	};

	struct DrawCommand : public Command
	{
		DrawCommand() : Command(Type::Draw) { }

		Mesh* pMesh {};
		math::Matrix4x4 transform;
		uint32 instanceCount { 1 };
	};

	struct ComputeCommand : public Command
	{
		ComputeCommand() : Command(Type::Compute) { }

		vk::VulkanComputePipeline* pPipeline {};

	};

	struct StateCommand : public Command
	{
		StateCommand() : Command(Type::State) { }
	};

	class CommandList
	{
	public:
		using command_list_t = AxArray<UniquePtr<Command>>;

		CommandList() = default;
		~CommandList() = default;

		template <typename T> requires std::is_base_of_v<Command, T>
		void addCommand(UniquePtr<T>&& command)
		{
			_addCommand(std::forward<UniquePtr<T>>(command));
		}

		void _addCommand(UniquePtr<Command>&& command);
		void clear();

		auto& getCommands() { return m_commands; }
		void sortCommands();

	private:
		command_list_t m_commands;
	};

}
}

