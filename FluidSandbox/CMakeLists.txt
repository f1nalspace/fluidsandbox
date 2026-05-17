#
# Fluid Sandbox - CMake build
#
# Builds the FluidSandbox executable for Windows and Linux (x64, Debug/Release).
#
# Recommended usage (single-config generators):
#
#   cmake -S FluidSandbox -B Immediates/linux-x64-debug   -DCMAKE_BUILD_TYPE=Debug
#   cmake --build Immediates/linux-x64-debug
#
#   cmake -S FluidSandbox -B Immediates/linux-x64-release -DCMAKE_BUILD_TYPE=Release
#   cmake --build Immediates/linux-x64-release
#
# Intermediate (object) files land in   <repo>/Immediates/<config>/
# The executable plus runtime assets in <repo>/Build/<config>/
# where <config> is e.g. linux-x64-debug or win32-x64-release.
#
cmake_minimum_required(VERSION 3.16)

project(FluidSandbox CXX)

# --- Build type -------------------------------------------------------------
# Only Debug and Release are supported; default to Debug when unset.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be Debug or Release (got '${CMAKE_BUILD_TYPE}')")
endif()
string(TOLOWER "${CMAKE_BUILD_TYPE}" FS_BUILDTYPE)

# --- Platform / config identifiers -----------------------------------------
if(WIN32)
	set(FS_PLATFORM   "win32")    # token used in Build/Immediates folder names
	set(FS_PHYSX_OS   "windows")  # token used in the PhysX library folder names
else()
	set(FS_PLATFORM   "linux")
	set(FS_PHYSX_OS   "linux")
endif()

set(FS_CONFIG "${FS_PLATFORM}-x64-${FS_BUILDTYPE}")

