#pragma once
#include "Memory/AxManagedClass.h"

namespace apex {

	/**
	 * \brief Non-resizable dynamic array that does not guarantee order of elements
	 *
	 *  - insertions and deletions in middle of array do not preserve ordering.
	 *	- append and pop from end preserve ordering.
	 *	\n For ordered insertions and deletions, use `AxArray`
	 * \tparam StoredType type of elements stored in the array
	 */
	template <typename StoredType>
	class AxUnorderedArray : public AxManagedClass
	{
		friend class AxUnorderedArrayTest;
	};

}
