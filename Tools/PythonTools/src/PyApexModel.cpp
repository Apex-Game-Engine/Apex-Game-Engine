#include <pybind11/pybind11.h>

#include <filesystem>
#include <MeshSerializer.h>

namespace py = pybind11;

template <typename Type>
py::list getList(const std::vector<Type>& inVector)
{
	py::list outList(inVector.size());
	for (uint32_t i = 0; i < inVector.size(); i++)
	{
		outList[i] = inVector[i];
	}
	return outList;
}

template <typename Type>
py::list getList(const Type* inArray, size_t inSize)
{
	py::list outList(inSize);
	for (uint32_t i = 0; i < inSize; i++)
	{
		outList[i] = inArray[i];
	}
	return outList;
}

template <typename Type>
void setList(std::vector<Type>& outVector, py::list inList)
{
	outVector.resize(inList.size());
	for (size_t i = 0; i < outVector.size(); i++)
	{
		outVector[i] = inList[i].cast<Type>();
	}
}

template <typename Type>
void setList(Type** outArray, size_t* outSize, py::list inList)
{
	*outSize = inList.size();
	*outArray = new Type[inList.size()];
	for (size_t i = 0; i < inList.size(); i++)
	{
		(*outArray)[i] = inList[i].cast<Type>();
	}
}

constexpr static uint32_t getSizeForAttribute(AxMeshAttributeType attr)
{
	switch (attr)
	{
	case AxMeshAttribute_Position:		return 3 * sizeof(float);
	case AxMeshAttribute_Normal:		return 3 * sizeof(float);
	case AxMeshAttribute_Tangent:		return 3 * sizeof(float);
	case AxMeshAttribute_TexCoords0:	return 2 * sizeof(float);
	case AxMeshAttribute_TexCoords1:	return 2 * sizeof(float);
	case AxMeshAttribute_Color:			return 4 * sizeof(float);
	case AxMeshAttribute_BoneWeights:	return 4 * sizeof(float);
	case AxMeshAttribute_BoneIndices:	return 4 * sizeof(uint32_t);
	}
}

class PyMeshData;

class PyMeshData
{
public:
	using StreamList = std::vector<AxMeshStream>;
	using AttributeList = std::vector<AxMeshAttribute>;
	using IndexList = std::vector<uint32_t>;
	using VertexList = std::vector<float>;

	PyMeshData() = default;

	py::list getStreams() const
	{
		return getList(streams);
	}

	py::list getAttributes() const
	{
		return getList(attributes);
	}

	py::list getIndices() const
	{
		return getList(indices);
	}

	py::list getVertices() const
	{
		return getList(vertices);
	}

	void setStreams(py::list streamList)
	{
		setList(streams, streamList);
	}

	void setAttributes(py::list attributeList)
	{
		setList(attributes, attributeList);
	}

	void setIndices(py::list indexList)
	{
		setList(indices, indexList);
	}

	void setVertices(py::list vertexList)
	{
		setList(vertices, vertexList);
	}

	void newStream(uint32_t attribute_start, uint32_t attribute_count)
	{
		streams.emplace_back(
			getStride(attributes.begin() + attribute_start, attributes.begin() + attribute_start + attribute_count),
			attribute_start, attribute_count,
			streams.size() > 0 ? (streams.back().dataOffset + streams.back().stride * vertexCount) : 0);
	}

	size_t exportToFile(const std::string& path)
	{
		uint32_t vertexStride = getVertexStride();
		if (vertices.size() * sizeof(float) != static_cast<size_t>(vertexCount) * vertexStride)
			throw std::runtime_error("Something is wrong with the data. len(vertices) must be equal to (vertexCount * vertexStride)");

		const AxMeshData meshData {
			.vertexStride = getVertexStride(),
			.streamCount = static_cast<uint32_t>(streams.size()),
			.attributeCount = static_cast<uint32_t>(attributes.size()),
			.indexCount = static_cast<uint32_t>(indices.size()),
			.vertexCount = vertexCount,
			.pStreams = streams.data(),
			.pAttributes = attributes.data(),
			.pIndices = indices.data(),
			.pVertices = vertices.data(),
		};
		return axMeshSaveFile(path.c_str(), {}, &meshData);
	}

	static uint32_t getStride(AttributeList::const_iterator begin, AttributeList::const_iterator end)
	{
		uint32_t stride = 0;
		for (auto it = begin; it != end; ++it)
		{
			stride += getSizeForAttribute(*it);
		}
		return stride;
	}

	uint32_t getVertexStride() const
	{
		return getStride(attributes.begin(), attributes.end());
	}

private:
	StreamList streams;
	AttributeList attributes;
	IndexList indices;
	VertexList vertices;
public:
	uint32_t vertexCount;

	friend class PyImmutableMeshData;
};

class PyImmutableMeshData
{
public:
	static PyImmutableMeshData importFromFile(const std::string& path)
	{
		AxMeshData* wrapped = axMeshLoadFile(path.c_str(), {});
		if (!wrapped)
		{
			throw std::runtime_error("Could not import mesh file: " + path);
		}
		return PyImmutableMeshData { wrapped };
	}

