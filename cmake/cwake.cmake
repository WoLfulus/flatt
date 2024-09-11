#
# Heavily opinionated build scripts for C/C++ projects
# using Conan packages and external projects.
#
# https://github.com/WoLfulus
#

# set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
# set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
# set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CWAKE_CMAKE_VERSION 3.21)
set(CWAKE_SCRIPT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set(CWAKE_TOOL_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/tools")
list(APPEND CMAKE_PROGRAM_PATH CWAKE_TOOL_DIRECTORY)


set(CWAKE_DEPENDENCY_ROOT "${CWAKE_SCRIPT_DIRECTORY}/.cwake/cache/dependencies")
set(CWAKE_THIRDPARTY_BUILD_ROOT "${CWAKE_SCRIPT_DIRECTORY}/.cwake/cache/thirdparty")

set(CWAKE_DEPENDENCY_CONAN_URL "https://raw.githubusercontent.com/conan-io/cmake-conan/451fa97d2c59c07b13fb4812a64b2a6391f9e781/conan_provider.cmake")
set(CWAKE_DEPENDENCY_CONAN_HASH "e2cb125128df25470201019d2f16eb18102e958c775c0eb445b6577af8963a3d")

list(APPEND CMAKE_MODULE_PATH ${CWAKE_SCRIPT_DIRECTORY})
list(APPEND CMAKE_PREFIX_PATH ${CWAKE_SCRIPT_DIRECTORY})
list(APPEND CMAKE_MODULE_PATH ${CWAKE_DEPENDENCY_ROOT})
list(APPEND CMAKE_PREFIX_PATH ${CWAKE_DEPENDENCY_ROOT})

#
# -- Logging
#

function(cwake_log msg)
  message(STATUS "[cwake] ${msg}")
endfunction()


function(cwake_log_line msg)
  message(STATUS "[cwake] ${msg}\n---------------------------------------------------------------------------")
endfunction()


function(cwake_log_header msg)
  message(STATUS "\n---------------------------------------------------------------------------\n[cwake] ${msg}\n---------------------------------------------------------------------------\n--")
endfunction()

#
# -- Dependencies
#

macro(cwake_dependency_init NAME)
  string(TOUPPER ${NAME} _dep_name)
  string(TOLOWER ${NAME} _dep_file)

  set(_dep_url ${CWAKE_DEPENDENCY_${_dep_name}_URL})
  set(_dep_hash ${CWAKE_DEPENDENCY_${_dep_name}_HASH})
  set(_dep_path ${CWAKE_DEPENDENCY_ROOT}/${_dep_file}.cmake)

  if(NOT EXISTS "${CWAKE_DEPENDENCY_ROOT}")
    make_directory("${CWAKE_DEPENDENCY_ROOT}")
  endif()

  if(NOT EXISTS "${_dep_path}")
    cwake_log("Downloading ${_dep_name} dependency from ${_dep_url}")
    file(DOWNLOAD "${_dep_url}" "${_dep_path}.tmp"
                  EXPECTED_HASH SHA256=${_dep_hash}
                  TLS_VERIFY ON)

    file(COPY_FILE "${_dep_path}.tmp" "${_dep_path}")
    file(REMOVE "${_dep_path}.tmp")
  endif()

  include(${_dep_path})
endmacro()

macro(cwake_thirdparty_init)
  if(NOT EXISTS "${CWAKE_THIRDPARTY_BUILD_ROOT}/${OPT_CONAN_PROFILE_NAME}")
    make_directory("${CWAKE_THIRDPARTY_BUILD_ROOT}/${OPT_CONAN_PROFILE_NAME}")
  endif()

  file(GLOB _thirdparties "${CWAKE_THIRDPARTY_DIRECTORY}/*")
  foreach(_thirdparty ${_thirdparties})
    if(IS_DIRECTORY ${_thirdparty})
      get_filename_component(_thirdparty_name ${_thirdparty} NAME)
      if(EXISTS "${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/CMakeLists.txt")

        cwake_thirdparty_list_dir(${_thirdparty_name} _thirdparty_list_dir)
        cwake_thirdparty_build_dir(${_thirdparty_name} _thirdparty_build_dir)
        cwake_thirdparty_install_dir(${_thirdparty_name} _thirdparty_install_dir)

        if(NOT EXISTS "${_thirdparty_build_dir}")
          make_directory("${_thirdparty_build_dir}")
        endif()

        if(NOT EXISTS "${_thirdparty_install_dir}")
          execute_process(
            COMMAND
              ${CMAKE_COMMAND}
              "-G" "${CMAKE_GENERATOR}"
              "-A" "${CMAKE_GENERATOR_PLATFORM}"
              "-DCMAKE_INSTALL_PREFIX=${_thirdparty_install_dir}"
              "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
              "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
              "-DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}"
              "-DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}"
              "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
              "-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}"
              "-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}"
              "-DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW"
              "-DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}"
              "-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=${CMAKE_PROJECT_TOP_LEVEL_INCLUDES}"
              "-DCWAKE_THIRDPARTY_ROOT=${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/"
              "-DCWAKE_THIRDPARTY_BINARY_DIR=${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/bin/"
              "-DCWAKE_THIRDPARTY_INSTALL_DIR=${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/install/"
              "-DCWAKE_THIRDPARTY_SOURCE_DIR=${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/src/"
              "-DCWAKE_THIRDPARTY_DOWNLOAD_DIR=${CWAKE_THIRDPARTY_DIRECTORY}/${_thirdparty_name}/download/"
              "${_thirdparty_list_dir}"
            WORKING_DIRECTORY
              "${_thirdparty_build_dir}"
            RESULT_VARIABLE _error
          )

          if(NOT _error EQUAL 0)
            message(FATAL_ERROR "CMAKE CONFIGURE FAILED")
          endif()

          execute_process(
            COMMAND
              ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE}
            WORKING_DIRECTORY
              "${_thirdparty_build_dir}"
            RESULT_VARIABLE _error
          )

          if(NOT _error EQUAL 0)
            message(FATAL_ERROR "CMAKE BUILD FAILED")
          endif()

          execute_process(
            COMMAND
              ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE}
            WORKING_DIRECTORY
              "${_thirdparty_build_dir}"
            RESULT_VARIABLE _error
          )

          if(NOT _error EQUAL 0)
            message(FATAL_ERROR "CMAKE INSTALL FAILED")
          endif()

        endif()
      endif()
    endif()
  endforeach()
