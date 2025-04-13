#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef unsigned long ulong;

enum AxMeshFileOptions
{
	AxMeshFileOptions_ReadOnly,
	AxMeshFileOptions_ReadWrite,
	AxMeshFileOptions_ReadAppend,
};

struct AxMeshFileCallbacks
{
	void*  (*fileOpen)(const char* filename, AxMeshFileOptions options, void* udata);
	void   (*fileClose)(void* file, void* udata);
	size_t (*fileRead)(void* file, void* buffer, size_t size, void* udata);
	size_t (*fileWrite)(void* file, void* buffer, size_t size, void* udata);
	ulong  (*fileSize)(void* file, void* udata);
};

struct AxMeshMemoryCallbacks
{
	void* (*allocate)(size_t size, void* udata);
	void* (*reallocate)(void* old_mem, size_t old_size, size_t new_size, void* udata);
	void  (*deallocate)(void* mem, void* udata);
};

enum AxMeshLoaderFlagBits
{
	AxMeshLoaderFlags_None                = 0x00000000,
	AxMeshLoaderFlags_Triangulate         = 0x00000001,
	AxMeshLoaderFlags_FlipNormals         = 0x00000002,
	AxMeshLoaderFlags_CalculateNormals    = 0x00000004,
	AxMeshLoaderFlags_CalculateTangents   = 0x00000008,
};

typedef uint32_t AxMeshLoaderFlags;

enum AxMeshAttributeType
{
	AxMeshAttribute_Position,
	AxMeshAttribute_Normal,
	AxMeshAttribute_Tangent,
	AxMeshAttribute_TexCoords0,
	AxMeshAttribute_TexCoords1,
	AxMeshAttribute_Color,
	AxMeshAttribute_BoneWeights,
	AxMeshAttribute_BoneIndices,
};

enum AxMeshDataType
{
	
};

struct AxMeshStream
{
	uint32_t stride;
	uint32_t attributeStart;
	uint32_t attributeCount;
	uint32_t dataOffset;
};

//struct AxMeshAttribute
//{
//	AxMeshAttributeType		attributeType;
//	AxMeshDataType			dataType;
//};

typedef AxMeshAttributeType AxMeshAttribute;

struct AxMeshData
{
	uint32_t				vertexStride;
	uint32_t				streamCount;
	uint32_t				attributeCount;
	uint32_t				indexCount;
	uint32_t				vertexCount;
	AxMeshStream*			pStreams;
	AxMeshAttribute*		pAttributes;
	uint32_t*				pIndices;
	void*					pVertices;
};

struct AxMeshIndex
{
	uint32_t				p;
	uint32_t				n;
	uint32_t				t;
	uint32_t				uv0;
	uint32_t				uv1;
	uint32_t				c;
};

struct AxMeshDataSoA
{
	uint32_t				positionCount;
	uint32_t				normalCount;
	uint32_t				tangentCount;
	uint32_t				texCoords0Count;
	uint32_t				texCoords1Count;
	uint32_t				colorCount;
	uint32_t				faceCount;
	uint32_t				indexCount;
	float*					pPositions;
	float*					pNormals;
	float*					pTangents;
	float*					pTexCoords0;
	float*					pTexCoords1;
	float*					pColors;
	uint32_t*				pFaceVertices;
	uint32_t*				pFaceMaterials;
	AxMeshIndex*			pIndices;
};


typedef struct
{
	AxMeshLoaderFlags		flags;
	AxMeshFileCallbacks		fileCallbacks;
	AxMeshMemoryCallbacks	memoryCallbacks;
	void*					udata;

} AxMeshLoaderOptions;

AxMeshData* axMeshLoadFile(const char* filename, AxMeshLoaderOptions options);
AxMeshData* axMeshLoadFileSoA(const char* filename, AxMeshLoaderOptions options);

size_t axMeshSaveFile(const char* filename, AxMeshLoaderOptions options, const AxMeshData* mesh);
size_t axMeshSaveFileSoA(const char* filename, AxMeshLoaderOptions options, const AxMeshDataSoA* mesh);

#ifdef __cplusplus
}
#endif
