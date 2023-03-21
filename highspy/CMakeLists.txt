
# pybind11_add_module(highs-py MODULE "${HIGHS_SOURCE_DIR}/src/interfaces/highspy/highspy/highs_bindings.cpp")
pybind11_add_module(highs-py MODULE "highspy/highs_bindings.cpp")

# note: macOS is APPLE and also UNIX !
if(APPLE)
  set_target_properties(highs-py PROPERTIES
    SUFFIX ".so"
    INSTALL_RPATH "@loader_path;@loader_path/../../../${PYTHON_PROJECT}/.libs"
    )
  set_property(TARGET highs-py APPEND PROPERTY
    LINK_FLAGS "-flat_namespace -undefined suppress"
    )
elseif(UNIX)
  set_target_properties(highs-py PROPERTIES
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../../../${PYTHON_PROJECT}/.libs"
    )
endif()

target_link_libraries(highs-py PRIVATE ${PROJECT_NAMESPACE}::highs)
add_library(${PROJECT_NAMESPACE}::highs-py ALIAS highs-py)
