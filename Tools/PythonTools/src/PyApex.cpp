#include <iostream>

#include "Core/Macros.h"
#if APEX_TOOL
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <filesystem>
#include <numeric>
#include <ranges>

#include "Asset/Mesh.h"
#include "String/AxStream.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace {

	struct EarlyInit
	{
		EarlyInit()
		{
			apex::logging::Logger::Init(false);
#if !APEX_DEBUG
			apex::logging::Logger::get().setMinLogLevel(apex::logging::LogLevel::Info);
#endif
			apex::mem::MemoryManager::initialize({});
		}
		~EarlyInit()
		{
			apex::mem::MemoryManager::shutdown();
		}
	} g_EarlyInit;

}

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

template <typename Type>
void setList(Type* outArray, size_t sizeInBytes, const py::list& inList)
{
	for (size_t i = 0; i < std::min(inList.size(), sizeInBytes / sizeof(Type)); i++)
	{
		outArray[i] = inList[i].cast<Type>();
		// axDebugFmt("i: {}, in_val: {}, out_val: {}, offset: {}", i, inList[i].cast<Type>(), outArray[i], (apex::u8*)&outArray[i] - (apex::u8*)outArray);
	}
}

constexpr static const char* GetVertexAttributeName(apex::gfx::VertexAttribute::Type attr)
{
	switch (attr)
	{
	case apex::gfx::VertexAttribute::ePOSITION:		return "POSITION";
	case apex::gfx::VertexAttribute::eNORMAL:		return "NORMAL";
	case apex::gfx::VertexAttribute::eTANGENT:		return "TANGENT";
	case apex::gfx::VertexAttribute::eBITANGENT:	return "BITANGENT";
	case apex::gfx::VertexAttribute::eTEXCOORD0:	return "TEXCOORD0";
	case apex::gfx::VertexAttribute::eTEXCOORD1:	return "TEXCOORD1";
	case apex::gfx::VertexAttribute::eTEXCOORD2:	return "TEXCOORD2";
	case apex::gfx::VertexAttribute::eTEXCOORD3:	return "TEXCOORD3";
	case apex::gfx::VertexAttribute::eBONEINDICES0:	return "BONEINDICES0";
	case apex::gfx::VertexAttribute::eBONEWEIGHTS0:	return "BONEWEIGHTS0";
	case apex::gfx::VertexAttribute::eBONEINDICES1:	return "BONEINDICES1";
	case apex::gfx::VertexAttribute::eBONEWEIGHTS1:	return "BONEWEIGHTS1";
	case apex::gfx::VertexAttribute::eCOLOR:		return "COLOR";
	case apex::gfx::VertexAttribute::eUSERVALUE0:	return "USERVALUE0";
	case apex::gfx::VertexAttribute::eUSERVALUE1:	return "USERVALUE1";
	case apex::gfx::VertexAttribute::eUSERVALUE2:	return "USERVALUE2";
	case apex::gfx::VertexAttribute::eUSERVALUE3:	return "USERVALUE3";
	case apex::gfx::VertexAttribute::eCOUNT:		return "COUNT";
	}
}

