#pragma once
#include "Containers/AxStringRef.h"

namespace apex {

	struct ModelMetadata
	{
		AxStringRef path;
		uint32      numVertices;
		uint32      numIndices;
		uint32      numMeshes;
		uint32      numMaterials;
		uint32      numTextures;
		bool        hasNormals;
		bool        hasTangents;
	};

	struct Model
	{

		ModelMetadata * m_metadata;
	};

	// Import Options for loading a model (gltf/obj/fbx)
	struct ModelImportOptions
	{
		bool flipUVs;
		bool flipWindingOrder;
		bool generateNormals;
		bool generateTangents;
		bool generateBoundingBox;
		bool generateCollider;
		bool importMaterials;
		bool importTextures;
		bool combineMeshes;
	};

	class ModelLoader
	{
	public:
		ModelLoader(ModelImportOptions options = {}):  m_options(options) {}
		~ModelLoader() = default;
		
		ModelMetadata readMetadata(AxStringRef path) const;

	private:
		ModelImportOptions m_options;
	};

}