endmacro()

function(cwake_thirdparty_list_dir name output)
  set(${output} "${CWAKE_THIRDPARTY_DIRECTORY}/${name}" PARENT_SCOPE)
endfunction()

function(cwake_thirdparty_build_dir name output)
  set(${output} "${CWAKE_THIRDPARTY_BUILD_ROOT}/${OPT_CONAN_PROFILE_NAME}/${name}/${CMAKE_GENERATOR_PLATFORM}/build" PARENT_SCOPE)
endfunction()

function(cwake_thirdparty_install_dir name output)
  set(${output} "${CWAKE_THIRDPARTY_BUILD_ROOT}/${OPT_CONAN_PROFILE_NAME}/${name}/${CMAKE_GENERATOR_PLATFORM}/install" PARENT_SCOPE)
endfunction()

#
# -- Bootstrapping
#

macro(cwake_bootstrap root)
  cwake_log_line("bootstrapping")

  cwake_default(CMAKE_BUILD_TYPE "Debug")
  cwake_default(OPT_OS "win")
  cwake_default(OPT_ARCH "x86")

  cwake_default(OPT_VARIANT "")
  set(VARIANT_VALUE ${OPT_VARIANT})
  if (VARIANT_VALUE STREQUAL "")
    set(VARIANT_VALUE "-${VARIANT_VALUE}")
  endif()

  cwake_default(OPT_CONFIG "${CMAKE_BUILD_TYPE}")
  cwake_default(OPT_EXTERNAL_ROOT "external")
  cwake_default(OPT_PACKAGES_ROOT "packages")
  cwake_default(OPT_THIRDPARTY_ROOT "thirdparty")
  cwake_default(OPT_PACKAGE_FILTER "*")
  cwake_default(OPT_CONAN_PROFILE_NAME "${OPT_OS}-${OPT_ARCH}${VARIANT_VALUE}-${OPT_CONFIG}")
  cwake_default(OPT_CONTEXT_NAME "${OPT_OS}-${OPT_ARCH}${VARIANT_VALUE}-${OPT_CONFIG}")

  set(CWAKE_ROOT_DIRECTORY "${root}")
  set(CWAKE_ROOT_DIRECTORY "${root}")
  set(CWAKE_DIST_DIRECTORY "${root}/dist/${OPT_CONTEXT_NAME}")
  set(CWAKE_BUILD_DIRECTORY "${root}/build/${OPT_CONTEXT_NAME}")
  set(CWAKE_PACKAGES_DIRECTORY "${CWAKE_ROOT_DIRECTORY}/${OPT_PACKAGES_ROOT}")
  set(CWAKE_THIRDPARTY_DIRECTORY "${CWAKE_ROOT_DIRECTORY}/${OPT_THIRDPARTY_ROOT}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CWAKE_DIST_DIRECTORY}")
  file(MAKE_DIRECTORY "${CWAKE_DIST_DIRECTORY}")

  cmake_minimum_required(VERSION ${CWAKE_CMAKE_VERSION})

  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  include(ProcessorCount)
  include(ExternalProject)
  include(FetchContent)

  ProcessorCount(CWAKE_PROCESSOR_COUNT_OS)
  if("${CWAKE_PROCESSOR_COUNT_OS}" STREQUAL "0")
    set(CWAKE_PROCESSOR_COUNT_OS 4)
  endif()

  if ("$ENV{CWAKE_PROCESSOR_COUNT}" STREQUAL "")
    set(CWAKE_PROCESSOR_COUNT "${CWAKE_PROCESSOR_COUNT_OS}")
  else()
    set(CWAKE_PROCESSOR_COUNT $ENV{CWAKE_PROCESSOR_COUNT})
  endif()

  if("${CWAKE_PROCESSOR_COUNT}" STREQUAL "0")
    set(CWAKE_PROCESSOR_COUNT "${CWAKE_PROCESSOR_COUNT_OS}")
  endif()

  message(STATUS "Compilation concurrency: ${CWAKE_PROCESSOR_COUNT}")

  if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP${CWAKE_PROCESSOR_COUNT} /openmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${CWAKE_PROCESSOR_COUNT} /openmp /std:c++latest")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  endif(MSVC)

  # -- conan

  if(POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
  endif()

  if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
  endif()

  # Configuration types

  if(NOT DEFINED CMAKE_BUILD_TYPE)
    if(DEFINED CMAKE_BUILD_TYPE_INIT)
      set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_INIT})
    else()
      if(DEFINED CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES})
      else()
        set(CMAKE_BUILD_TYPE "Debug")
      endif()
    endif()
  endif()

  cwake_enable_tests(google)

  cwake_thirdparty_init()
  cwake_package_init()

  cwake_log_line("end")
