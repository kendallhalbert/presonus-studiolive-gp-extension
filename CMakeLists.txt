cmake_minimum_required(VERSION 3.21)

# ============================================================================
#  PreSonus StudioLive Gig Performer Extension
#  ---------------------------------------------------------------------------
#  Builds  PreSonusStudioLive.dll  which Gig Performer loads from
#      C:\Users\Public\Documents\Gig Performer\Extensions\
#  See   docs/GP_EXTENSION_PLAN.md   for the design.
# ============================================================================

project(
  PreSonusStudioLive
  VERSION 1.0.0
  DESCRIPTION "PreSonus StudioLive III control extension for Gig Performer"
  HOMEPAGE_URL "https://github.com/featherbear/presonus-studiolive-api"
  LANGUAGES C CXX)

# ----------------------------------------------------------------------------
#  Project-wide defaults
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

# Group SDK and dependency targets under a "third-party/" folder in IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER third-party)

# Windows-only by design (see §1 of the plan).
if(NOT WIN32)
  message(FATAL_ERROR
    "This extension is Windows-only. macOS/Linux are out of scope for v1 "
    "(see docs/GP_EXTENSION_PLAN.md §1).")
endif()

# Single-config Ninja or Make? Force a default config so installs are sane.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Default build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()

# Put every DLL/EXE in a single sub-tree so the install paths are predictable.
foreach(_cfg "" Debug Release RelWithDebInfo MinSizeRel)
  if(_cfg STREQUAL "")
    set(_suffix "")
  else()
    string(TOUPPER "_${_cfg}" _suffix)
  endif()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${_suffix} "${CMAKE_BINARY_DIR}/bin/${_cfg}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY${_suffix} "${CMAKE_BINARY_DIR}/bin/${_cfg}")
endforeach()

# ----------------------------------------------------------------------------
#  GP SDK location
# ----------------------------------------------------------------------------
set(GP_SDK_DIR "C:/Users/KenHa/source/repos/gigperformer/gp-sdk"
  CACHE PATH "Path to a local clone of the Gig Performer SDK.")

if(NOT EXISTS "${GP_SDK_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "GP_SDK_DIR='${GP_SDK_DIR}' does not contain a CMakeLists.txt. "
    "Clone https://github.com/gigperformer/gp-sdk and pass "
    "-DGP_SDK_DIR=<path> to CMake.")
endif()

add_subdirectory("${GP_SDK_DIR}" "${CMAKE_BINARY_DIR}/_gp-sdk" EXCLUDE_FROM_ALL)

# ----------------------------------------------------------------------------
#  Options
# ----------------------------------------------------------------------------
option(PSL_BUILD_TESTS  "Build unit tests"                 ON)
option(PSL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Where 'cmake --install' drops the DLL. Override with -DPSL_INSTALL_DIR=... .
set(PSL_INSTALL_DIR "C:/Users/Public/Documents/Gig Performer/Extensions"
  CACHE PATH "Directory Gig Performer scans for extension DLLs.")

# ----------------------------------------------------------------------------
#  Subdirectories
# ----------------------------------------------------------------------------
set(CMAKE_FOLDER "")
add_subdirectory(extension)

if(PSL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ----------------------------------------------------------------------------
#  Summary banner
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "  ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  ----------------------------------------------------------")
message(STATUS "  GP SDK            : ${GP_SDK_DIR}")
message(STATUS "  Build tests       : ${PSL_BUILD_TESTS}")
message(STATUS "  Warnings -> errors: ${PSL_WARNINGS_AS_ERRORS}")
message(STATUS "  Install target    : ${PSL_INSTALL_DIR}")
message(STATUS "  Generator         : ${CMAKE_GENERATOR}")
message(STATUS "")