static apex::gfx::VertexAttribute::Type GetVertexAttributeFromName(const char* attrName)
{
	if (_stricmp(attrName, "POSITION") != 0)		return apex::gfx::VertexAttribute::ePOSITION;
	if (_stricmp(attrName, "NORMAL") != 0)			return apex::gfx::VertexAttribute::eNORMAL;
	if (_stricmp(attrName, "TANGENT") != 0)			return apex::gfx::VertexAttribute::eTANGENT;
	if (_stricmp(attrName, "BITANGENT") != 0)		return apex::gfx::VertexAttribute::eBITANGENT;
	if (_stricmp(attrName, "TEXCOORD0") != 0)		return apex::gfx::VertexAttribute::eTEXCOORD0;
	if (_stricmp(attrName, "TEXCOORD1") != 0)		return apex::gfx::VertexAttribute::eTEXCOORD1;
	if (_stricmp(attrName, "TEXCOORD2") != 0)		return apex::gfx::VertexAttribute::eTEXCOORD2;
	if (_stricmp(attrName, "TEXCOORD3") != 0)		return apex::gfx::VertexAttribute::eTEXCOORD3;
	if (_stricmp(attrName, "BONEINDICES0") != 0)	return apex::gfx::VertexAttribute::eBONEINDICES0;
	if (_stricmp(attrName, "BONEWEIGHTS0") != 0)	return apex::gfx::VertexAttribute::eBONEWEIGHTS0;
	if (_stricmp(attrName, "BONEINDICES1") != 0)	return apex::gfx::VertexAttribute::eBONEINDICES1;
	if (_stricmp(attrName, "BONEWEIGHTS1") != 0)	return apex::gfx::VertexAttribute::eBONEWEIGHTS1;
	if (_stricmp(attrName, "COLOR") != 0)			return apex::gfx::VertexAttribute::eCOLOR;
	if (_stricmp(attrName, "USERVALUE0") != 0)		return apex::gfx::VertexAttribute::eUSERVALUE0;
	if (_stricmp(attrName, "USERVALUE1") != 0)		return apex::gfx::VertexAttribute::eUSERVALUE1;
	if (_stricmp(attrName, "USERVALUE2") != 0)		return apex::gfx::VertexAttribute::eUSERVALUE2;
	if (_stricmp(attrName, "USERVALUE3") != 0)		return apex::gfx::VertexAttribute::eUSERVALUE3;
	if (_stricmp(attrName, "COUNT") != 0)			return apex::gfx::VertexAttribute::eCOUNT;
}

enum DataType
{
	DT_NONE,
	DT_BOOL,
	DT_CHAR,
	DT_UCHAR,
	DT_SHORT,
	DT_USHORT,
	DT_INT,
	DT_UINT,
	DT_LONGLONG,
	DT_ULONGLONG,
	DT_HALF,
	DT_FLOAT,
	DT_DOUBLE,
	DT_INT8 = DT_CHAR,
	DT_UINT8 = DT_UCHAR,
	DT_INT16 = DT_SHORT,
	DT_UINT16 = DT_USHORT,
	DT_INT32 = DT_INT,
	DT_UINT32 = DT_UINT,
	DT_INT64 = DT_LONGLONG,
	DT_UINT64 = DT_ULONGLONG,
	DT_FLOAT16 = DT_HALF,
	DT_FLOAT32 = DT_FLOAT,
	DT_FLOAT64 = DT_DOUBLE,
};

constexpr static apex::u32 GetDataTypeSize(DataType dt)
{
	switch (dt)
	{
	case DT_INT8:	return 1;
	case DT_UINT8:  return 1;
	case DT_INT16:	return 2;
	case DT_UINT16: return 2;
	case DT_INT32:	return 4;
	case DT_UINT32: return 4;
	case DT_INT64:  return 8;
	case DT_UINT64: return 8;
	case DT_FLOAT16: return 2;
	case DT_FLOAT32: return 4;
	case DT_FLOAT64: return 8;
	default:
	case DT_NONE:	return 0;
	}
}