endmacro()

#
# -- Utilities
#

macro(cwake_default name value)
  if(NOT DEFINED ${name})
    set(${name} "${value}")
  endif()
endmacro()

#
# -- Testing
#

macro(cwake_enable_tests name)
  if (${name} STREQUAL "google")
    enable_testing()
    include(GoogleTest)
    option(MY_PROJECT_TESTS "Build unit tests" ON) # Test explorer?
  else()
    cwake_error("unsupported test: ${name}")
  endif()
endmacro()

#
# -- Global KV
#

function(_cwake_global_get key output)
  get_property(key_exists GLOBAL PROPERTY "${key}" SET)
  if(${key_exists})
    get_property(value GLOBAL PROPERTY "${key}")
    set(${output} "${value}" PARENT_SCOPE)
  endif()
endfunction()

function(_cwake_global_tryget key default output)
  set(value ${default})
  get_property(key_exists GLOBAL PROPERTY "${key}" SET)
  if(${key_exists})
    get_property(value GLOBAL PROPERTY "${key}")
  endif()
  set(${output} "${value}" PARENT_SCOPE)
endfunction()

function(_cwake_global_set key value)
  set_property(GLOBAL PROPERTY "${key}" "${value}")
endfunction()

#
# -- KV internals
#

function(_cwake_kv_group_func_ group prefix)
  string(SHA1 group_id "${group}")
  string(TOUPPER ${group_id} group_id)
  set("${prefix}id" ${group_id} PARENT_SCOPE)
  set("${prefix}keys_ref" "cwake.kv.${group_id}.__keys__" PARENT_SCOPE)
endfunction()

function(_cwake_kv_group_value_ref group key output)
  _cwake_kv_group_func_(${group} this_)

  string(SHA1 key "${key}")
  string(TOUPPER ${key} key)
  set(${output} "cwake.kv.${this_id}.__values__.${key}" PARENT_SCOPE)
