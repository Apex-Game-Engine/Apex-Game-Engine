#pragma once

#include "Containers/AxArray.h"
#include "String/AxString.h"

#include <array>

namespace apex {
namespace asset {

	struct PipelineDesc
	{
		enum ShaderTypes
		{
			Vertex, Fragment, Compute, COUNT
		};

		AxString name;
		std::array<AxString, COUNT> shaders;

	};

	typedef AxArray<PipelineDesc> PipelineDescs;

	class PipelineLoader
	{
	public:
		static PipelineDescs ReadFromFile(const char* filename);
	};

}
}
