add_executable(TerrainGenerator TerrainGenerator.cpp)

target_link_libraries(TerrainGenerator
	PUBLIC
	ApexFoundation
	ApexPlatform
	ApexGraphics
	TerrainGeneratorShaders
)

target_include_directories(TerrainGenerator
	PRIVATE
	${APEX_THIRDPARTY_DIR}/renderdoc
)

add_shader_library(TerrainGeneratorShaders
	SOURCES
	simulate_erosion.comp
)

target_link_libraries(TerrainGeneratorShaders
	INTERFACE
	ApexShaders
)
