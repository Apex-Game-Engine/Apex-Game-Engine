#include "Asset/Pipeline.h"

#include "Core/Files.h"

namespace apex::asset {

	bool ReadFileContents(const char* filename, AxArray<char>& buffer)
	{
		File file = File::OpenExisting(filename);
		if (!file) return false;
		size_t size = file.GetSize();
		buffer.resize(size);
		return file.Read(buffer.dataMutable(), size);
	}

	PipelineDescs PipelineLoader::ReadFromFile(const char* filename)
	{
		PipelineDescs pipelines(1);
		AxArray<char> buffer;
		if (!axVerify(ReadFileContents(filename, buffer)))
			return {};

		AxStringView strview { buffer.data(), buffer.size() };

		enum Stage { eGlobal /* no active context */, ePipeline /* pipeline context */ } stage = eGlobal;
		PipelineDesc* pActivePipeline = nullptr;

		for (Strings::LineIterator lineIt { strview }; lineIt; ++lineIt)
		{
			AxStringView line = *lineIt;

			if (line == "[pipeline]"sv)
			{
				if (pipelines.size() == pipelines.capacity())
					pipelines.reserve(pipelines.size() + 2);
				pActivePipeline = &pipelines.emplace_back();
				stage = ePipeline;
				continue;
			}

			switch (stage)
			{
			case eGlobal:
				{
					break;
				}
			case ePipeline:
				{
					axAssertFmt(pActivePipeline, "Expected a [pipeline] before this!");
					size_t eq = line.find('=');
					AxStringView key = line.substr(0, eq);
					AxStringView value = line.substr(eq+1);
					if (key == "name"sv)
					{
					#if APEX_DEBUG
						if (pActivePipeline->name)
							axDebugFmt("Pipeline name is being overwritten; was \"{}\" now \"{}\"", pActivePipeline->name, value);
					#endif // APEX_DEBUG
						pActivePipeline->name = value;
					}
					else if (key == "shaders.vertex"sv)
					{
					#if APEX_DEBUG
						if (pActivePipeline->shaders[PipelineDesc::Vertex])
							axDebugFmt("Pipeline Vertex shader is being overwritten; was \"{}\" now \"{}\"", pActivePipeline->shaders[PipelineDesc::Vertex], value);
					#endif // APEX_DEBUG
						pActivePipeline->shaders[PipelineDesc::Vertex] = value;
					}
					else if (key == "shaders.fragment"sv)
					{
					#if APEX_DEBUG
						if (pActivePipeline->shaders[PipelineDesc::Fragment])
							axDebugFmt("Pipeline Fragment shader is being overwritten; was \"{}\" now \"{}\"", pActivePipeline->shaders[PipelineDesc::Fragment], value);
					#endif // APEX_DEBUG
						pActivePipeline->shaders[PipelineDesc::Fragment] = value;
					}		
					break;
				}
			}

			
		}

		return pipelines;
	}

}