constexpr static DataType GetComponentDataType(apex::gfx::Format format)
{
	switch (format)
	{
	case apex::gfx::Format::R8_UNORM:	
	case apex::gfx::Format::R8_UINT:	
	case apex::gfx::Format::R8_SRGB:	
	case apex::gfx::Format::R8G8_UNORM:	
	case apex::gfx::Format::R8G8_UINT:	
	case apex::gfx::Format::R8G8_SRGB:	
	case apex::gfx::Format::R8G8B8_UNORM:
	case apex::gfx::Format::R8G8B8_UINT:
	case apex::gfx::Format::R8G8B8_SRGB:
	case apex::gfx::Format::B8G8R8_UNORM:
	case apex::gfx::Format::B8G8R8_UINT:
	case apex::gfx::Format::B8G8R8_SRGB:
	case apex::gfx::Format::R8G8B8A8_UNORM:
	case apex::gfx::Format::R8G8B8A8_UINT:
	case apex::gfx::Format::R8G8B8A8_SRGB:
	case apex::gfx::Format::B8G8R8A8_UNORM:
	case apex::gfx::Format::B8G8R8A8_UINT:
	case apex::gfx::Format::B8G8R8A8_SRGB:
		return DT_UINT8;
	case apex::gfx::Format::R8_SNORM:	
	case apex::gfx::Format::R8_SINT:	
	case apex::gfx::Format::R8G8_SNORM:	
	case apex::gfx::Format::R8G8_SINT:
	case apex::gfx::Format::R8G8B8_SNORM:
	case apex::gfx::Format::R8G8B8_SINT:
	case apex::gfx::Format::B8G8R8_SNORM:
	case apex::gfx::Format::B8G8R8_SINT:
	case apex::gfx::Format::R8G8B8A8_SNORM:
	case apex::gfx::Format::R8G8B8A8_SINT:
	case apex::gfx::Format::B8G8R8A8_SNORM:
	case apex::gfx::Format::B8G8R8A8_SINT:
		return DT_INT8;
	case apex::gfx::Format::R16_UNORM:
	case apex::gfx::Format::R16_UINT:
	case apex::gfx::Format::R16G16_UNORM:
	case apex::gfx::Format::R16G16_UINT:
	case apex::gfx::Format::R16G16B16_UNORM:
	case apex::gfx::Format::R16G16B16_UINT:
	case apex::gfx::Format::R16G16B16A16_UNORM:
	case apex::gfx::Format::R16G16B16A16_UINT:
		return DT_UINT16;
	case apex::gfx::Format::R16_SNORM:
	case apex::gfx::Format::R16_SINT:
	case apex::gfx::Format::R16G16_SNORM:
	case apex::gfx::Format::R16G16_SINT:
	case apex::gfx::Format::R16G16B16_SNORM:
	case apex::gfx::Format::R16G16B16_SINT:
	case apex::gfx::Format::R16G16B16A16_SNORM:
	case apex::gfx::Format::R16G16B16A16_SINT:
		return DT_INT16;
	case apex::gfx::Format::R16_SFLOAT:
	case apex::gfx::Format::R16G16_SFLOAT:
	case apex::gfx::Format::R16G16B16_SFLOAT:
	case apex::gfx::Format::R16G16B16A16_SFLOAT:
		return DT_FLOAT16;
	case apex::gfx::Format::R32_UINT:
	case apex::gfx::Format::R32G32_UINT:
	case apex::gfx::Format::R32G32B32_UINT:
	case apex::gfx::Format::R32G32B32A32_UINT:
		return DT_UINT32;
	case apex::gfx::Format::R32_SINT:
	case apex::gfx::Format::R32G32_SINT:
	case apex::gfx::Format::R32G32B32_SINT:
	case apex::gfx::Format::R32G32B32A32_SINT:
		return DT_INT32;
	case apex::gfx::Format::R32_SFLOAT:
	case apex::gfx::Format::R32G32_SFLOAT:
	case apex::gfx::Format::R32G32B32_SFLOAT:
	case apex::gfx::Format::R32G32B32A32_SFLOAT:
		return DT_FLOAT32;
	case apex::gfx::Format::UNDEFINED:
	default:
		return DT_NONE;
	}
}

constexpr static bool IsFloatingPointDataType(DataType dt)
{
	switch (dt)
	{
	case DT_FLOAT16:
	case DT_FLOAT32:
	case DT_FLOAT64:
		return true;
	case DT_INT8:
	case DT_UINT8:
	case DT_INT16:
	case DT_UINT16:
	case DT_INT32:
	case DT_UINT32:
	case DT_INT64: 
	case DT_UINT64:
	case DT_NONE:
	default:
		return false;
	}
}

#define throw_runtime_error(fmt,...) do { throw std::runtime_error(std::format(fmt, ##__VA_ARGS__)); } while (0)

constexpr apex::u32 INVALID_VALUE = static_cast<apex::u32>(-1);

struct StreamAttribute
{
	apex::gfx::VertexAttribute::Type	type;
	apex::gfx::Format					format;
};

struct Stream
{
	apex::u32						index;
	apex::u32						stride;
	std::vector<StreamAttribute>	attributes;
};

class ReadOnlyMesh
{
public:
	ReadOnlyMesh(apex::asset::ReadOnlyMeshData&& mesh_data, const std::vector<Stream>& streams) : m_meshData(std::move(mesh_data)), m_streams(streams) {}
	ReadOnlyMesh(apex::asset::ReadOnlyMeshData&& mesh_data, std::vector<Stream>&& streams) : m_meshData(std::move(mesh_data)), m_streams(std::move(streams)) {}

	const std::vector<Stream>& get_streams() const { return m_streams; }

	const apex::asset::ReadOnlyMeshData& get_data() const { return m_meshData; }

private:
	apex::asset::ReadOnlyMeshData m_meshData;
	std::vector<Stream> m_streams;
};