# Repository root is the parent of this source folder.
get_filename_component(FS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(FS_LIBS_DIR  "${FS_REPO_ROOT}/Libraries")
set(FS_PHYSX_DIR "${FS_LIBS_DIR}/PhysX")
set(FS_OUTPUT_DIR "${FS_REPO_ROOT}/Build/${FS_CONFIG}")

# Drop all executable output (every config) into <repo>/Build/<config>/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                "${FS_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${FS_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${FS_OUTPUT_DIR}")

message(STATUS "FluidSandbox config : ${FS_CONFIG}")
message(STATUS "  Intermediates     : ${CMAKE_BINARY_DIR}")
message(STATUS "  Output            : ${FS_OUTPUT_DIR}")

# --- Sources ----------------------------------------------------------------
# Note: SphereActor.cpp is intentionally excluded - it references a missing
# SphereActor.h and is not part of the Visual Studio project either.
set(FS_SOURCES
	FBO.cpp
	final_dynamic_opengl.cpp
	final_platform_layer.cpp
	final_xml.cpp
	FluidSandbox.cpp
	FontAtlas.cpp
	Frustum.cpp
	GLSL.cpp
	OSLowLevel.cpp
	PhysicsEngine.cpp
	Primitives.cpp
	Renderer.cpp
	Renderer2.cpp
	Scenario.cpp
	Scene.cpp
	ScreenSpaceFluidRendering.cpp
	SphericalPointSprites.cpp
	Texture.cpp
	Texture2D.cpp
	TextureCubemap.cpp
	TextureFont.cpp
	TextureManager.cpp
	Utils.cpp
	ValueTypes.cpp
	VariableManager.cpp
	VBO.cpp
	XMLUtils.cpp
)

add_executable(FluidSandbox ${FS_SOURCES})

set_target_properties(FluidSandbox PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# --- Include directories ----------------------------------------------------
target_include_directories(FluidSandbox PRIVATE
	"${FS_PHYSX_DIR}/physx/include"
	"${FS_PHYSX_DIR}/pxshared/include"
	"${FS_LIBS_DIR}/glm/include"
	"${FS_LIBS_DIR}/stb/include"
	"${FS_LIBS_DIR}/final/include"
)

# --- Preprocessor defines ---------------------------------------------------
if(WIN32)
	target_compile_definitions(FluidSandbox PRIVATE
		WIN32 _CONSOLE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS
	)
else()
	# Both final_platform_layer and final_dynamic_opengl pull in <X11/Xlib.h>,
	# whose macros (None, Always, ...) collide with the renderer's enum values.
	# These switches make both libraries use their own ABI-compatible X11 types.
	target_compile_definitions(FluidSandbox PRIVATE
		FPL_NO_PLATFORM_INCLUDES
		FGL_X11_NO_HEADERS
	)
endif()
# PhysX requires exactly one of NDEBUG / _DEBUG. CMake defines NDEBUG for
# Release; _DEBUG is an MSVC-ism, so define it ourselves for the Debug config.
target_compile_definitions(FluidSandbox PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

# --- PhysX library locations ------------------------------------------------
set(FS_PHYSX_LIBDIR "${FS_PHYSX_OS}-x64-${FS_BUILDTYPE}")

if(WIN32)
	# Windows: link import libraries, distinguished by a DEBUG name suffix.
	target_link_directories(FluidSandbox PRIVATE
		"${FS_PHYSX_DIR}/physx/lib/${FS_PHYSX_LIBDIR}"
		"${FS_PHYSX_DIR}/pxshared/lib/${FS_PHYSX_LIBDIR}"
	)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_link_libraries(FluidSandbox PRIVATE
			PhysX3DEBUG_x64 PhysX3CommonDEBUG_x64 PhysX3ExtensionsDEBUG
			PxFoundationDEBUG_x64 PxTaskDEBUG_x64 PxPvdSDKDEBUG_x64
		)
	else()
		target_link_libraries(FluidSandbox PRIVATE
			PhysX3_x64 PhysX3Common_x64 PhysX3Extensions
			PxFoundation_x64 PxTask_x64 PxPvdSDK_x64
		)
	endif()
else()
	# Linux: PhysX is a shared build; the .so files are both link and runtime
	# targets and carry identical names in the debug/release folders. Static
	# archives (.a) must precede the shared objects they depend on.
	set(FS_PX_BIN    "${FS_PHYSX_DIR}/physx/bin/${FS_PHYSX_LIBDIR}")
	set(FS_PX_LIB    "${FS_PHYSX_DIR}/physx/lib/${FS_PHYSX_LIBDIR}")
	set(FS_PXS_BIN   "${FS_PHYSX_DIR}/pxshared/bin/${FS_PHYSX_LIBDIR}")
	set(FS_PXS_LIB   "${FS_PHYSX_DIR}/pxshared/lib/${FS_PHYSX_LIBDIR}")

	target_link_libraries(FluidSandbox PRIVATE
		"${FS_PX_LIB}/libPhysX3Extensions.a"
		"${FS_PX_BIN}/libPhysX3_x64.so"
		"${FS_PX_BIN}/libPhysX3Cooking_x64.so"
		"${FS_PX_BIN}/libPhysX3CharacterKinematic_x64.so"
		"${FS_PX_BIN}/libPhysX3Common_x64.so"
		"${FS_PXS_BIN}/libPxPvdSDK_x64.so"
		"${FS_PXS_BIN}/libPxFoundation_x64.so"
		"${FS_PXS_LIB}/libPxTask.a"
		"${FS_PXS_LIB}/libPsFastXml.a"
		pthread dl rt m
	)

	# Find the bundled PhysX .so files next to the executable at runtime.
	set_target_properties(FluidSandbox PROPERTIES
		BUILD_RPATH   "$ORIGIN"
		INSTALL_RPATH "$ORIGIN"
	)
endif()

# --- Post-build: stage runtime assets next to the executable ----------------
set(FS_ASSET_DIRS shaders scenarios textures)
foreach(_dir ${FS_ASSET_DIRS})
	add_custom_command(TARGET FluidSandbox POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/${_dir}" "${FS_OUTPUT_DIR}/${_dir}"
		COMMENT "Staging ${_dir}/")
endforeach()

add_custom_command(TARGET FluidSandbox POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/scene.xml" "${FS_OUTPUT_DIR}/scene.xml"
	COMMENT "Staging scene.xml")

# --- Post-build: stage PhysX runtime binaries -------------------------------
if(WIN32)
	add_custom_command(TARGET FluidSandbox POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${FS_PHYSX_DIR}/physx/bin/${FS_PHYSX_LIBDIR}"        "${FS_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${FS_PHYSX_DIR}/physx/bin/windows-x64-shared"        "${FS_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${FS_PHYSX_DIR}/pxshared/bin/${FS_PHYSX_LIBDIR}"     "${FS_OUTPUT_DIR}"
		COMMENT "Staging PhysX DLLs")
else()
	add_custom_command(TARGET FluidSandbox POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${FS_PHYSX_DIR}/physx/bin/${FS_PHYSX_LIBDIR}"        "${FS_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${FS_PHYSX_DIR}/pxshared/bin/${FS_PHYSX_LIBDIR}"     "${FS_OUTPUT_DIR}"
		COMMENT "Staging PhysX shared objects")
endif()
