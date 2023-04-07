if(NOT PYTHON)
  return()
endif()

cmake_minimum_required(VERSION 3.18)

if(NOT TARGET ${PROJECT_NAMESPACE}::highs)
  message(FATAL_ERROR "Python: missing highs TARGET")
endif()


# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

# Find if the python module is available,
# otherwise install it (PACKAGE_NAME) to the Python3 user install directory.
# If CMake option FETCH_PYTHON_DEPS is OFF then issue a fatal error instead.
# e.g
# search_python_module(
#   NAME
#     mypy_protobuf
#   PACKAGE
#     mypy-protobuf
#   NO_VERSION
# )
function(search_python_module)
  set(options NO_VERSION)
  set(oneValueArgs NAME PACKAGE)
  set(multiValueArgs "")
  cmake_parse_arguments(MODULE
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )
  message(STATUS "Searching python module: \"${MODULE_NAME}\"")
  if(${MODULE_NO_VERSION})
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}"
      RESULT_VARIABLE _RESULT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(MODULE_VERSION "unknown")
  else()
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}; print(${MODULE_NAME}.__version__)"
      RESULT_VARIABLE _RESULT
      OUTPUT_VARIABLE MODULE_VERSION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif()
  if(${_RESULT} STREQUAL "0")
    message(STATUS "Found python module: \"${MODULE_NAME}\" (found version \"${MODULE_VERSION}\")")
  else()
    if(FETCH_PYTHON_DEPS)
      message(WARNING "Can't find python module: \"${MODULE_NAME}\", install it using pip...")
      execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install --user ${MODULE_PACKAGE}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
      message(FATAL_ERROR "Can't find python module: \"${MODULE_NAME}\", please install it using your system package manager.")
    endif()
  endif()
endfunction()

# Find if a python builtin module is available.
# e.g
# search_python_internal_module(
#   NAME
#     mypy_protobuf
# )
function(search_python_internal_module)
  set(options "")
  set(oneValueArgs NAME)
  set(multiValueArgs "")
  cmake_parse_arguments(MODULE
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )
  message(STATUS "Searching python module: \"${MODULE_NAME}\"")
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}"
    RESULT_VARIABLE _RESULT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(${_RESULT} STREQUAL "0")
    message(STATUS "Found python internal module: \"${MODULE_NAME}\"")
  else()
    message(FATAL_ERROR "Can't find python internal module \"${MODULE_NAME}\", please install it using your system package manager.")
  endif()
endfunction()

###################
##  Python Test  ##
###################
if(BUILD_VENV)
  search_python_module(NAME virtualenv PACKAGE virtualenv)
  # venv not working on github runners
  # search_python_internal_module(NAME venv)
  # Testing using a vitual environment
  set(VENV_EXECUTABLE ${Python3_EXECUTABLE} -m virtualenv)
  #set(VENV_EXECUTABLE ${Python3_EXECUTABLE} -m venv)
  set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/python/venv)
  if(WIN32)
    set(VENV_Python3_EXECUTABLE ${VENV_DIR}/Scripts/python.exe)
  else()
    set(VENV_Python3_EXECUTABLE ${VENV_DIR}/bin/python)
  endif()
endif()

if(BUILD_TESTING)
  # add_python_test()
  # CMake function to generate and build python test.
  # Parameters:
  #  the python filename
  # e.g.:
  # add_python_test(foo.py)
  function(add_python_test FILE_NAME)
    message(STATUS "Configuring test ${FILE_NAME} ...")
    get_filename_component(TEST_NAME ${FILE_NAME} NAME_WE)
    get_filename_component(WRAPPER_DIR ${FILE_NAME} DIRECTORY)
    get_filename_component(COMPONENT_DIR ${WRAPPER_DIR} DIRECTORY)
    get_filename_component(COMPONENT_NAME ${COMPONENT_DIR} NAME)

    add_test(
      NAME python_${COMPONENT_NAME}_${TEST_NAME}
      COMMAND ${VENV_Python3_EXECUTABLE} -m pytest ${FILE_NAME}
      WORKING_DIRECTORY ${VENV_DIR})
    message(STATUS "Configuring test ${FILE_NAME} done")
  endfunction()
endif()

# #######################
# ##  PYTHON WRAPPERS  ##
# #######################

# set(PYTHON_PROJECT highspy)
# message(STATUS "Python project: ${PYTHON_PROJECT}")
# set(PYTHON_PROJECT_DIR ${PROJECT_BINARY_DIR}/python/)
# message(STATUS "Python project build path: ${PYTHON_PROJECT_DIR}/highspy")

# # add_subdirectory(${HIGHS_SOURCE_DIR}/highspy)

# #######################
# ## Python Packaging  ##
# #######################
# # file(MAKE_DIRECTORY ${PYTHON_PROJECT_DIR}/python/highspy)
# # file(GENERATE OUTPUT ${PYTHON_PROJECT_DIR}/__init__.py CONTENT "__version__ = \"${PROJECT_VERSION}\"\n")

# file(COPY
#   highspy/__init__.py
#   DESTINATION ${PYTHON_PROJECT_DIR})

# file(COPY
#   highspy/highs.py
#   DESTINATION ${PYTHON_PROJECT_DIR})

# file(COPY
#   setup.py
#   DESTINATION ${PYTHON_PROJECT_DIR})

# file(COPY
#   highspy/highs_bindings.cpp
#   DESTINATION ${PYTHON_PROJECT_DIR})

# # configure_file(
# #   ${PROJECT_SOURCE_DIR}/ortools/python/README.pypi.txt
# #   ${PROJECT_BINARY_DIR}/python/README.txt
# #   COPYONLY)

# # Look for required python modules
# search_python_module(
#   NAME setuptools
#   PACKAGE setuptools)
# search_python_module(
#   NAME wheel
#   PACKAGE wheel)

