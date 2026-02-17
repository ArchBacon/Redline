<div align="center">

# 🐝 Breda Educational Engine (BEE)

</div>

BEE is an ongoing project of giving CMGT Programming Year 2 students a solid foundation to start their projects with. It also aims to provide scaffolding in a way that projects can from different students can be integrated later on. As such, BEE is not a full featured game engine. Some aspects of BEE are either in flux or buggy (it is however better tested than most student projects).

# 📖 Docs

This documentation goes over the basics of using BEE.

## 🚀 Running BEE
You can build BEE as a static library using the `bee.vcxproj` Visual Studio project. All games/samples are separate projects that use this library. The library and games can be compiled and run on PC. Our samples should serve as a starting point into exploring how BEE works. We did not design the samples as critically as BEE itself; you'll find that the code in the samples is more pragmatic than the engine code. 

## 📁 Folder Structure
BEE aims to demonstrate good usage of folder structure for a medium-size project. The root folder should look like this:
- **include** - All public header files (`.hpp` extension). Do not add source files here.
- **source** - All source files (`.cpp` extension or shaders). Do not add headers here (unless they are truly *private*)
- **external** - All external dependencies. Some files here need to be *compiled*.
- **assets** - All general game assets that should be accessible to all samples.
- **properties** - Property sheets that are used to configure the project. Make use of these whenever you can.
- **script** - Python scripts to maintain the project.

The *include* and *source* folders have a similar structure, and they map to various features of the engine.
- **core** - Core BEE facilities, like file IO, the ECS and the resource system.
- **ai** - Navigation system.
- **graph** - General graph-based data structures and algorithms.
- **math** - Small set of math tools.
- **physics** - A custom 2D physics engine.
- **rendering** - Platform-independent parts of the rendering code.
- **platform** - Code specific to each platform.
    - **pc** - PC-specific code.
    - **opengl** - OpenGL code. Not strictly a platform, but can run on multiple platforms as needed.
- **tools** - Everything else. Logging, profiling, engine UI, thread pools, and so on.

All files and folder use `lower_case` naming, while the class/struct names are in `CamelCase`. Please follow BEE's convention where you can.

## 🦅 Overview of Features

This section outlines various important features of BEE and how you can use them. Please read this carefully before implementing a feature; it might help you avoid re-inventing the wheel.

### ⚙️ Core
The entry point in `Engine`. This object is an instance of the BEE engine. It is an access point to most facilities offered by BEE. It does not include any headers, so you need to include the headers you will be using. It runs the update loop.

### 🧩 ECS
BEE uses an EnTT as an ECS. Transforms, physics simulation, mesh rendering etc. are all done via simple structs attached to entities. The `ECS` class, accessible via `Engine.ECS()`, is a basic wrapper around the EnTT registry, which you can fully access via `Engine.ECS().Registry`. The `ECS` class also has convenience functions for safely creating and deleting entities and components; please use these whenever you can. 

Also, `Engine.ECS()` adds the concept of *systems* on top of EnTT. Each `System` should be in charge of some part of the game/engine logic, by accessing or manipulating certain types of components. It's up to you what you delegate to separate systems. Existing examples in BEE are the renderer and the 2D and 3D physics systems.

**Important:** Don't store references or pointers to components. Adding a new component to EnTT can *invalidate* all references to components of that type. This can lead to bugs that are very hard to resolve. Whenever you need references ot specific objects, use entity IDs only, and retrieve the components when you need them.

### 🫳 Transforms
BEE has a `Transform` component, with support for hierarchies and naming the entities. Make use of it, rather than making your own.

### 🧺 Resources
BEE has a decently documented resource system. It uses shared pointers for reference counting. You need to manually call `CleanUp()`. For more info check the comments in `resources.hpp`. You can access it via `Engine.Resources()`.

### 🕹️ Input
BEE has a cross-platform input API that supports controllers, keyboard, and mouse. It's well documented with comments. You can access it via `Engine.Input()`.

### 🎵 Audio
BEE uses the FMOD Studio library for cross-platform audio support. You can access it via `Engine.Audio()`. There is support for simple sound playback (using only FMOD Core), but for anything more advanced, we recommend using the full FMOD Studio support. This means you'll do most of the 'audio design' in a separate application (FMOD Studio), and expose channels and parameters to BEE. There is currently no support for  spatial audio that syncs with ECS transforms, but this should not be difficult for you to add.

