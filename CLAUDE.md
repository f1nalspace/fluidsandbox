# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Fluid Sandbox is a real-time Windows application for simulating 3D fluids and rigid bodies, built on NVIDIA PhysX 3.4.2 with a custom OpenGL renderer. It is a single executable (`FluidSandbox`) — there is no test suite or library output.

## Build

Windows + Visual Studio 2022 only. Open `FluidSandbox.sln` and build, or use MSBuild:

```
msbuild FluidSandbox.sln /p:Configuration=Release /p:Platform=x64
```

Configurations are `Debug`/`Release`, platforms `x64`/`Win32`. The build is driven by `FluidSandbox/FluidSandbox.vcxproj` (the `.vcproj` is a stale legacy file — ignore it).

### PhysX prerequisite

The repo does **not** ship the PhysX SDK binaries. Before the project links, the PhysX 3.4.2 SDK must be compiled as Multithreaded DLLs (not static CRT) and its `.dll`, `.lib`, and header files copied into `Libraries/PhysX/` — see README.md for the exact file lists and target folders. PhysX headers are referenced from `Libraries/PhysX/pxshared/include/` and `Libraries/PhysX/physx/include/`.

To enable the PhysX Visual Debugger, uncomment `#define PVD_ENABLED` near the top of `FluidSandbox.cpp`.

## Architecture

`FluidSandbox/FluidSandbox.cpp` is the entry point and main loop — it owns window/GL setup, input handling, the on-screen display, simulation stepping, and the render pipeline. Everything else is a subsystem it drives.

**Vendored libraries** live in `Libraries/` and are single-header style: `final_platform_layer` (windowing + GL context, `libfpl.org`), `final_dynamic_opengl` (GL loader), `final_xml`, `glm`, and `stb`. Each header-only lib is compiled exactly once via a dedicated `.cpp` (`final_platform_layer.cpp`, `final_dynamic_opengl.cpp`, `final_xml.cpp`); the application entry point comes from FPL via `#define FPL_ENTRYPOINT`.

**Physics** — `PhysicsEngine.{h,cpp}` is the only place that touches the PhysX API. It exposes glm-based wrapper types (`PhysicsTransform`, `PhysicsBoundingBox`, etc.) so the rest of the code never sees PhysX types directly. GPU vs CPU particle acceleration is a runtime toggle.

**Rendering** — there are two renderers. `Renderer.{h,cpp}` is the older direct-GL renderer. `Renderer2.{h,cpp}` is a newer abstraction layer under the `fsr` namespace (typed resource handles like `BufferID`, `Buffer`/`BufferType`/`BufferUsage`) intended as the path toward GL 4.x / multi-backend support. `ScreenSpaceFluidRendering.{h,cpp}` implements the screen-space fluid surface technique (depth → blur → thickness → composite).

**Actors & scenes** — `Actor.hpp` is the base for simulated objects; `Scene` holds the live set of actors and `Scenario` loads/saves scene definitions. Almost all properties (in `FluidProperties.h`, `ValueTypes`, `VariableManager`) can be changed live at runtime.

**Aggregate headers** — `AllShaders.hpp`, `AllFBOs.hpp`, `AllActors.hpp`, `AllVBOs.hpp` each bundle the concrete subclasses of one base type into a single include.

## Runtime assets

These folders are loaded at runtime relative to the executable and are not compiled in:

- `shaders/` — GLSL programs as paired `.vertex` / `.fragment` files; `GLSL.{h,cpp}` loads them.
- `scenarios/` — `Scenario00.xml` … `Scenario10.xml`, the selectable simulation setups (validated against `sceneSchema.xsd`).
- `scene.xml` — top-level scene configuration.
- `textures/` — skybox and surface textures.
