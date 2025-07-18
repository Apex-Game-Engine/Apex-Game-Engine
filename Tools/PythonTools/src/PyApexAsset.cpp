#include <pybind11/pybind11.h>

#include <filesystem>
#include <numeric>
#include <ranges>

#include "Asset/Mesh.h"
#include "String/AxStream.h"

struct EarlyInit
{
	EarlyInit()
	{
		axMeshSetUserMemoryCallbacks(malloc, free);
	}

	~EarlyInit() {}
} static g_earlyInit;

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

constexpr static uint32_t getAttributeElementCount(AxMeshAttribute attr)
{
	switch (attr)
	{
	case AxMeshAttribute_Position:		return 3;
	case AxMeshAttribute_Normal:		return 3;
	case AxMeshAttribute_Tangent:		return 3;
	case AxMeshAttribute_TexCoords0:	return 2;
	case AxMeshAttribute_TexCoords1:	return 2;
	case AxMeshAttribute_Color:			return 4;
	case AxMeshAttribute_BoneWeights:	return 4;
	case AxMeshAttribute_BoneIndices:	return 4;
	}
}

constexpr static uint32_t getAttributeSize(AxMeshAttribute attr)
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

constexpr static const char* getAttributeName(AxMeshAttribute attr)
{
	switch (attr)
	{
	case AxMeshAttribute_Position:		return "Position";
	case AxMeshAttribute_Normal:		return "Normal";
	case AxMeshAttribute_Tangent:		return "Tangent";
	case AxMeshAttribute_TexCoords0:	return "TexCoords0";
	case AxMeshAttribute_TexCoords1:	return "TexCoords1";
	case AxMeshAttribute_Color:			return "Color";
	case AxMeshAttribute_BoneWeights:	return "BoneWeights";
	case AxMeshAttribute_BoneIndices:	return "BoneIndices";
	}
}

static AxMeshAttribute getAttributeFromName(const char* attrName)
{
	if (!_stricmp(attrName, "Position"))	return AxMeshAttribute_Position; 
	if (!_stricmp(attrName, "Normal"))		return AxMeshAttribute_Normal;
	if (!_stricmp(attrName, "Tangent"))		return AxMeshAttribute_Tangent;
	if (!_stricmp(attrName, "TexCoords0"))	return AxMeshAttribute_TexCoords0;
	if (!_stricmp(attrName, "TexCoords1"))	return AxMeshAttribute_TexCoords1;
	if (!_stricmp(attrName, "Color"))		return AxMeshAttribute_Color;
	if (!_stricmp(attrName, "BoneWeights"))	return AxMeshAttribute_BoneWeights;
	if (!_stricmp(attrName, "BoneIndices")) return AxMeshAttribute_BoneIndices;
}

class PyMeshStream;
class PyMeshData;
class PyImmutableMeshData;

using PyStreamList = std::vector<PyMeshStream>;
using StreamList = std::vector<AxMeshStream>;
using AttributeList = std::vector<AxMeshAttribute>;
using IndexList = std::vector<uint32_t>;
using VertexData = std::vector<char>;
using VertexStream = apex::AxStream;

#define throw_runtime_error(fmt,...) do { throw std::runtime_error(std::format(fmt, ##__VA_ARGS__)); } while (0)

class PyMeshStream
{
public:
	PyMeshStream() = default;

	py::list GetAttributeList() const
	{
		return getList(m_attributes);
	}

	uint32_t GetVertexCount() const
	{
		return (uint32_t)m_data.size() / CalcStride();
	}

	void SetVertices(const py::dict& attr_vertex_dict)
	{
		bool first = true;
		uint32_t vertexCount = 0;
		std::vector<py::list> attrStreams;
		for (auto [key, val] : attr_vertex_dict)
		{
			if (!py::isinstance<py::str>(key))
				throw_runtime_error("Expected string keys");
			auto attrNameStr = key.cast<std::string>();
			auto attrValues = val.cast<py::list>();
			if (!first && attrValues.size() != vertexCount)
			{
				throw_runtime_error("Incompatible streams. Stream for attribute {} contains {} vertices, expected {}", attrNameStr, attrValues.size(), vertexCount);
			}
			m_attributes.push_back(getAttributeFromName(attrNameStr.c_str()));
			attrStreams.push_back(attrValues);
			vertexCount = attrValues.size();
			first = false;
		}

		const uint32_t stride = CalcStride();
		m_data.resize(stride * vertexCount);
		m_vertices.SetBuffer(m_data.data(), m_data.size());
		for (uint32_t v = 0; v < vertexCount; v++)
		{
			for (uint32_t a = 0; a < attrStreams.size(); a++)
			{
				const uint32_t attrSize = getAttributeElementCount(m_attributes[a]);
				auto tup = attrStreams[a][v].cast<py::tuple>();
				if (tup.size() != attrSize)
					throw_runtime_error("Incompatible vertex attribute elements at index {}. Expected {}, got {}", v, attrSize, tup.size());
				for (auto val : tup)
				{
					auto tmp = val.cast<float>();
					m_vertices.WriteObject<float>(&tmp);
				}
			}
		}
	}

