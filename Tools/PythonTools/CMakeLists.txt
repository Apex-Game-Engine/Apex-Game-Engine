project(ApexPythonTools LANGUAGES CXX)

pybind11_add_module(PyApexModel MODULE src/PyApexModel.cpp)
install(TARGETS PyApexModel DESTINATION ${APEX_TOOLS_DIR}/bin)

target_link_libraries(PyApexModel
    PUBLIC
    ApexModel
    pybind11::pybind11_headers
)
#target_include_directories(PyApexModel PRIVATE ${APEX_THIRDPARTY_DIR}/pybind11/include)