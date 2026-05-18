# ============================================================================
#  extension/  -  the PreSonusStudioLive.dll target itself
# ============================================================================

add_library(PreSonusStudioLive SHARED
  src/LibMain.h
  src/LibMain.cpp
  src/Version.h)

target_link_libraries(PreSonusStudioLive
  PRIVATE
    gigperformer::sdk::cpp)

target_include_directories(PreSonusStudioLive
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(PreSonusStudioLive PROPERTIES
  OUTPUT_NAME    PreSonusStudioLive
  PREFIX         ""                # avoid "lib" prefix on MinGW etc.
  CXX_STANDARD   20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
#  Compiler / linker flags
# ----------------------------------------------------------------------------
if(MSVC)
  target_compile_options(PreSonusStudioLive PRIVATE
    /W4
    /permissive-
    /utf-8
    /Zc:__cplusplus
    /Zc:preprocessor
    /EHsc)

  target_compile_definitions(PreSonusStudioLive PRIVATE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

  if(PSL_WARNINGS_AS_ERRORS)
    target_compile_options(PreSonusStudioLive PRIVATE /WX)
  endif()
endif()

# Make our project version available to the C++ code.
target_compile_definitions(PreSonusStudioLive PRIVATE
  PSL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PSL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PSL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  PSL_VERSION_TAG="phase0")

# ----------------------------------------------------------------------------
#  Install: drop the DLL into the GP Extensions folder.
#  Run:  cmake --install build --config Debug --component dev
# ----------------------------------------------------------------------------
install(TARGETS PreSonusStudioLive
  COMPONENT  dev
  RUNTIME    DESTINATION "${PSL_INSTALL_DIR}"
  LIBRARY    DESTINATION "${PSL_INSTALL_DIR}")

# ----------------------------------------------------------------------------
#  Expose the target to the test tree (so tests can later link against
#  individual translation units once Phase 1 starts producing a static lib).
# ----------------------------------------------------------------------------
add_library(PreSonusStudioLive::PreSonusStudioLive ALIAS PreSonusStudioLive)