	uint32_t CalcStride() const
	{
		uint32_t stride = 0;
		for (const AxMeshAttributeType attr : m_attributes)
		{
			stride += getAttributeSize(attr);
		}
		return stride;
	}

	uint32_t CalcElementCount() const
	{
		uint32_t elementCount = 0;
		for (const AxMeshAttributeType attr : m_attributes)
		{
			elementCount += getAttributeElementCount(attr);
		}
		return elementCount;
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "Stream (";
		for (const AxMeshAttributeType attr : m_attributes)
		{
			ss << getAttributeName(attr) << ",";
		}
		ss.seekp(-1, ss.cur);
		ss << ")";
		return ss.str();
	}

private:
	AttributeList m_attributes;
	VertexStream m_vertices;
	VertexData m_data;

	friend class PyMeshData;
	friend class PyImmutableMeshData;
};

class PyMeshData
{
public:

	PyMeshData() = default;

	py::list GetStreamList() const
	{
		return getList(m_streams);
	}

	py::list GetUniqueAttributes() const
	{
		size_t attributeCount = 0;
		for (auto& stream : m_streams)
		{
			attributeCount += stream.m_attributes.size();
		}
		py::list outList(attributeCount);
		attributeCount = 0;
		for (const PyMeshStream& stream : m_streams)
		{
			for (const AxMeshAttributeType attr : stream.m_attributes)
			{
				outList[attributeCount++] = attr;
			}
		}
		return outList;
	}

	py::list GetIndexList() const
	{
		return getList(m_indices);
	}

	void SetStreamList(const py::list& stream_list)
	{
		setList(m_streams, stream_list);
	}

	void SetIndexList(const py::list& index_list)
	{
		setList(m_indices, index_list);
	}

	size_t ExportToFile(const std::string& path) const
	{
		uint32_t vertexCount;
		AttributeList attributes = FlattenAttributes();
		StreamList streams = FlattenStreams();
		VertexData vertices = FlattenVertices(vertexCount);

		const AxMeshData meshData {
			.streamCount = static_cast<uint32_t>(m_streams.size()),
			.attributeCount = static_cast<uint32_t>(attributes.size()),
			.indexCount = static_cast<uint32_t>(m_indices.size()),
			.vertexCount = vertexCount,
			.pStreams = streams.data(),
			.pAttributes = attributes.data(),
			.pIndices = const_cast<uint32_t*>(m_indices.data()),
			.pVertices = vertices.data(),
		};

		return axMeshSaveFile(path.c_str(), {}, &meshData);
	}

	uint32_t CalcVertexStride() const
	{
		uint32_t vertexStride = 0;
		for (const PyMeshStream& stream : m_streams)
		{
			vertexStride += stream.CalcStride();
		}
		return vertexStride;
	}

protected:
	AttributeList FlattenAttributes() const
	{
		size_t attributeCount = 0;
		for (auto& stream : m_streams)
		{
			attributeCount += stream.m_attributes.size();
		}

		AttributeList flattened; flattened.reserve(attributeCount);
		for (const PyMeshStream& stream : m_streams)
		{
			for (const AxMeshAttributeType attr : stream.m_attributes)
			{
				if (std::ranges::find(flattened, attr) != flattened.end())
					throw_runtime_error("Attribute '{}' already exists!", getAttributeName(attr));
				flattened.push_back(attr);
			}
		}
		return flattened;
	}

	StreamList FlattenStreams() const
	{
		StreamList streams(m_streams.size());
		uint32_t attributeIdx = 0;
		for (uint32_t si = 0; si < m_streams.size(); si++)
		{
			streams[si].stride = m_streams[si].CalcStride();
			streams[si].attributeStart = attributeIdx;
			streams[si].attributeCount = m_streams[si].m_attributes.size();
			attributeIdx += m_streams[si].m_attributes.size();
		}
		return streams;
	}