	PyMeshData cloneToMutable() const
	{
		PyMeshData mutableMesh;
		try {
			mutableMesh.streams.resize(_wrapped->streamCount);
			printf("%u streams\n", _wrapped->streamCount);
			mutableMesh.attributes.resize(_wrapped->attributeCount);
			printf("%u attributes\n", _wrapped->attributeCount);
			mutableMesh.indices.resize(_wrapped->indexCount);
			printf("%u indices\n", _wrapped->indexCount);
			mutableMesh.vertices.resize(_wrapped->vertexCount);
			printf("%u vertices\n", _wrapped->vertexCount);
			std::copy_n(_wrapped->pStreams, _wrapped->streamCount, mutableMesh.streams.begin());
			std::copy_n(_wrapped->pAttributes, _wrapped->attributeCount, mutableMesh.attributes.begin());
			std::copy_n(_wrapped->pIndices, _wrapped->indexCount, mutableMesh.indices.begin());
			std::copy_n((float*)_wrapped->pVertices, _wrapped->vertexCount * _wrapped->vertexStride / sizeof(float), mutableMesh.vertices.begin());
		}
		catch (std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
		return mutableMesh;
	}

	py::list getStreams() const
	{
		return getList(_wrapped->pStreams, _wrapped->streamCount);
	}

	py::list getAttributes() const
	{
		return getList(_wrapped->pAttributes, _wrapped->attributeCount);
	}

	py::list getIndices() const
	{
		return getList(_wrapped->pIndices, _wrapped->indexCount);
	}

	py::list getVertices() const
	{
		return getList(static_cast<float*>(_wrapped->pVertices), _wrapped->vertexCount * _wrapped->vertexStride / sizeof(float));
	}

	uint32_t getStride() const
	{
		return _wrapped->vertexStride;
	}

private:
	PyImmutableMeshData(AxMeshData* wrapped) : _wrapped(wrapped) {}

private:
	AxMeshData* _wrapped {};
};

PYBIND11_MODULE(ApexModel, m)
{
	m.doc() = "Apex Mesh Exporter";

	py::enum_<AxMeshAttributeType>(m, "MeshAttributeType")
		.value("Position", AxMeshAttribute_Position)
		.value("Normal", AxMeshAttribute_Normal)
		.value("Tangent", AxMeshAttribute_Tangent)
		.value("TexCoords0", AxMeshAttribute_TexCoords0)
		.value("TexCoords1", AxMeshAttribute_TexCoords1)
		.value("Color", AxMeshAttribute_Color)
		.value("BoneWeights", AxMeshAttribute_BoneWeights)
		.value("BoneIndices", AxMeshAttribute_BoneIndices)
		.export_values();

	//py::class_<AxMeshAttribute>(m, "MeshAttribute")
	//	.def(py::init<>())
	//	.def(py::init<AxMeshAttributeType>())
	//	.def_readwrite("attributeType", &AxMeshAttribute::attributeType);

	py::class_<AxMeshStream>(m, "MeshStream")
		.def(py::init<>())
		.def(py::init<uint32_t, uint32_t, uint32_t>())
		.def(py::init<uint32_t, uint32_t, uint32_t, uint32_t>())
		.def_readwrite("stride", &AxMeshStream::stride)
		.def_readwrite("attributeStart", &AxMeshStream::attributeStart)
		.def_readwrite("attributeCount", &AxMeshStream::attributeCount)
		.def_readwrite("dataOffset", &AxMeshStream::dataOffset)
		.def("__repr__", [](const AxMeshStream& stream) { return std::format("MeshStream ({}, {}, {}, {})", stream.stride, stream.attributeStart, stream.attributeCount, stream.dataOffset); });

	py::class_<PyMeshData>(m, "Mesh")
		.def(py::init<>())
		.def_readwrite("vertexCount", &PyMeshData::vertexCount)
		.def_property_readonly("stride", &PyMeshData::getVertexStride)
		.def_property("streams", &PyMeshData::getStreams, &PyMeshData::setStreams)
		.def_property("attributes", &PyMeshData::getAttributes, &PyMeshData::setAttributes)
		.def_property("indices", &PyMeshData::getIndices, &PyMeshData::setIndices)
		.def_property("vertices", &PyMeshData::getVertices, &PyMeshData::setVertices)
		.def("new_stream", &PyMeshData::newStream)
		.def("export", &PyMeshData::exportToFile);

	py::class_<PyImmutableMeshData>(m, "ImmutableMesh")
		.def("clone_mutable", &PyImmutableMeshData::cloneToMutable)
		.def_property_readonly("stride", &PyImmutableMeshData::getStride)
		.def_property_readonly("streams", &PyImmutableMeshData::getStreams)
		.def_property_readonly("attributes", &PyImmutableMeshData::getAttributes)
		.def_property_readonly("indices", &PyImmutableMeshData::getIndices)
		.def_property_readonly("vertices", &PyImmutableMeshData::getVertices);

	m.def("import_mesh", &PyImmutableMeshData::importFromFile);
}
