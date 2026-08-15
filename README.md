# GameEngine

A modular 2D/3D game engine written in modern C++20. The project is built
incrementally around clear subsystem boundaries, a testable core, and a small
interactive editor sandbox.

It is an engine project—not a wrapper around an existing game engine. Rendering,
ECS storage, scene handling, physics, asset loading, and the editor integration
are implemented in the repository.

## Highlights

- C++20, CMake, OpenGL, GLFW, and GLAD
- Layer-based application lifecycle with events, input, logging, time, and UUIDs
- Sparse-set ECS with generation-safe entity handles and typed views
- Scenes with transform hierarchies, cycle-safe parenting, YAML serialization,
  and deserialization
- Math library with vectors, matrices, quaternions, projections, and axis-angle
  conversion
- Renderer abstraction with an OpenGL backend, shaders, textures, framebuffers,
  2D batching, cameras, procedural meshes, and OBJ loading
- Asset cache for textures, shaders, and meshes
- Fixed-step AABB physics with gravity, restitution, friction, and collision
  manifolds
- Dear ImGui editor integration with a hierarchy, inspector, and framebuffer
  viewport
- Automated unit tests for core systems, math, ECS, scenes, assets, physics, and
  rendering infrastructure

## Editor sandbox

The `Sandbox` executable includes an interactive editor demo. It renders a
sample scene into an off-screen framebuffer and displays it inside an ImGui
viewport.

- **Scene Hierarchy** — view roots and children, select entities, create roots,
  and delete entities
- **Inspector** — edit the selected entity's name, position, axis-angle
  rotation, and scale
- **Viewport** — resize-aware framebuffer display with focused camera input
- **Input handling** — ImGui captures keyboard and mouse input before editor
  controls below it receive it

## Quick start

### Requirements

- CMake 3.24 or newer
- A C++20 compiler (GCC, Clang, or MSVC)
- Git and an internet connection for CMake to fetch dependencies on the first
  configure
- OpenGL-capable graphics drivers and a desktop window system to run `Sandbox`

### Build

```bash
git clone https://github.com/bagdanchik007/GameEngine.git
cd GameEngine

cmake -S GameEngine -B build
cmake --build build --parallel
```

CMake fetches the required libraries automatically: spdlog, GLFW, yaml-cpp,
Dear ImGui, and Catch2. GLAD, stb_image, and tinyobjloader are included in the
source tree.

### Run the sandbox

```bash
./build/bin/Sandbox
```

On Windows, use:

```powershell
.\build\bin\Sandbox.exe
```

### Run tests

```bash
./build/bin/EngineTests --reporter compact
```

The current suite contains **136 test cases and 311 assertions**.

## Project layout

```text
GameEngine/
├── GameEngine/
│   ├── Engine/
│   │   ├── include/Engine/       # Public engine API
│   │   │   ├── Assets/           # Asset handles and cache
│   │   │   ├── Core/             # Application, layers, input, logging
│   │   │   ├── ECS/              # Registry, entities, component pools, views
│   │   │   ├── Editor/           # Hierarchy, inspector, viewport panels
│   │   │   ├── Math/             # Vectors, matrices, quaternions
│   │   │   ├── Physics/          # AABB and rigid-body simulation
│   │   │   ├── Renderer/         # Renderer-facing abstractions
│   │   │   ├── Scene/            # Entities, transforms, YAML scenes
│   │   │   └── UI/               # Dear ImGui layer
│   │   ├── src/                  # Engine implementations
│   │   └── vendor/               # GLAD, stb_image, tinyobjloader, ImGui CMake target
│   ├── Sandbox/                  # Runnable demonstrations and editor sandbox
│   ├── Tests/                    # Catch2 test suite
│   ├── cmake/                    # Shared CMake helpers
│   └── CMakeLists.txt
├── README.md
└── GameEngine.sln                # Visual Studio solution
```

## Architecture

```text
Sandbox / Tests
       │
       ▼
Engine
 ├── Core ───── application loop, layers, events, input
 ├── ECS / Scene ─ entities, components, hierarchy, serialization
 ├── Math ───── vectors, matrices, quaternions
 ├── Renderer ─ OpenGL, cameras, meshes, shaders, framebuffers
 ├── Assets ─── cached texture, shader, and mesh loading
 ├── Physics ── fixed-step rigid-body simulation
 └── Editor/UI ─ ImGui lifecycle, hierarchy, inspector, viewport
```

## Development commands

```bash
# Reconfigure after CMake changes
cmake -S GameEngine -B build

# Build the complete project
cmake --build build --parallel

# Run all tests in one process
./build/bin/EngineTests --reporter compact
```

## Next steps

- Scene asset references and save/load actions in the editor
- Gizmos for transform editing in the viewport
- Broader collision shapes and a spatial broad phase
- Renderable scene components and material abstraction
- Continuous integration for configure, build, and test verification