	VertexData FlattenVertices(uint32_t& count) const
	{
		count = 0;
		uint32_t totalSize = 0;
		bool first = true;
		for (const PyMeshStream& stream : m_streams)
		{
			const uint32_t streamCount = stream.GetVertexCount();
			if (!first && count != streamCount)
				throw_runtime_error("Mismatch in vertex count of different streams!");
			count = streamCount;
			totalSize += stream.m_data.size();
			first = false;
		}
		VertexData vertices; vertices.reserve(totalSize);
		for (const PyMeshStream& stream : m_streams)
		{
			std::ranges::copy(stream.m_data, std::back_inserter(vertices));
		}
		return vertices;
	}

private:
	PyStreamList m_streams;
	IndexList m_indices;

	friend class PyImmutableMeshData;
};

class PyImmutableMeshData
{
public:
	~PyImmutableMeshData() { axMeshUnload(m_wrapped); }

	static PyImmutableMeshData importFromFile(const std::string& path)
	{
		AxMeshData* wrapped = axMeshLoadFile(path.c_str(), AxMeshLoaderFlags_None);
		if (!wrapped)
		{
			throw_runtime_error("Could not import mesh file: {}", path);
		}
		return PyImmutableMeshData { wrapped };
	}

	PyMeshData CloneToMutable() const
	{
		PyMeshData mutableMesh;
		throw_runtime_error("Not implemented!");
		return mutableMesh;
	}

	py::list GetStreams() const
	{
		return getList(m_wrapped->pStreams, m_wrapped->streamCount);
	}

	py::list GetAttributes() const
	{
		return getList(m_wrapped->pAttributes, m_wrapped->attributeCount);
	}

	py::list GetIndices() const
	{
		return getList(m_wrapped->pIndices, m_wrapped->indexCount);
	}

	py::list GetVertices() const
	{
		return getList(static_cast<float*>(m_wrapped->pVertices), m_wrapped->vertexCount * CalcVertexStride() / sizeof(float));
	}

	uint32_t CalcVertexStride() const
	{
		return axMeshCalcVertexStride(m_wrapped);
	}

private:
	PyImmutableMeshData(AxMeshData* wrapped) : m_wrapped(wrapped) {}

	/*void unflattenStreams(PyStreamList& streams) const
	{
		streams.resize(m_wrapped->streamCount);
		for (uint32_t si = 0; si < m_wrapped->streamCount; si++)
		{
			const AxMeshStream& stream = m_wrapped->pStreams[si];
			streams[si].m_attributes.resize(stream.attributeCount);
			for (uint32_t ai = 0; ai < stream.attributeCount; ai++)
			{
				streams[si].m_attributes[ai] = m_wrapped->pAttributes[stream.attributeStart + ai];
			}
			const uint32_t stride = streams[si].calcStride();
			streams[si].m_vertices.reserve(m_wrapped->vertexCount * stride);
			std::copy_n((char*)m_wrapped->pVertices  stream.dataOffset, streams[si].m_vertices);
		}
	}*/

private:
	AxMeshData* m_wrapped {};

	friend class PyMeshData;
};


PYBIND11_MODULE(PyApexAsset, m)
{
	m.doc() = "Apex Mesh Exporter";

	py::register_local_exception<std::runtime_error>(m, "CppRuntimeError", PyExc_RuntimeError);

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

	py::class_<PyMeshStream>(m, "Stream")
		.def(py::init<>())
		.def_property_readonly("stride", &PyMeshStream::CalcStride)
		.def("set_vertices", &PyMeshStream::SetVertices)
		.def("get_attributes", &PyMeshStream::GetAttributeList)
		.def("__repr__", &PyMeshStream::ToString);

	py::class_<PyMeshData>(m, "Mesh")
		.def(py::init<>())
		.def("calc_stride", &PyMeshData::CalcVertexStride)
		.def("get_streams", &PyMeshData::GetStreamList)
		.def("set_streams", &PyMeshData::SetStreamList)
		.def("get_indices", &PyMeshData::GetIndexList)
		.def("set_indices", &PyMeshData::SetIndexList)
		.def("export", &PyMeshData::ExportToFile);

	py::class_<PyImmutableMeshData>(m, "ImmutableMesh")
		.def("clone_mutable", &PyImmutableMeshData::CloneToMutable)
		.def_property_readonly("streams", &PyImmutableMeshData::GetStreams)
		.def_property_readonly("attributes", &PyImmutableMeshData::GetAttributes)
		.def_property_readonly("indices", &PyImmutableMeshData::GetIndices)
		.def_property_readonly("vertices", &PyImmutableMeshData::GetVertices)
		.def_property_readonly("stride", &PyImmutableMeshData::CalcVertexStride);

	m.def("import_mesh", &PyImmutableMeshData::importFromFile);
}
