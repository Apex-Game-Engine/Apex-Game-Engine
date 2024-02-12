#include "Graphics/CommandList.h"

namespace apex::gfx {

	void CommandList::_addCommand(UniquePtr<Command>&& command)
	{
		m_commands.append(std::move(command));
	}

	void CommandList::clear()
	{
		m_commands.clear();
	}

	void CommandList::sortCommands()
	{
		std::sort(m_commands.begin(), m_commands.end(), [](auto const& lhs, auto const& rhs)
		{
			if (lhs->type != rhs->type)
				return lhs->type < rhs->type;

			if (lhs->type == Command::Type::Draw)
			{
				auto const& lhsDraw = static_cast<DrawCommand const&>(*lhs);
				auto const& rhsDraw = static_cast<DrawCommand const&>(*rhs);

				return lhsDraw.pMesh < rhsDraw.pMesh;
			}
			axAssertMsg(false, "Not implemented!");
			// TODO: Implement sorting for other command types
			return &lhs < &rhs;
		});
	}
}
