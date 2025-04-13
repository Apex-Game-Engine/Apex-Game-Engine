#include "MeshSerializer.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define UNUSED(...)

typedef long long ssize_t;
static_assert(sizeof(size_t) == sizeof(ssize_t));

static void* default_allocate(size_t size, void* UNUSED(udata))
{
	return malloc(size);
}

static void* default_reallocate(void* old_mem, size_t UNUSED(old_size), size_t new_size, void* UNUSED(udata))
{
	return realloc(old_mem, new_size);
}

static void default_deallocate(void* mem, void* UNUSED(udata))
{
	free(mem);
}

constexpr static AxMeshMemoryCallbacks default_memory_callbacks {
	.allocate = default_allocate,
	.reallocate = default_reallocate,
	.deallocate = default_deallocate,
};

static void* file_open(const char* path, AxMeshFileOptions options, void* UNUSED(udata))
{
	FILE* f;
    (void)fopen_s(&f, path, options == AxMeshFileOptions_ReadOnly ? "rb" : AxMeshFileOptions_ReadWrite ? "wb+" : "ab+");
	return f;
}

static void file_close(void* file, void* UNUSED(udata))
{
    (void)fclose((FILE*)(file));
}

static size_t file_read(void* file, void* dst, size_t bytes, void* UNUSED(udata))
{
    return fread(dst, 1, bytes, (FILE*)(file));
}

static size_t file_write(void* file, void* src, size_t bytes, void* UNUSED(udata))
{
	return fwrite(src, 1, bytes, (FILE*)(file));
}

static unsigned long file_size(void* file, void* UNUSED(udata))
{
    FILE* f = (FILE*)(file);

    const long beg = ftell(f);
    (void)fseek(f, 0, SEEK_END);
    const long end = ftell(f);
    (void)fseek(f, beg, SEEK_SET);

    return (end > 0) ? end : 0;
}

constexpr static AxMeshFileCallbacks default_file_callbacks {
	.fileOpen = file_open,
	.fileClose = file_close,
	.fileRead = file_read,
	.fileWrite = file_write,
	.fileSize = file_size,
};

class StreamReader
{
public:
	StreamReader(char* buffer, size_t size) : buffer(buffer), readptr(buffer), size(size) {}

	template <typename T>
	T* read_object()
	{
		assert(readptr + sizeof(T) <= buffer + size);
		T* obj = reinterpret_cast<T*>(readptr);
		readptr += sizeof(T);
		return obj;
	}

	template <typename T>
	T* read_array(size_t element_count)
	{
		assert(readptr + sizeof(T) * element_count <= buffer + size);
		T* arr = reinterpret_cast<T*>(readptr);
		readptr += sizeof(T) * element_count;
		return arr;
	}

private:
	char* buffer;
	char* readptr;
	size_t size;
};

class StreamWriter
{
public:
	StreamWriter(char* buffer, size_t size) : buffer(buffer), writeptr(buffer), size(size) {}

	template <typename T>
	void write_object(const T* object)
	{
		assert(writeptr + sizeof(T) <= buffer + size);
		memcpy(writeptr, object, sizeof(T));
		writeptr += sizeof(T);
	}

	template <typename T>
	void write_array(const T* arr, size_t element_count)
	{
		assert(writeptr + sizeof(T) * element_count <= buffer + size);
		memcpy(writeptr, arr, sizeof(T) * element_count);
		writeptr += sizeof(T) * element_count;
	}

private:
	char* buffer;
	char* writeptr;
	size_t size;
};

constexpr static uint32_t MAGIC = 0x58455041; // 'APEX'
constexpr static uint32_t SIGNATURE_MESH = 0x00000001;
constexpr static uint32_t SIGNATURE_MESH_SOA = 0x0000002;

char* load_file_contents(const char* filename, const AxMeshLoaderOptions& options, size_t* out_size)
{
	void* file = options.fileCallbacks.fileOpen(filename, AxMeshFileOptions_ReadOnly, options.udata);
	size_t size = options.fileCallbacks.fileSize(file, options.udata);
	char* buffer = (char*)(options.memoryCallbacks.allocate(size, options.udata));
	options.fileCallbacks.fileRead(file, buffer, size, options.udata);
	options.fileCallbacks.fileClose(file, options.udata);
	*out_size = size;
	return buffer;
}

