#include "apex_pch.h"
#include "Apex/ModelLoader.h"

#include <fastgltf/core.hpp>

namespace apex {

	std::string recurseGltfNode(fastgltf::Asset const& asset, size_t node_index, uint32 depth = 0)
	{
		std::string indent(depth, ' ');

		std::stringstream ss;
		ss << "\n";

		auto& node = asset.nodes[node_index];
		ss << fmt::format("{}Node: {}\n", indent, node.name);
		if (node.meshIndex.has_value()) {
			auto& mesh = asset.meshes[node.meshIndex.value()];
			ss << fmt::format("{}  Mesh: {}\n", indent, mesh.name);
			for (auto& primitive : mesh.primitives) {
				ss << fmt::format("{}    Primitive: {}\n", indent, (uint32)primitive.type);
				ss << fmt::format("{}      Indices: {}\n", indent, primitive.indicesAccessor.value_or(-1));
				ss << fmt::format("{}        Count: {}\n", indent, asset.accessors[primitive.indicesAccessor.value()].count);
				for (auto& [attribute, accessor] : primitive.attributes) {
					ss << fmt::format("{}      Attribute: {}\n", indent, attribute);
					ss << fmt::format("{}        Accessor: {}\n", indent, accessor);
				}
			}
		}

		for (auto childIndex : node.children) {
			ss << recurseGltfNode(asset, childIndex, depth+1);
		}

		return ss.str();
	}

	ModelMetadata ModelLoader::readMetadata(AxStringRef path) const
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer data;
		std::filesystem::path fpath(path.c_str());
		data.loadFromFile(fpath);

		auto expected = parser.loadGltf(&data, fpath.parent_path(), fastgltf::Options::None);
		if (auto error = expected.error(); error != fastgltf::Error::None) {
			axErrorFmt("Failed to load gltf file: {}", (uint32)error);
			return {};
		}

		axCheck(fastgltf::validate(expected.get()) == fastgltf::Error::None);

		auto& asset = expected.get();

		auto& scene = asset.scenes[asset.defaultScene.value_or(0)];
		for (auto nodeIndex : scene.nodeIndices)
		{
			auto str = recurseGltfNode(asset, nodeIndex);
			axInfoFmt("GLTF Scene:{}", str);
		}

		return {};
	}

}