class MeshBuilder;
class StreamBuilder;

class MeshBuilder
{
public:
	MeshBuilder() = default;

	StreamBuilder begin_stream();

	MeshBuilder& set_data(apex::gfx::VertexAttribute::Type type, py::list data)
	{
		// axDebugFmt("enter");
		m_data[type] = std::move(data);
		return *this;
	}

	MeshBuilder& set_indices(std::vector<apex::u32> indices)
	{
		// axDebugFmt("enter");
		m_indices = std::move(indices);
		return *this;
	}

	ReadOnlyMesh build()
	{
		// axDebugFmt("enter");
		apex::gfx::VertexFormat vertexFormat{};
		std::vector<apex::u8> data;
		apex::u32 vertexStride = 0;
		apex::u32 location = 0;
		apex::u32 vertexCount = 0;
		bool firstIter = true;
		for (const auto& stream : m_streams)
		{
			apex::u32 offsetInStream = 0;
			for (const auto& attr : stream.attributes)
			{
				if (vertexFormat.attributes[attr.type].format != 0)
				{
					throw_runtime_error("Duplicate vertex attribute '{}' found!", GetVertexAttributeName(attr.type));
				}
				auto& vertAttr = vertexFormat.attributes[attr.type];
				vertAttr.format = ((apex::u32)attr.format) & VERT_ATTR_FORMAT_MAX;
				vertAttr.offset = offsetInStream;
				vertAttr.stream = stream.index;
				vertAttr.location = location++;
				const apex::u32 size = apex::gfx::Factory::GetFormatSize(attr.format);
				const DataType dt = GetComponentDataType(attr.format);
				const apex::u32 compSize = GetDataTypeSize(dt);
				const apex::u32 nComp = size / compSize;
				const auto& dataList = m_data[attr.type];
				const apex::u32 attrVertexCount = dataList.size() / nComp;
				offsetInStream += size;
				vertexStride += size;
				if (firstIter)
				{
					vertexCount = attrVertexCount;
				}
				else if (attrVertexCount != vertexCount)
				{
					throw_runtime_error("Mismatch in vertex counts across streams!");
				}
				firstIter = false;
				// axDebugFmt("Attr : (type={}, format={}, location={}, stream={}, offset={})", GetVertexAttributeName(attr.type), (apex::u32)vertAttr.format, (apex::u32)vertAttr.offset, (apex::u32)vertAttr.stream, (apex::u32)vertAttr.location);
			}
		}
		data.resize(vertexStride * vertexCount);
		axDebugFmt("vtx stride: {}, vtx count: {}, data.size : {}", vertexStride, vertexCount, data.size());
		apex::u32 streamOffset = 0;
		for (const auto& stream : m_streams)
		{
			for (const auto& attr : stream.attributes)
			{
				const auto& vertAttr = vertexFormat.attributes[attr.type];
				const apex::u32 size = apex::gfx::Factory::GetFormatSize(attr.format);
				const apex::u32 stride = stream.stride;
				const apex::u32 offset = vertAttr.offset;
				const auto& dataList = m_data[attr.type];
				const DataType dt = GetComponentDataType(attr.format);
				const apex::u32 compSize = GetDataTypeSize(dt);
				const apex::u32 nComp = size / compSize;
				apex::u32 out = streamOffset;
				std::vector<apex::u8> dataStream;
				dataStream.resize(dataList.size() * compSize);
				axDebugFmt("dataStream size: {}", dataStream.size());
				axInfoFmt("{} {}", GetVertexAttributeName(attr.type), dataList.size());
				switch (dt)
				{
				case DT_BOOL:
					setList<bool>((bool*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_INT8:
					setList<apex::s8>((apex::s8*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_UINT8:
					setList<apex::u8>((apex::u8*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_INT16:
					setList<apex::s16>((apex::s16*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_UINT16:
					setList<apex::u16>((apex::u16*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_INT32:
					setList<apex::s32>((apex::s32*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_UINT32:
					setList<apex::u32>((apex::u32*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_INT64:
					setList<apex::s64>((apex::s64*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_UINT64:
					setList<apex::u64>((apex::u64*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_FLOAT16:
				case DT_FLOAT32:
					setList<apex::f32>((apex::f32*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_FLOAT64:
					setList<apex::f64>((apex::f64*)dataStream.data(), dataStream.size(), dataList); break;
				case DT_NONE:
				default:
					throw_runtime_error("Unknown data type!");
				}
				apex::u32 count = 0;
				for (apex::u32 i = 0; i < dataStream.size(); i += size, out += stride)
				{
					const apex::u8* dat = &dataStream[i];
					//if (IsFloatingPointDataType(dt))
					axDebugFmt(" -> {}, out: {}, offset: {}, dat: {}, dataStream[{}]: {}", count++, out, offset, *(apex::f32*)dat, i, *(apex::f32*)&dataStream[i]);
					//else
					//	axDebugFmt(" -> {}, out: {}, offset: {}, dat: {}, dataStream[{}]: {}", count++, out, offset, *dat, i, *(apex::u32*)&dataStream[i]);
					apex::memcpy_s<apex::u8>(&data[out + offset], size, dat, size);
				}
			}
			streamOffset += stream.stride * vertexCount;
		}
		auto meshData =  apex::asset::MeshDataBuilder().Build(vertexFormat, (apex::u32)data.size(), data.data(), (apex::u32)m_indices.size(), m_indices.data());

		//axDebugFmt("indices");
		//for (auto idx : meshData.GetIndices())
		//{
		//	axDebugFmt("  idx: {}", idx);
		//}

		axDebugFmt("mesh built");
		return ReadOnlyMesh{ std::move(meshData), m_streams };
	}

private:
	std::vector<Stream> m_streams;
	std::vector<apex::u32> m_indices;
	std::array<py::list, apex::gfx::VertexAttribute::eCOUNT> m_data;

	friend class StreamBuilder;
};

class StreamBuilder
{
public:
	StreamBuilder(MeshBuilder& parent) : m_parent(parent) {}

	StreamBuilder& add_attribute(apex::gfx::VertexAttribute::Type type, apex::gfx::Format format)
	{
		// axDebugFmt("enter");
		m_attributes.emplace_back(type, format);
		return *this;
	}

	MeshBuilder& end_stream()
	{
		// axDebugFmt("enter");
		apex::u32 stride = 0;
		for (const auto& attr : m_attributes)
		{
			stride += apex::gfx::Factory::GetFormatSize(attr.format);
		}
		m_parent.m_streams.emplace_back((apex::u32)m_parent.m_streams.size(), stride, std::move(m_attributes));
		return m_parent;
	}

private:
	MeshBuilder m_parent;
	std::vector<StreamAttribute> m_attributes;
};

StreamBuilder MeshBuilder::begin_stream()
{
	// axDebugFmt("enter");
	return { *this };
}

class MeshExporter
{
public:
	MeshExporter() = default;

	void export_mesh(const ReadOnlyMesh& mesh, const std::string& filepath)
	{
		(void)apex::asset::MeshLoader().WriteToFile(mesh.get_data(), filepath.c_str());
	}
	
};

PYBIND11_MODULE(PyApex, m)
{
	m.doc() = "Apex Game Engine - Python bindings";

	py::register_local_exception<std::runtime_error>(m, "CppRuntimeError", PyExc_RuntimeError);

	py::enum_<apex::gfx::VertexAttribute::Type>(m, "VertexAttributeType")
		.value("POSITION", apex::gfx::VertexAttribute::ePOSITION)
		.value("NORMAL", apex::gfx::VertexAttribute::eNORMAL)
		.value("TANGENT", apex::gfx::VertexAttribute::eTANGENT)
		.value("BITANGENT", apex::gfx::VertexAttribute::eBITANGENT)
		.value("TEXCOORD0", apex::gfx::VertexAttribute::eTEXCOORD0)
		.value("TEXCOORD1", apex::gfx::VertexAttribute::eTEXCOORD1)
		.value("TEXCOORD2", apex::gfx::VertexAttribute::eTEXCOORD2)
		.value("TEXCOORD3", apex::gfx::VertexAttribute::eTEXCOORD3)
		.value("BONEINDICES0", apex::gfx::VertexAttribute::eBONEINDICES0)
		.value("BONEWEIGHTS0", apex::gfx::VertexAttribute::eBONEWEIGHTS0)
		.value("BONEINDICES1", apex::gfx::VertexAttribute::eBONEINDICES1)
		.value("BONEWEIGHTS1", apex::gfx::VertexAttribute::eBONEWEIGHTS1)
		.value("COLOR", apex::gfx::VertexAttribute::eCOLOR)
		.value("USERVALUE0", apex::gfx::VertexAttribute::eUSERVALUE0)
		.value("USERVALUE1", apex::gfx::VertexAttribute::eUSERVALUE1)
		.value("USERVALUE2", apex::gfx::VertexAttribute::eUSERVALUE2)
		.value("USERVALUE3", apex::gfx::VertexAttribute::eUSERVALUE3)
		.value("COUNT", apex::gfx::VertexAttribute::eCOUNT)
		.export_values();

#pragma region apex::gfx::Format
	py::enum_<apex::gfx::Format>(m, "Format")
		.value("UNDEFINED", apex::gfx::Format::UNDEFINED)
		.value("R8_UNORM", apex::gfx::Format::R8_UNORM)
		.value("R8_SNORM", apex::gfx::Format::R8_SNORM)
		.value("R8_UINT", apex::gfx::Format::R8_UINT)
		.value("R8_SINT", apex::gfx::Format::R8_SINT)
		.value("R8_SRGB", apex::gfx::Format::R8_SRGB)
		.value("R8G8_UNORM", apex::gfx::Format::R8G8_UNORM)
		.value("R8G8_SNORM", apex::gfx::Format::R8G8_SNORM)
		.value("R8G8_UINT", apex::gfx::Format::R8G8_UINT)
		.value("R8G8_SINT", apex::gfx::Format::R8G8_SINT)
		.value("R8G8_SRGB", apex::gfx::Format::R8G8_SRGB)
		.value("R8G8B8_UNORM", apex::gfx::Format::R8G8B8_UNORM)
		.value("R8G8B8_SNORM", apex::gfx::Format::R8G8B8_SNORM)
		.value("R8G8B8_UINT", apex::gfx::Format::R8G8B8_UINT)
		.value("R8G8B8_SINT", apex::gfx::Format::R8G8B8_SINT)
		.value("R8G8B8_SRGB", apex::gfx::Format::R8G8B8_SRGB)
		.value("B8G8R8_UNORM", apex::gfx::Format::B8G8R8_UNORM)
		.value("B8G8R8_SNORM", apex::gfx::Format::B8G8R8_SNORM)
		.value("B8G8R8_UINT", apex::gfx::Format::B8G8R8_UINT)
		.value("B8G8R8_SINT", apex::gfx::Format::B8G8R8_SINT)
		.value("B8G8R8_SRGB", apex::gfx::Format::B8G8R8_SRGB)
		.value("R8G8B8A8_UNORM", apex::gfx::Format::R8G8B8A8_UNORM)
		.value("R8G8B8A8_SNORM", apex::gfx::Format::R8G8B8A8_SNORM)
		.value("R8G8B8A8_UINT", apex::gfx::Format::R8G8B8A8_UINT)
		.value("R8G8B8A8_SINT", apex::gfx::Format::R8G8B8A8_SINT)
		.value("R8G8B8A8_SRGB", apex::gfx::Format::R8G8B8A8_SRGB)
		.value("B8G8R8A8_UNORM", apex::gfx::Format::B8G8R8A8_UNORM)
		.value("B8G8R8A8_SNORM", apex::gfx::Format::B8G8R8A8_SNORM)
		.value("B8G8R8A8_UINT", apex::gfx::Format::B8G8R8A8_UINT)
		.value("B8G8R8A8_SINT", apex::gfx::Format::B8G8R8A8_SINT)
		.value("B8G8R8A8_SRGB", apex::gfx::Format::B8G8R8A8_SRGB)
		.value("R16_UNORM", apex::gfx::Format::R16_UNORM)
		.value("R16_SNORM", apex::gfx::Format::R16_SNORM)
		.value("R16_UINT", apex::gfx::Format::R16_UINT)
		.value("R16_SINT", apex::gfx::Format::R16_SINT)
		.value("R16_SFLOAT", apex::gfx::Format::R16_SFLOAT)
		.value("R16G16_UNORM", apex::gfx::Format::R16G16_UNORM)
		.value("R16G16_SNORM", apex::gfx::Format::R16G16_SNORM)
		.value("R16G16_UINT", apex::gfx::Format::R16G16_UINT)
		.value("R16G16_SINT", apex::gfx::Format::R16G16_SINT)
		.value("R16G16_SFLOAT", apex::gfx::Format::R16G16_SFLOAT)
		.value("R16G16B16_UNORM", apex::gfx::Format::R16G16B16_UNORM)
		.value("R16G16B16_SNORM", apex::gfx::Format::R16G16B16_SNORM)
		.value("R16G16B16_UINT", apex::gfx::Format::R16G16B16_UINT)
		.value("R16G16B16_SINT", apex::gfx::Format::R16G16B16_SINT)
		.value("R16G16B16_SFLOAT", apex::gfx::Format::R16G16B16_SFLOAT)
		.value("R16G16B16A16_UNORM", apex::gfx::Format::R16G16B16A16_UNORM)
		.value("R16G16B16A16_SNORM", apex::gfx::Format::R16G16B16A16_SNORM)
		.value("R16G16B16A16_UINT", apex::gfx::Format::R16G16B16A16_UINT)
		.value("R16G16B16A16_SINT", apex::gfx::Format::R16G16B16A16_SINT)
		.value("R16G16B16A16_SFLOAT", apex::gfx::Format::R16G16B16A16_SFLOAT)
		.value("R32_UINT", apex::gfx::Format::R32_UINT)
		.value("R32_SINT", apex::gfx::Format::R32_SINT)
		.value("R32_SFLOAT", apex::gfx::Format::R32_SFLOAT)
		.value("R32G32_UINT", apex::gfx::Format::R32G32_UINT)
		.value("R32G32_SINT", apex::gfx::Format::R32G32_SINT)
		.value("R32G32_SFLOAT", apex::gfx::Format::R32G32_SFLOAT)
		.value("R32G32B32_UINT", apex::gfx::Format::R32G32B32_UINT)
		.value("R32G32B32_SINT", apex::gfx::Format::R32G32B32_SINT)
		.value("R32G32B32_SFLOAT", apex::gfx::Format::R32G32B32_SFLOAT)
		.value("R32G32B32A32_UINT", apex::gfx::Format::R32G32B32A32_UINT)
		.value("R32G32B32A32_SINT", apex::gfx::Format::R32G32B32A32_SINT)
		.value("R32G32B32A32_SFLOAT", apex::gfx::Format::R32G32B32A32_SFLOAT)
		.value("D16_UNORM", apex::gfx::Format::D16_UNORM)
	    .value("D32_SFLOAT", apex::gfx::Format::D32_SFLOAT)
	    .value("S8_UINT", apex::gfx::Format::S8_UINT)
	    .value("D24_UNORM_S8_UINT", apex::gfx::Format::D24_UNORM_S8_UINT);
#pragma endregion

	py::enum_<apex::asset::MeshExporterFlagBits>(m, "MeshExporterOptions")
		.value("None", apex::asset::MeshExporterFlagBits::None)
		.value("Triangulate", apex::asset::MeshExporterFlagBits::Triangulate)
		.value("FlipNormals", apex::asset::MeshExporterFlagBits::FlipNormals)
		.value("CalculateNormals", apex::asset::MeshExporterFlagBits::CalculateNormals)
		.value("CalculateTangents", apex::asset::MeshExporterFlagBits::CalculateTangents);

	py::class_<StreamAttribute>(m, "StreamAttribute")
		.def_readonly("type", &StreamAttribute::type)
		.def_readonly("format", &StreamAttribute::format);

	py::class_<Stream>(m, "Stream")
		.def_readonly("index", &Stream::index)
		.def_readonly("stride", &Stream::stride)
		.def_readonly("attributes", &Stream::attributes);

	py::class_<StreamBuilder>(m, "StreamBuilder")
		.def("add_attribute", &StreamBuilder::add_attribute)
		.def("end_stream", &StreamBuilder::end_stream);

	py::class_<MeshBuilder>(m, "MeshBuilder")
		.def(py::init())
		.def("begin_stream", &MeshBuilder::begin_stream)
		.def("set_indices", &MeshBuilder::set_indices)
		.def("set_data", &MeshBuilder::set_data)
		.def("build", &MeshBuilder::build);

	py::class_<ReadOnlyMesh>(m, "ReadOnlyMesh")
		.def("get_streams", &ReadOnlyMesh::get_streams);

	py::class_<MeshExporter>(m, "MeshExporter")
		.def(py::init())
		.def("export", &MeshExporter::export_mesh);
}

#else
#error This project can only be build in Tool config - ToolDebug | ToolRelease
#endif