endfunction()

function(_cwake_kv_group_key_add group key)
  _cwake_kv_group_func_(${group} this_)

  _cwake_global_tryget(${this_keys_ref} "" current_keys)
  list(FIND current_keys ${key} key_exists)
  if(${key_exists} LESS 0)
    list(APPEND current_keys ${key})
    _cwake_global_set("${this_keys_ref}" "${current_keys}")
  endif()
endfunction()

#
# -- KV functions
#

function(cwake_kv_init group)
  _cwake_kv_group_func_(${group} this_)
  _cwake_global_set("${this_keys_ref}" "")
endfunction()

function(cwake_kv_keys group output)
  _cwake_kv_group_func_(${group} this_)

  _cwake_global_get(${this_keys_ref} keys)
  set(${output} "${keys}" PARENT_SCOPE)
endfunction()

function(cwake_kv_set group key value)
  _cwake_kv_group_func_(${group} this_)

  _cwake_kv_group_key_add(${group} ${key})
  _cwake_kv_group_value_ref(${group} ${key} value_ref)
  _cwake_global_set("${value_ref}" "${value}")
endfunction()

function(cwake_kv_get group key output)
  _cwake_kv_group_func_(${group} this_)

  _cwake_kv_group_value_ref(${group} ${key} value_ref)
  _cwake_global_get("${value_ref}" value)
  set(${output} "${value}" PARENT_SCOPE)
endfunction()

function(cwake_kv_tryget group key default output)
  _cwake_kv_group_func_(${group} this_)

  _cwake_kv_group_value_ref(${group} ${key} value_ref)
  _cwake_global_tryget("${value_ref}" "${default}" value)
  set(${output} "${value}" PARENT_SCOPE)
endfunction()

function(cwake_kv_expand group prefix)
  _cwake_kv_group_func_(${group} this_)

  _cwake_global_tryget(${this_keys_ref} "" current_keys)
  foreach(key ${current_keys})
    _cwake_kv_group_value_ref(${group} ${key} value_ref)
    _cwake_global_get(${value_ref} value)
    set("${prefix}${key}" "${value}" PARENT_SCOPE)
  endforeach()
endfunction()

#
# -- Event
#

macro(cwake_trigger_event name)
  cwake_log_header("TRIGGERING EVENT: ${name}")
  if(COMMAND ${name})
    cmake_language(CALL ${name})
  endif()
endmacro()

#
# -- Others
#

function(cwake_set_scoped_property scope_name name value)
  set(props "")
  get_property(scoped_props GLOBAL PROPERTY "CWAKE_KV_ENTRIES_${CWAKE_KV_GROUP}" SET)
  if(${scoped_props})
    get_property(props GLOBAL PROPERTY "CWAKE_KV_ENTRIES_${CWAKE_KV_GROUP}")
  endif()
  string(SHA1 scope_id "${scope_name}")
  string(TOUPPER ${scope_id} scope_id)
  list(APPEND props "SCOPED_${scope_id}_${name}")
  set_property(GLOBAL PROPERTY "CWAKE_KV_ENTRIES_${CWAKE_KV_GROUP}" "${props}")
  set_property(GLOBAL PROPERTY "SCOPED_${scope_id}_${name}" "${value}")
endfunction()

function(cwake_get_scoped_property scope_name name output)
  string(SHA1 scope "${scope_name}")
  string(TOUPPER ${scope} scope)
  get_property(val GLOBAL PROPERTY "SCOPED_${scope}_${name}")
  set("${output}" "${val}" PARENT_SCOPE)
endfunction()

#
# -- Packages
#

#
# Macros
#

