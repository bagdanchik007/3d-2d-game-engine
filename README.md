# GameEngine

A modular 2D/3D game engine written in modern C++20, built from scratch
with a focus on clean architecture, explicit abstractions, testability,
and incremental development.

The project is developed as a long-term engine-building project rather
than as a collection of isolated rendering experiments. Each subsystem
is introduced through small, focused milestones and backed by tests where
practical.

---

## Features

### Core

- Application lifecycle
- Layer-based application architecture
- Event system
- Input abstraction
- Time / Timestep system
- Logging
- Assertions
- UUID generation

### ECS / Scene

- Entity and component storage
- Sparse-set based component pools
- Entity lifecycle management
- Component views and iteration
- `TransformComponent`
- Scene-oriented architecture
- Transform hierarchy
- Scene serialization *(Milestone 11)*

### Mathematics

- `Vec2`, `Vec3`, `Vec4`
- `Mat4`
- Quaternion support
- Matrix transformations
- Translation / rotation / scaling

### Rendering

- Renderer API abstraction
- OpenGL backend
- GLAD
- Vertex buffers
- Index buffers
- Vertex arrays
- Shaders
- Textures
- Framebuffers
- Render commands
- Orthographic cameras
- Perspective cameras
- Batched 2D rendering
- Procedural mesh generation
- OBJ mesh loading
- Basic 3D lighting

### Assets

- Asset manager
- Asset handles
- UUID-based asset identification
- Shader loading
- Texture loading
- OBJ mesh loading
- `tinyobjloader`
- `stb_image`

### Physics

- `PhysicsWorld`
- Rigid bodies
- Static and dynamic bodies
- AABB collision detection
- Collision manifolds
- Gravity
- Friction
- Restitution
- Fixed timestep simulation
- Accumulator and catch-up limit
- Physics unit tests
- Interactive physics sandbox demonstration

### Testing

The engine contains an automated test suite covering:

- Core systems
- Mathematics
- ECS
- Scene components
- Assets
- Physics
- Rendering-related infrastructure

Current test status:

> **112 test cases / 249 assertions — all passing**

---

## Architecture

The engine is organized around explicit subsystem boundaries.

```text
GameEngine/
├── Engine/
│   ├── include/
│   │   └── Engine/
│   │       ├── Assets/
│   │       ├── Core/
│   │       ├── ECS/
│   │       ├── Events/
│   │       ├── Math/
│   │       ├── Physics/
│   │       ├── Renderer/
│   │       └── ...
│   │
│   ├── src/
│   │   ├── Assets/
│   │   ├── Core/
│   │   ├── ECS/
│   │   ├── Math/
│   │   ├── Physics/
│   │   ├── Renderer/
│   │   └── Platform/
│   │
│   └── vendor/
│
├── Sandbox/
│   ├── assets/
│   └── src/
│
├── Tests/
│   └── src/
│
├── CMakeLists.txt
└── README.md