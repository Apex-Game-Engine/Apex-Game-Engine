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

}
