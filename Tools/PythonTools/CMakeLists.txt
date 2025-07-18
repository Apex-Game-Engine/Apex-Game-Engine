project(ApexPythonTools LANGUAGES CXX)

pybind11_add_module(PyApexAsset MODULE src/PyApexAsset.cpp)

target_link_libraries(PyApexAsset
    PUBLIC
    ApexAsset
    pybind11::pybind11_headers
)

install(TARGETS PyApexAsset DESTINATION ${APEX_TOOLS_DIR}/bin CONFIGURATIONS Debug Release)