macro(cwake_package_configure_tests)
  file(GLOB_RECURSE TEST_FILES "${PACKAGE_TEST_DIRECTORY}/*_test.cpp")

  foreach(TEST_FILE ${TEST_FILES})
    file(RELATIVE_PATH TEST_FILE_RELATIVE ${PACKAGE_TEST_DIRECTORY} ${TEST_FILE})
    string(REGEX REPLACE ".cpp$" "" TEST_FILE_BASENAME ${TEST_FILE_RELATIVE})
    string(REGEX REPLACE "^.*/" "" TEST_FILE_BASENAME ${TEST_FILE_BASENAME})
    string(REGEX REPLACE ".cpp$" "" TEST_NAME ${TEST_FILE_RELATIVE})
    string(REGEX REPLACE "/" "_" TEST_NAME ${TEST_NAME})
    set(TEST_TARGET "${PACKAGE_NAME}_${TEST_NAME}")
    add_executable(${TEST_TARGET} ${TEST_FILE})
    target_link_libraries(${TEST_TARGET} PRIVATE ${PACKAGE_LINK})
    target_include_directories(${TEST_TARGET}
      PRIVATE
        ${PACKAGE_INCLUDE_DIRECTORY}
        ${PACKAGE_SOURCE_DIRECTORY}
        ${PACKAGE_RESOURCE_DIRECTORY}
    )
    set_target_properties(${TEST_TARGET}
      PROPERTIES
        VS_JUST_MY_CODE_DEBUGGING ON
        RUNTIME_OUTPUT_DIRECTORY "${PACKAGE_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PACKAGE_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PACKAGE_OUTPUT_DIRECTORY}"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    gtest_discover_tests(${TEST_TARGET})
  endforeach()
endmacro()

#######################

macro(cwake_package_init)
  cwake_package_list(packages)
  foreach(_pkg ${packages})
    set(PACKAGE_ID ${_pkg})
    add_subdirectory("${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}")
  endforeach()
  foreach(_pkg ${packages})
    cwake_package_setup(ID ${_pkg})
  endforeach()
endmacro()

function(cwake_package_list output)
  file(GLOB children "${CWAKE_PACKAGES_DIRECTORY}/${OPT_PACKAGE_FILTER}")
  set(PACKAGES "")
  foreach(child ${children})
    if(IS_DIRECTORY ${child})
      get_filename_component(PACKAGE ${child} NAME)
      if(NOT EXISTS "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE}/.skip")
        if(EXISTS "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE}/CMakeLists.txt")
          list(APPEND PACKAGES ${PACKAGE})
        elseif(EXISTS "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE}/conanfile.py")
          list(APPEND PACKAGES ${PACKAGE})
        elseif(EXISTS "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE}/conanfile.txt")
          list(APPEND PACKAGES ${PACKAGE})
        endif()
      endif()
    endif()
  endforeach()
  set(${output} "${PACKAGES}" PARENT_SCOPE)
endfunction()

macro(cwake_package_info)
  cmake_parse_arguments(ARGS "" "NAME;TYPE" "USES;LINK" ${ARGN})

  cwake_kv_init("${ARGS_NAME}-package")
  cwake_kv_set("${ARGS_NAME}-package" ID "${PACKAGE_ID}")

  cwake_kv_init(${PACKAGE_ID})
  cwake_kv_set(${PACKAGE_ID} ID "${PACKAGE_ID}")
  cwake_kv_set(${PACKAGE_ID} NAME "${ARGS_NAME}")
  cwake_kv_set(${PACKAGE_ID} TYPE "${ARGS_TYPE}")
  cwake_kv_set(${PACKAGE_ID} REFERENCES "${ARGS_USES}")
  cwake_kv_set(${PACKAGE_ID} LINK "${ARGS_LINK}")
  cwake_kv_set(${PACKAGE_ID} DIR "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}")
  cwake_kv_set(${PACKAGE_ID} DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}")
  cwake_kv_set(${PACKAGE_ID} DATA_DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}/data")
  cwake_kv_set(${PACKAGE_ID} RESOURCE_DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}/res")
  cwake_kv_set(${PACKAGE_ID} SOURCE_DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}/src")
  cwake_kv_set(${PACKAGE_ID} INCLUDE_DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}/include")
  cwake_kv_set(${PACKAGE_ID} TEST_DIRECTORY "${CWAKE_PACKAGES_DIRECTORY}/${PACKAGE_ID}/test")
  cwake_kv_set(${PACKAGE_ID} BUILD_DIRECTORY "${CWAKE_BUILD_DIRECTORY}/${OPT_PACKAGES_ROOT}/${ARGS_NAME}")
  cwake_kv_set(${PACKAGE_ID} CONAN_DIRECTORY "${CWAKE_BUILD_DIRECTORY}/${OPT_PACKAGES_ROOT}/${ARGS_NAME}/.conan")
  cwake_kv_set(${PACKAGE_ID} OUTPUT_DIRECTORY "${CWAKE_DIST_DIRECTORY}/${ARGS_NAME}")
  cwake_kv_set(${PACKAGE_ID} DATA_OUTPUT_DIRECTORY "${CWAKE_DIST_DIRECTORY}/${ARGS_NAME}/data/")
endmacro()

function(cwake_package_build_dir name output)
  cwake_kv_get("${name}-package" ID _pkg_id)
  cwake_kv_get(${_pkg_id} BUILD_DIRECTORY _pkg_dir)
  set(${output} "${_pkg_dir}" PARENT_SCOPE)
endfunction()

function(cwake_package_output_dir name output)
  cwake_kv_get("${name}-package" ID _pkg_id)
  cwake_kv_get(${_pkg_id} OUTPUT_DIRECTORY _pkg_dir)
  set(${output} "${_pkg_dir}" PARENT_SCOPE)
endfunction()

function(cwake_package_dir name output)
  cwake_kv_get("${name}-package" ID _pkg_id)
  cwake_kv_get(${_pkg_id} DIR _pkg_dir)
  set(${output} "${_pkg_dir}" PARENT_SCOPE)
endfunction()

function(cwake_package_depends name type input)

  cwake_package_dir(${name} _package_dir)

  cwake_log("depends: ${name} ${type} ${input} (${_package_dir})")
  file(GLOB_RECURSE _collected_files CONFIGURE_DEPENDS LIST_DIRECTORIES true "${input}")

  if (EXISTS ${input})
    list(APPEND _collected_files ${input})
  endif()

  foreach(_current ${_collected_files})
    file(RELATIVE_PATH _relative_path ${input} ${_current})
    if(NOT IS_DIRECTORY ${_current})
      configure_file(${_current} "${$ENV{TMP}}/cwake_temp" COPYONLY)
    endif()
  endforeach()

endfunction()

macro(cwake_set_conan_target_vars)
  if(NOT "${CONAN_LIBS}" STREQUAL "")
    set(TARGET_CONAN_LIBS "${CONAN_LIBS}")
  elseif(NOT "${CONAN_LIBS_DEBUG}" STREQUAL "")
    set(TARGET_CONAN_LIBS "${CONAN_LIBS_DEBUG}")
  elseif(NOT "${CONAN_LIBS_RELEASE}" STREQUAL "")
    set(TARGET_CONAN_LIBS "${CONAN_LIBS_RELEASE}")
  else()
    set(TARGET_CONAN_LIBS "")
  endif()

  if(NOT "${CONAN_INCLUDE_DIRS}" STREQUAL "")
    set(TARGET_CONAN_INCLUDE_DIRS "${CONAN_INCLUDE_DIRS}")
  elseif(NOT "${CONAN_INCLUDE_DIRS_DEBUG}" STREQUAL "")
    set(TARGET_CONAN_INCLUDE_DIRS "${CONAN_INCLUDE_DIRS_DEBUG}")
  elseif(NOT "${CONAN_INCLUDE_DIRS_RELEASE}" STREQUAL "")
    set(TARGET_CONAN_INCLUDE_DIRS "${CONAN_INCLUDE_DIRS_RELEASE}")
  else()
    set(TARGET_CONAN_INCLUDE_DIRS "")
  endif()

  message(STATUS "=== CONAN LIB DIRS ===")
  foreach(_dir ${TARGET_CONAN_LIBS})
    message(STATUS " > ${_dir}")
  endforeach()

  message(STATUS "=== CONAN INCLUDE DIRS ===")
  foreach(_dir ${TARGET_CONAN_INCLUDE_DIRS})
    message(STATUS " > ${_dir}")
  endforeach()
endmacro()

macro(cwake_package_setup)
  cmake_parse_arguments(PACKAGE "" "ID" "" ${ARGN})
  cwake_kv_expand("${PACKAGE_ID}" PACKAGE_)

  file(MAKE_DIRECTORY ${PACKAGE_CONAN_DIRECTORY})

  set(_EVENT "on_${PACKAGE_NAME}_package_setup")
  cwake_trigger_event(${_EVENT})

  if(NOT ("${PACKAGE_TYPE}" STREQUAL "cmake"))
    #conan_cmake_autodetect(settings)
    #conan_cmake_install(PATH_OR_REFERENCE ${PACKAGE_DIRECTORY}
    #                    BUILD missing
    #                    INSTALL_FOLDER ${PACKAGE_CONAN_DIRECTORY}
    #                    SETTINGS ${settings})

    #if(EXISTS "${PACKAGE_CONAN_DIRECTORY}/conanbuildinfo_multi.cmake")
    #  include("${PACKAGE_CONAN_DIRECTORY}/conanbuildinfo_multi.cmake")
    #else()
    #  include("${PACKAGE_CONAN_DIRECTORY}/conanbuildinfo.cmake")
    #endif()

    #conan_basic_setup()
    #cwake_set_conan_target_vars()
  endif()

  set(PACKAGE_ADDITIONAL_INCLUDE_DIRECTORY "")
  foreach(ref ${PACKAGE_REFERENCES})
    cwake_kv_get("${ref}-package" ID ref_id)
    if("${ref_id}" STREQUAL "")
      message(FATAL_ERROR "package [${PACKAGE_NAME}] uses unknown or misconfigured package [${ref}]")
    endif()
    cwake_kv_get(${ref_id} INCLUDE_DIRECTORY REF_INCLUDE_DIRECTORY)
    list(APPEND PACKAGE_ADDITIONAL_INCLUDE_DIRECTORY ${REF_INCLUDE_DIRECTORY})
  endforeach()

  list(APPEND PACKAGE_ADDITIONAL_INCLUDE_DIRECTORY ${TARGET_CONAN_INCLUDE_DIRS})
  list(APPEND PACKAGE_LINK ${TARGET_CONAN_LIBS})

  set(RES_HEADERS "")
  if(EXISTS "${PACKAGE_RESOURCE_DIRECTORY}")
    file(GLOB_RECURSE RES_HEADERS "${PACKAGE_RESOURCE_DIRECTORY}/*.h")
    file(GLOB_RECURSE RES_RCFILES "${PACKAGE_RESOURCE_DIRECTORY}/*.rc")
    list(APPEND RES_HEADERS ${RES_RCFILES})
  endif()

  file(GLOB_RECURSE INC_HEADERS "${PACKAGE_INCLUDE_DIRECTORY}/*.h")
  file(GLOB_RECURSE INC_HEADERSPP "${PACKAGE_INCLUDE_DIRECTORY}/*.hpp")
  list(APPEND INC_HEADERS ${INC_HEADERSPP})

  file(GLOB_RECURSE SRC_HEADERS "${PACKAGE_SOURCE_DIRECTORY}/*.h")
  file(GLOB_RECURSE SRC_HEADERSPP "${PACKAGE_SOURCE_DIRECTORY}/*.hpp")
  list(APPEND SRC_HEADERS ${SRC_HEADERSPP})

  file(GLOB_RECURSE SRC_SOURCES "${PACKAGE_SOURCE_DIRECTORY}/*.cpp")
  file(GLOB_RECURSE SRC_SOURCESPP "${PACKAGE_SOURCE_DIRECTORY}/*.c++")
  list(APPEND SRC_SOURCES ${SRC_SOURCESPP})

  # file(GLOB_RECURSE INC_MODULES "${PACKAGE_INCLUDE_DIRECTORY}/*.cxx")
  # file(GLOB_RECURSE SRC_MODULES "${PACKAGE_SOURCE_DIRECTORY}/*.cxx")

  # cwake_package_depends(${PACKAGE_ID} DIRECTORY "${PACKAGE_RESOURCE_DIRECTORY}/*")
  # cwake_package_depends(${PACKAGE_ID} DIRECTORY "${PACKAGE_INCLUDE_DIRECTORY}/*")
  # cwake_package_depends(${PACKAGE_ID} DIRECTORY "${PACKAGE_SOURCE_DIRECTORY}/*")

  if(NOT ("${PACKAGE_TYPE}" STREQUAL "cmake"))
    if("${PACKAGE_TYPE}" STREQUAL "headers")
      add_library(${PACKAGE_NAME} INTERFACE)
      target_include_directories(${PACKAGE_NAME}
        INTERFACE
          ${PACKAGE_ADDITIONAL_INCLUDE_DIRECTORY}
          ${PACKAGE_INCLUDE_DIRECTORY}
          ${PACKAGE_SOURCE_DIRECTORY}
      )
      set_target_properties(${PACKAGE_NAME}
        PROPERTIES
          LINKER_LANGUAGE CXX
      )
    else()
      if("${PACKAGE_TYPE}" STREQUAL "console")
        add_executable(${PACKAGE_NAME} ${RES_HEADERS} ${INC_HEADERS} ${SRC_HEADERS} ${INC_SOURCES} ${SRC_SOURCES})
      elseif("${PACKAGE_TYPE}" STREQUAL "application")
        add_executable(${PACKAGE_NAME} WIN32 ${RES_HEADERS} ${INC_HEADERS} ${SRC_HEADERS} ${INC_SOURCES} ${SRC_SOURCES})
      elseif("${PACKAGE_TYPE}" STREQUAL "library")
        add_library(${PACKAGE_NAME} STATIC ${RES_HEADERS} ${INC_HEADERS} ${SRC_HEADERS} ${INC_SOURCES} ${SRC_SOURCES})
      else()
        message(FATAL "invalid package type: ${PACKAGE_TYPE}")
      endif()
      # target_sources(${PACKAGE_NAME}
      #   PUBLIC
      #     FILE_SET cxx_modules TYPE CXX_MODULES FILES
      #       ${INC_MODULES}
      #       ${SRC_MODULES}
      # )

      # target_include_directories(${PACKAGE_NAME}
      #   PRIVATE
      #     ${PACKAGE_INCLUDE_DIRECTORY}
      #     ${PACKAGE_RESOURCE_DIRECTORY}
      #     ${PACKAGE_SOURCE_DIRECTORY}
      #   PUBLIC
      #     ${PACKAGE_ADDITIONAL_INCLUDE_DIRECTORY}
      # )

      target_link_libraries(${PACKAGE_NAME} ${PACKAGE_LINK})
      if (MSVC)
        set_target_properties(${PACKAGE_NAME}
          PROPERTIES
            VS_JUST_MY_CODE_DEBUGGING ON
            RUNTIME_OUTPUT_DIRECTORY "${PACKAGE_OUTPUT_DIRECTORY}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PACKAGE_OUTPUT_DIRECTORY}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PACKAGE_OUTPUT_DIRECTORY}"
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
      endif()
    endif()

    if(EXISTS "${PACKAGE_SOURCE_DIRECTORY}/pch.h")
      target_precompile_headers(${PACKAGE_NAME} PRIVATE
        "${PACKAGE_SOURCE_DIRECTORY}/pch.h"
      )
    endif()
  else()

  endif()

  set(_EVENT "on_${PACKAGE_NAME}_package_configure")
  cwake_trigger_event(${_EVENT})

  if(EXISTS "${PACKAGE_DATA_DIRECTORY}")
    file(GLOB_RECURSE _collected_files "${PACKAGE_DATA_DIRECTORY}/*")
    foreach(_current ${_collected_files})
      file(RELATIVE_PATH _relative_path ${PACKAGE_DATA_DIRECTORY} ${_current})
      if(NOT IS_DIRECTORY ${_current})
        configure_file(${_current} "${PACKAGE_DATA_OUTPUT_DIRECTORY}/${_relative_path}" COPYONLY)
      endif()
    endforeach()
  endif()

  cwake_package_configure_tests()

endmacro()

#
# -- Tools
#

function(cwake_tool)
  cmake_parse_arguments(ARG "" "TOOL;WORKING_DIRECTORY" "ARGUMENTS" ${ARGN})

  set(_arguments "${ARG_ARGUMENTS}")
  if("${_working_dir}" STREQUAL "")
    set(_working_dir "${CWAKE_ROOT_DIRECTORY}")
  endif()

  set(_extension "")
  if(WIN32)
    set(_extension ".exe")
  endif()

  set(_working_dir "${ARG_WORKING_DIRECTORY}")
  if("${_working_dir}" STREQUAL "")
    set(_working_dir "${CWAKE_ROOT_DIRECTORY}")
  endif()

  set(_command "")
  if(EXISTS "${CWAKE_TOOL_DIRECTORY}/${ARG_TOOL}${_extension}")
    set(_command "${CWAKE_TOOL_DIRECTORY}/${ARG_TOOL}${_extension}")
  elseif(EXISTS "${CWAKE_TOOL_DIRECTORY}/${ARG_TOOL}")
    set(_command "${CWAKE_TOOL_DIRECTORY}/${ARG_TOOL}")
  else()
    find_program(_command ${ARG_TOOL})
  endif()

  cwake_log("Command: ${_command}")
  cwake_log("Arguments: ${_arguments}")
  cwake_log("Env: $ENV{PATH}")

  execute_process(
    COMMAND "${_command}" ${_arguments}
    WORKING_DIRECTORY "${_working_dir}"
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()
