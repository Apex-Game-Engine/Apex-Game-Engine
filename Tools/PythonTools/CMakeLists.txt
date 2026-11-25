project(ApexPythonTools LANGUAGES CXX)

pybind11_add_module(PyApex MODULE src/PyApex.cpp)

target_link_libraries(PyApex
    PUBLIC
    ApexAsset
    pybind11::pybind11_headers
)

set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_Debug TRUE)
set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DebugGame TRUE)
set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_Development TRUE)
set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_Release TRUE)
set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DebugProfile TRUE)
set_property(TARGET PyApex PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_ReleaseProfile TRUE)

target_compile_options(PyApex PRIVATE /EHsc /EHc)
target_compile_definitions(PyApex PRIVATE _HAS_EXCEPTIONS=1)

install(TARGETS PyApex DESTINATION ${APEX_TOOLS_DIR}/bin CONFIGURATIONS ToolDebug ToolRelease)