### 💾 File IO
BEE has a file IO API that you should use to access data on disk. Prefer that over accessing files directly. The API is also well documented. You can access it via `Engine.FileIO()`.

### 📺 Device
You can access the device API via the Engine. This API is different per platform, but it gives you access to the window size on both platforms. You can access it via `Engine.Device()`.

### 🌈 Multi-platform support
BEE games can currently run on PC. Other platforms could be added in the future. This has impact on the how the code is structured, but we do not apply the same solution to the problem everywhere. Some classes are completely different per platform and only share the header file (for example `image.hpp`):

```
#pragma once
#if defined(BEE_PLATFORM_PC)
#include "platform/opengl/image_gl.hpp"
#endif
```

Other places, like the `DebugRenderer` are using the private implementation pattern. Yet other, like `FileIO` are simply give the definition on some methods  at different `cpp` files.

### 📷 Rendering & Models
BEE has a basic PBR renderer. While it lacks advanced features, it does load and render GLTF models. The model loading is a two step process, where the Model is loaded as a resource and instantiated into the scene manually as needed. It can also be instantiated as a child to an existing entity.

### ⚽ Physics (2D and 3D)
`bee::physics::World` is an ECS system for simple 2D physics. It supports disks, capsules, convex polygons, and rotation. It is meant as an educational example; it's not optimized with acceleration structures, so it does not scale well to huge numbers of objects. For more demanding applications, consider integrating a professional 2D physics engine like Box2D.

For 3D physics, BEE optionally makes use of the Jolt physics library. We have included the full source code of Jolt as a separate project that compiles into a static library. `bee::physics::JoltSystem` is an ECS system that wraps around Jolt. If you want to use that system (and therefore Jolt) in a game, you need to link to the Jolt static library yourself and add some definitions. Please see our Jolt sample projects to find out how.

### 🗺️ Navigation
`bee::AI::NavigationSystem` is an ECS system that can compute and use a 2D navigation mesh for pathfinding. Most navmesh functionality is hidden away; interaction goes mostly via the `NavmeshAgent` component. The system is dependent on 2D physics; it will only work if you also use the `bee::physics::World` system.

### 📜 Logging
BEE has a logging functionality. Prefer that over `printf` or `cout`.

### 🔎 Inspector
BEE has a inspector UI that can be exteded to act as an editor.

### 🧶 Serialization
BEE has a serialization API that makes use of the `visit_struct` library for run-time reflection. 

### 🧑‍⚕️ Static Code Analysis and Formatting
BEE uses clang tools for linting. 

Formatting is done through [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html). A `.clang-format` file is provided in the root of Bee. You can run formatting on all files by running `format.bat` in your command line.

Static Code Analysis is done through [ClangTidy](https://clang.llvm.org/extra/clang-tidy/). A `.clang-tidy` file is provided in the root of Bee, which defines the set of rules we check for. You can run ClangTidy per platform by running `python script\tidy_with_compile_commands.py --platform x64` for PC. This script will create folders named `tidy\[Platform]` based on the `--platform` provided as argument. In these new folders you can find all the suggestions from running ClangTidy (which are also printed in the command line). 

By default the checks target `bee.sln`, but you can also target another `.sln` or `vcxproj` file by providing it as argument like so: `python script\tidy_with_compile_commands.py --platform x64 --target bee.vcxproj`

> [!WARNING]  
> This requires the following programs to be installed and added to your `PATH`:
>   + Python 3+ https://www.python.org/downloads/ or https://www.anaconda.com/download/; *anaconda is recommended if you plan on developing with Python
>   + Install llvm clang 18.1.8 https://github.com/llvm/llvm-project/releases/tag/llvmorg-18.1.8, on windows `LLVM-18.1.8-win64.exe`
>   + MSBuild should already be installed via Visual Studio but it has to be added to your PATH, e.g. `C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\`

`ClangTidy` and `ClangFormat` are run as part of the CI on Pull Request.

## 🏄‍♀️ GitHub workflow

The following file explains our workflow for Git and branches:
[GitHub Workflow Guide](docs/github.md). Once you work in a team, we recommend you to start with this workflow and adapt it when needed. 