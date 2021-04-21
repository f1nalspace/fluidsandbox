# fluidsandbox
A real time application for playing around with 3D fluids and rigid bodies.

##Features
- Fluid and rigid body simulation based on NVIDIA PhysX (R)
- Screen Space Fluid Rendering with clear and colored particle fluids
- Fully customizable scenarios stored in XML
- Changing allmost all simulation properties in real time
- Uses a custom OpenGL rendering engine
- Uses Final Platform Layer (libfpl.org) for creating a window and a opengl rendering context

## Copyright / License
Copyright (C) Torsten Spaete 2011-2021. All rights reserved.

MPL v.2 licensed. See LICENSE.txt for more details.

## Dependencies
- Windows 7 or higher (With a few changes it may compile and run on other platforms too)
- Visual Studio 2019

- PhysX SDK 3.4.2 (Multithreaded DLL, x64/win32, Headers are included)

- STB image (included)
- STB freetype (included)
- Final Platform Layer (included)
- Final XML (included)
- Glad (included)
- glm (included)

##How to compile:

###PhysX SDK:
- Compile the PhysX SDK as Multithreaded DLLs for x64 and x64 platform with the configurations Debug/Release
NOTE: Do not compile with static CRT library, change to (Multithreaded DLL and Multithreaded Debug DLL).

- Copy all 10 resulting PhysX DLL files into the folders for each platform:
	- /Libraries/PhysX/physx/bin/win32/
		- nvToolsExt32_1.dll
		- PhysX3_x86.dll
		- PhysX3Common_x86.dll
		- PhysX3CommonDEBUG_x86.dll
		- PhysX3Cooking_x86.dll
		- PhysX3CookingDEBUG_x86.dll
		- PhysX3DEBUG_x86.dll
		- PhysX3Gpu_x86.dll
		- PhysX3GpuDEBUG_x86.dll
		- PhysXDevice.dll

	/Libraries/PhysX/physx/bin/x64/
		- nvToolsExt32_1.dll
		- PhysX3_x64.dll
		- PhysX3Common_x64.dll
		- PhysX3CommonDEBUG_x64.dll
		- PhysX3Cooking_x64.dll
		- PhysX3CookingDEBUG_x64.dll
		- PhysX3DEBUG_x64.dll
		- PhysX3Gpu_x64.dll
		- PhysX3GpuDEBUG_x64.dll
		- PhysXDevice.dll

- Copy all 8 resulting PhysX LIB files into the folders respectively:
	- /Libraries/PhysX/physx/lib/win32/
		- PhysX3_x86.lib
		- PhysX3Common_x86.lib
		- PhysX3CommonDEBUG_x86.lib
		- PhysX3Cooking_x86.lib
		- PhysX3CookingDEBUG_x86.lib
		- PhysX3DEBUG_x86.lib
		- PhysX3Extensions.lib
		- PhysX3ExtensionsDEBUG.lib

	- /Libraries/PhysX/physx/lib/x64/
		- PhysX3_x64.lib
		- PhysX3Common_x64.lib
		- PhysX3CommonDEBUG_x64.lib
		- PhysX3Cooking_x64.lib
		- PhysX3CookingDEBUG_x64.lib
		- PhysX3DEBUG_x64.lib
		- PhysX3Extensions.lib
		- PhysX3ExtensionsDEBUG.lib

- Copy the include files for PhysX SDK in the folder:
	- /Libraries/PhysX/pxshared/include/ (~160 files)
	- /Libraries/PhysX/physx/include/ (~60 files)

- All other libraries are already included

- Build and run the FluidSandbox Solution

##Screenshots