# add_custom_command(
#   OUTPUT python/dist/timestamp
#   COMMAND ${CMAKE_COMMAND} -E remove_directory dist
#   COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_PROJECT}/.libs
#   # Don't need to copy static lib on Windows.
#   COMMAND ${CMAKE_COMMAND} -E $<IF:$<STREQUAL:$<TARGET_PROPERTY:highs_bindings,TYPE>,SHARED_LIBRARY>,copy,true>
#   $<$<STREQUAL:$<TARGET_PROPERTY:highs_bindings,TYPE>,SHARED_LIBRARY>:$<TARGET_SONAME_FILE:highs_bindings>>
#   ${PYTHON_PROJECT}/.libs
#   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:highs_bindings> ${PYTHON_PROJECT}/highspy
#     #COMMAND ${Python3_EXECUTABLE} setup.py bdist_egg bdist_wheel
#    COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
#   COMMAND ${CMAKE_COMMAND} -E touch ${PROJECT_BINARY_DIR}/python/dist/timestamp
#   DEPENDS
#     setup.py
#     ${PROJECT_NAMESPACE}::highs_bindings
#   BYPRODUCTS
#     python/${PYTHON_PROJECT}
#     python/${PYTHON_PROJECT}.egg-info
#     python/build
#     python/dist
# WORKING_DIRECTORY python
#   COMMAND_EXPAND_LISTS)

# # Main Target
# add_custom_target(python_package ALL
#   DEPENDS
#     python/dist/timestamp
#   WORKING_DIRECTORY python)

#   # detect virtualenv and set Pip args accordingly
# if(DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX})
#   set(_pip_args)
# else()
#   set(_pip_args "--user")
# endif()

# execute_process(COMMAND ${Python_EXECUTABLE} -m pip install ${_pip_args} -e "setup.py")

# if(BUILD_VENV)
#   # make a virtualenv to install our python package in it
#   add_custom_command(TARGET python_package POST_BUILD
#     # Clean previous install otherwise pip install may do nothing
#     COMMAND ${CMAKE_COMMAND} -E remove_directory ${VENV_DIR}
#     COMMAND ${VENV_EXECUTABLE} -p ${Python3_EXECUTABLE}
#     $<IF:$<BOOL:${VENV_USE_SYSTEM_SITE_PACKAGES}>,--system-site-packages,-q>
#       ${VENV_DIR}
#     #COMMAND ${VENV_EXECUTABLE} ${VENV_DIR}
#     # Must NOT call it in a folder containing the setup.py otherwise pip call it
#     # (i.e. "python setup.py bdist") while we want to consume the wheel package
#     COMMAND ${VENV_Python3_EXECUTABLE} -m pip install
#       --find-links=${CMAKE_CURRENT_BINARY_DIR}/python/dist ${PYTHON_PROJECT}==${PROJECT_VERSION}
#     BYPRODUCTS ${VENV_DIR}
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     COMMENT "Create venv and install ${PYTHON_PROJECT}"
#     VERBATIM)
# endif()

# # if(BUILD_TESTING)
# #   configure_file(
# #     ${PROJECT_SOURCE_DIR}/ortools/init/python/version_test.py.in
# #     ${PROJECT_BINARY_DIR}/python/version_test.py
# #     @ONLY)

# #   # run the tests within the virtualenv
# #   add_test(NAME python_init_version_test
# #     COMMAND ${VENV_Python3_EXECUTABLE} ${PROJECT_BINARY_DIR}/python/version_test.py)
# # endif()

# # #####################
# # ##  Python Sample  ##
# # #####################
# # # add_python_sample()
# # # CMake function to generate and build python sample.
# # # Parameters:
# # #  the python filename
# # # e.g.:
# # # add_python_sample(foo.py)
# # function(add_python_sample FILE_NAME)
# #   message(STATUS "Configuring sample ${FILE_NAME} ...")
# #   get_filename_component(SAMPLE_NAME ${FILE_NAME} NAME_WE)
# #   get_filename_component(SAMPLE_DIR ${FILE_NAME} DIRECTORY)
# #   get_filename_component(COMPONENT_DIR ${SAMPLE_DIR} DIRECTORY)
# #   get_filename_component(COMPONENT_NAME ${COMPONENT_DIR} NAME)

# #   if(BUILD_TESTING)
# #     add_test(
# #       NAME python_${COMPONENT_NAME}_${SAMPLE_NAME}
# #       COMMAND ${VENV_Python3_EXECUTABLE} ${FILE_NAME}
# #       WORKING_DIRECTORY ${VENV_DIR})
# #   endif()
# #   message(STATUS "Configuring sample ${FILE_NAME} done")
# # endfunction()

# ######################
# ##  Python Example  ##
# ######################
# # add_python_example()
# # CMake function to generate and build python example.
# # Parameters:
# #  the python filename
# # e.g.:
# # add_python_example(foo.py)
# function(add_python_example FILE_NAME)
#   message(STATUS "Configuring example ${FILE_NAME} ...")
#   get_filename_component(EXAMPLE_NAME ${FILE_NAME} NAME_WE)
#   get_filename_component(COMPONENT_DIR ${FILE_NAME} DIRECTORY)
#   get_filename_component(COMPONENT_NAME ${COMPONENT_DIR} NAME)

#   if(BUILD_TESTING)
#     add_test(
#       NAME python_${COMPONENT_NAME}_${EXAMPLE_NAME}
#       COMMAND ${VENV_Python3_EXECUTABLE} ${FILE_NAME}
#       WORKING_DIRECTORY ${VENV_DIR})
#   endif()
#   message(STATUS "Configuring example ${FILE_NAME} done")
# endfunction()