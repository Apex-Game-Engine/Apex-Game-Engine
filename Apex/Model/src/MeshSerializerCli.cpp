#include <algorithm>
#include <string_view>

#include "MeshSerializer.h"

enum Result
{
	SUCCESS = 0,
	ERROR_INVALID_ARGS
};

struct 
{
	char short_opt;
	const char* long_opt;
	const char* help;
	char* value;
} g_options[] = {
	{ 'o', "out", "Output file name" },
	{ 't', "tri", "Triangulate the mesh" },
	{ 'O', "opt", "Optimization flags - " },
};

Result getopts(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (char* val = strchr(argv[i], '='))
		{
			*val = 0;
			size_t o;
			if (val - argv[i] == 1)
			{
				for (o = 0; o < std::size(g_options); o++)
					if (g_options[o].short_opt == argv[i][0])
						break;
			}
			else
			{
				for (o = 0; o < std::size(g_options); o++)
					if (strcmp(g_options[o].long_opt, argv[i]) == 0)
						break;
			}
			if (o == std::size(g_options))
				return ERROR_INVALID_ARGS;

			g_options[o].value = val + 1;
		}
		else
		{
			size_t o;
			if (strlen(argv[i]) == 1)
			{
				for (o = 0; o < std::size(g_options); i++)
					if (g_options[o].short_opt == argv[i][0])
						break;
			}
			else
			{
				for (o = 0; o < std::size(g_options); o++)
					if (strcmp(g_options[o].long_opt, argv[i]) == 0)
						break;
			}
			if (o == std::size(g_options))
				return ERROR_INVALID_ARGS;

			g_options[o].value = argv[i + 1];
			i++;
		}
	}

	return SUCCESS;
}


int main(int argc, char* argv[])
{
	Result result;
	if (argc < 2)
	{
		return ERROR_INVALID_ARGS;
	}
	if ((result = getopts(argc, argv)) != SUCCESS)
	{
		return result;
	}


	{
		AxMeshAttribute attributes[] = {
			AxMeshAttribute_Position,
			AxMeshAttribute_TexCoords0,
		};

		typedef float Vertex[5];

		Vertex vertices[] = {
			{ 0.0, 0.0, 0.0, 0.0, 0.0 },
			{ 1.0, 0.0, 0.0, 0.0, 1.0 },
			{ 1.0, 1.0, 0.0, 1.0, 0.0 },
			{ 0.0, 1.0, 1.0, 1.0, 0.0 },
			{ 1.0, 0.0, 1.0, 1.0, 1.0 },
		};

		uint32_t indices[] = {
			0, 1, 2, 3, 4
		};

		AxMeshBuffers meshBuffers {
			.stride = 5 * sizeof(Vertex),
			.attributeCount = std::size(attributes),
			.pAttributes = attributes,
			.vertexCount = std::size(vertices),
			.pVertices = vertices[0],
			.indexCount = std::size(indices),
			.pIndices = indices,
		};

		size_t bytes = axMeshSaveFile("X:\\ApexGameEngine-Vulkan\\MeshDump.bin", {}, &meshBuffers);
	}

	{
		AxMeshBuffers* pMeshBuffers;
		bool success = axMeshLoadFile("X:\\ApexGameEngine-Vulkan\\MeshDump.bin", {}, &pMeshBuffers);
		delete pMeshBuffers;
	}

	return result;
}
