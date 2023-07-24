file(GLOB SOURCES *.cpp *.h)

add_executable(SoftwareRaytracer
    ${SOURCES}
)

target_link_libraries(SoftwareRaytracer
    PRIVATE
    Foundation
)

set_target_properties(SoftwareRaytracer PROPERTIES FOLDER "Examples")