size_t store_file_contents(const char* filename, const AxMeshLoaderOptions& options, char* buffer, size_t size)
{
	void* file = options.fileCallbacks.fileOpen(filename, AxMeshFileOptions_ReadWrite, options.udata);
	size_t sizeWritten = options.fileCallbacks.fileWrite(file, buffer, size, options.udata);
	options.fileCallbacks.fileClose(file, options.udata);
	return sizeWritten;
}

AxMeshData* axMeshLoadFile(const char* filename, AxMeshLoaderOptions options)
{
	if (options.fileCallbacks.fileOpen == nullptr)
	{
		options.fileCallbacks = default_file_callbacks;
	}
	if (options.memoryCallbacks.allocate == nullptr)
	{
		options.memoryCallbacks = default_memory_callbacks;
	}

	size_t size;
	char* buffer = load_file_contents(filename, options, &size);
	assert(buffer && size);

	StreamReader reader { buffer, size };

	if (const uint32_t magic = *reader.read_object<uint32_t>(); magic != MAGIC)
	{
		assert(magic == MAGIC);
		options.memoryCallbacks.deallocate(buffer, options.udata);
		return nullptr;
	}

	if (const uint32_t signature = *reader.read_object<uint32_t>(); signature != SIGNATURE_MESH)
	{
		assert(signature == SIGNATURE_MESH);
		options.memoryCallbacks.deallocate(buffer, options.udata);
		return nullptr;
	}

	AxMeshData* outMesh = reader.read_object<AxMeshData>();
	outMesh->pStreams = reader.read_array<AxMeshStream>(outMesh->streamCount);
	outMesh->pAttributes = reader.read_array<AxMeshAttribute>(outMesh->attributeCount);
	outMesh->pIndices = reader.read_array<uint32_t>(outMesh->indexCount);
	outMesh->pVertices = reader.read_array<char>((size_t)outMesh->vertexCount * outMesh->vertexStride);

	return outMesh;
}


AxMeshData* axMeshLoadFileSoA(const char* filename, AxMeshLoaderOptions options)
{
	return nullptr;
}

size_t axMeshSaveFile(const char* filename, AxMeshLoaderOptions options, const AxMeshData* mesh)
{
	if (options.fileCallbacks.fileOpen == nullptr)
	{
		options.fileCallbacks = default_file_callbacks;
	}
	if (options.memoryCallbacks.allocate == nullptr)
	{
		options.memoryCallbacks = default_memory_callbacks;
	}

	size_t size = sizeof(MAGIC) + sizeof(SIGNATURE_MESH) + sizeof(AxMeshData)
					+ sizeof(AxMeshStream) * mesh->streamCount
					+ sizeof(AxMeshAttribute) * mesh->attributeCount
					+ sizeof(uint32_t) * mesh->indexCount
					+ (size_t)mesh->vertexStride * mesh->vertexCount;

	char* buffer = (char*)options.memoryCallbacks.allocate(size, options.udata);

	assert([](const AxMeshData* mesh)
	{
		uint32_t vertexSize = 0;
		for (uint32_t i = 0; i < mesh->streamCount; i++)
	    {
	        vertexSize += mesh->pStreams[i].stride;
	    }
		return vertexSize;
	}(mesh) == mesh->vertexStride);

	StreamWriter writer { buffer, size };

	writer.write_object(&MAGIC);
	writer.write_object(&SIGNATURE_MESH);
	writer.write_object(mesh);
	writer.write_array(mesh->pStreams, mesh->streamCount);
	writer.write_array(mesh->pAttributes, mesh->attributeCount);
	writer.write_array(mesh->pIndices, mesh->indexCount);
	writer.write_array((char*)mesh->pVertices, (size_t)mesh->vertexCount * mesh->vertexStride);

	return store_file_contents(filename, options, buffer, size);

}

size_t axMeshSaveFileSoA(const char* filename, AxMeshLoaderOptions options, const AxMeshDataSoA* mesh)
{
	return 0;
}
