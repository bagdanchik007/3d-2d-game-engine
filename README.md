# 3d-2d-game-engine

**3d-2d-game-engine** ist eine modulare 2D-/3D-Game-Engine auf Basis von **modernem C++20**. Das Projekt wurde mit dem Ziel entwickelt, eine klar strukturierte, erweiterbare und testbare Engine-Architektur von Grund auf aufzubauen.

Der Fokus liegt auf einer sauberen Trennung der Subsysteme, deterministischem Verhalten, testbarer Infrastruktur und einer klar definierten Schnittstelle zwischen Engine-Core, Rendering, Szenenverwaltung, Physik und Editor.

Die Engine verwendet **keine bestehende Game-Engine als Abstraktionsschicht**. Zentrale Systeme wie ECS, Scene Management, Rendering-Abstraktionen, Asset Management, Physik und Editor-Integration sind Bestandteil des Projekts und werden innerhalb der Engine implementiert.

---

## Technischer Überblick

| Bereich             | Technologie / Implementierung              |
| ------------------- | ------------------------------------------ |
| Sprache             | C++20                                      |
| Build System        | CMake ≥ 3.24                               |
| Rendering API       | OpenGL                                     |
| Windowing           | GLFW                                       |
| OpenGL Loader       | GLAD                                       |
| UI / Editor         | Dear ImGui                                 |
| Scene Serialization | YAML / yaml-cpp                            |
| Testing             | Catch2                                     |
| Logging             | spdlog                                     |
| Image Loading       | stb_image                                  |
| Mesh Loading        | tinyobjloader                              |
| ECS                 | Eigenimplementiertes Sparse-Set ECS        |
| Physik              | Eigenimplementierte Fixed-Step-AABB-Physik |
| Plattformen         | Linux / Windows                            |
| Compiler            | GCC / Clang / MSVC                         |

---

# Architektur

Die Engine ist in klar voneinander getrennte Subsysteme gegliedert. Abhängigkeiten verlaufen grundsätzlich von höherliegenden Systemen in Richtung der darunterliegenden Infrastruktur.

```text
                    ┌─────────────────────┐
                    │      Sandbox        │
                    │   Editor / Demo     │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │       Engine        │
                    └──────────┬──────────┘
                               │
       ┌───────────────┬───────┼────────┬───────────────┐
       ▼               ▼       ▼        ▼               ▼
     Core           ECS/Scene  Math   Renderer        Editor/UI
       │               │                │
       │               │                ├── OpenGL
       │               │                ├── Buffers
       │               │                ├── Shaders
       │               │                ├── Textures
       │               │                ├── Meshes
       │               │                └── Framebuffers
       │               │
       │               └── Serialization
       │
       ├── Application
       ├── Layer System
       ├── Events
       ├── Input
       ├── Logging
       └── Time

                    ┌─────────────────────┐
                    │      Physics        │
                    │ Fixed-Step AABB     │
                    └─────────────────────┘

                    ┌─────────────────────┐
                    │       Assets        │
                    │ Cache / Loading     │
                    └─────────────────────┘
```

Die einzelnen Subsysteme sind so ausgelegt, dass Implementierungsdetails möglichst hinter stabilen Interfaces verborgen bleiben. Dadurch können Backend-spezifische Implementierungen ausgetauscht oder erweitert werden, ohne höherliegende Systeme unnötig anzupassen.

---

# Kernsysteme

## Core

Der Core stellt die grundlegende Laufzeitinfrastruktur der Engine bereit.

Enthalten sind unter anderem:

* Application Lifecycle
* Layer-System
* Event-System
* Input-Abstraktion
* Logging
* Time Management
* UUID-basierte Identifikation
* zentrale Engine-Infrastruktur

Das Layer-System ermöglicht es, unterschiedliche Laufzeitbereiche unabhängig voneinander zu organisieren, beispielsweise Gameplay, Debugging oder Editor-Funktionalität.

---

## Entity Component System

Die Engine verfügt über ein eigenentwickeltes **Sparse-Set ECS**.

Das ECS trennt Entity-Identität, Component Storage und Zugriffsschichten voneinander.

### Eigenschaften

* Sparse-Set-basierter Component Storage
* generation-sichere Entity Handles
* typisierte Component Pools
* typisierte Views
* effiziente Iteration über Components
* Trennung von Entity Lifetime und Component Storage
* Schutz vor der Wiederverwendung ungültiger Entity Handles

Beispielhaft kann eine Entity dadurch über ihre Komponenten beschrieben werden:

```text
Entity
 ├── TransformComponent
 ├── MeshComponent
 ├── MaterialComponent
 └── RigidBodyComponent
```

Das ermöglicht eine datenorientierte Organisation der Laufzeitdaten, ohne Gameplay- oder Szenenlogik an eine monolithische Entity-Klasse zu koppeln.

---

# Scene System

Das Scene-System baut auf dem ECS auf und stellt die höherliegende Organisation von Entities und deren Beziehungen bereit.

Unterstützt werden:

* Entity-Erstellung und -Zerstörung
* Component Management
* Transform-Hierarchien
* Parent-/Child-Beziehungen
* zyklussicheres Parenting
* Scene Serialization
* Scene Deserialization
* hierarchische Szenenstrukturen

Eine typische Hierarchie kann beispielsweise folgendermaßen aussehen:

```text
Solar System
├── Sun
│
├── Earth
│   └── Moon
│
└── Mars
```

Parenting wird validiert, um ungültige zyklische Beziehungen innerhalb der Transform-Hierarchie zu verhindern.

---

# Scene Serialization

Szenen können über **YAML** serialisiert und wiederhergestellt werden.

Der Deserialisierungsprozess berücksichtigt dabei Entity-Referenzen und Parent-Beziehungen. Die Rekonstruktion erfolgt in mehreren Phasen, sodass Parent-Referenzen auch dann korrekt aufgelöst werden können, wenn die referenzierte Entity erst später erzeugt wird.

Beispiel:

```yaml
Scene: SolarSystem

Entities:
  - ID: 1
    Name: Sun

  - ID: 2
    Name: Earth
    Parent: 1

  - ID: 3
    Name: Moon
    Parent: 2
```

Dadurch bleibt die hierarchische Struktur beim Speichern und Laden der Szene erhalten.

---

# Math Library

Die Engine enthält eine eigene mathematische Infrastruktur für Rendering, Transforms und räumliche Berechnungen.

Unterstützt werden unter anderem:

* 2D-/3D-Vektoren
* Matrizen
* Quaternions
* Transformationsmatrizen
* orthografische Projektionen
* perspektivische Projektionen
* Axis-Angle-Rotation
* Rotationskonvertierungen

Quaternions werden insbesondere für rotationsbezogene Operationen verwendet, um typische Probleme von Euler-Winkeln wie Gimbal Lock zu vermeiden.

---

# Rendering

Das Rendering-System basiert auf einer Abstraktionsschicht zwischen Engine und Grafik-API.

```text
Renderer
   │
   ▼
Renderer API
   │
   ▼
OpenGL Backend
   │
   ├── Buffers
   ├── Vertex Arrays
   ├── Shaders
   ├── Textures
   ├── Framebuffers
   └── Render Commands
```

Die Engine abstrahiert zentrale Rendering-Ressourcen wie:

* Vertex Buffers
* Index Buffers
* Vertex Arrays
* Shaders
* Textures
* Framebuffers
* Render Commands

Das OpenGL-Backend implementiert diese Interfaces, während höherliegende Systeme möglichst unabhängig von konkreten OpenGL-Aufrufen bleiben.

---

# 2D Rendering

Das 2D-Rendering verwendet ein **batched rendering approach**, um mehrere Sprites bzw. Quads mit möglichst wenigen Draw Calls zu rendern.

Unterstützt werden:

* Texturen
* Texture Coordinates
* Farb-Tinting
* Orthographic Camera
* Sprite-/Quad-Rendering
* Batched Rendering

---

# 3D Rendering

Das 3D-Rendering erweitert die Renderer-Infrastruktur um:

* Perspective Camera
* Fly-/Camera Controller
* 3D Meshes
* prozedural erzeugte Geometrie
* OBJ Mesh Loading
* Framebuffers
* Depth Rendering
* Blinn-Phong Lighting

Prozedurale Meshes können beispielsweise direkt innerhalb der Engine erzeugt werden:

```text
Mesh
├── Cube
└── Plane
```

Zusätzlich können externe OBJ-Modelle über `tinyobjloader` geladen werden.

---

# Camera System

Die Engine unterstützt unterschiedliche Kameramodelle.

### Orthographic Camera

Wird hauptsächlich für 2D-Rendering und Editor-Anwendungen eingesetzt.

### Perspective Camera

Wird für 3D-Szenen verwendet und unterstützt unter anderem:

* Field of View
* Aspect Ratio
* Near / Far Plane
* Yaw
* Pitch
* Fly Camera Movement

Die View- und Projection-Matrizen werden dabei getrennt verwaltet und in die Rendering-Pipeline integriert.

---

# Asset System

Das Asset-System stellt eine zentrale Schnittstelle für das Laden und Wiederverwenden von Ressourcen bereit.

Aktuell werden unter anderem folgende Asset-Typen unterstützt:

* Texturen
* Shader
* Meshes

Ein zentraler Cache verhindert unnötiges mehrfaches Laden derselben Ressource und trennt Asset-Identifikation von der konkreten Ressource.

Die Architektur ist darauf ausgelegt, zukünftig weitere Asset-Typen zu integrieren.

---

# Physics

Die Engine enthält ein eigenentwickeltes **Fixed-Step-AABB-Physiksystem**.

Unterstützt werden:

* Rigid Bodies
* Gravitation
* AABB Collision Detection
* Collision Manifolds
* Restitution
* Friction
* Fixed-Time-Step Simulation

Die Simulation verwendet einen festen Zeitschritt, um die physikalische Integration von der variablen Render-Framerate zu entkoppeln.

Vereinfacht:

```text
Application Loop
       │
       ├── Input
       │
       ├── Fixed Physics Update
       │       │
       │       ├── Integrate
       │       ├── Detect Collisions
       │       ├── Build Manifolds
       │       └── Resolve Collisions
       │
       └── Render
```

Damit bleibt das Verhalten der Physiksimulation wesentlich stabiler, unabhängig davon, ob die Render-Framerate beispielsweise 60 FPS oder 144 FPS beträgt.

---

# Editor

Die `Sandbox` dient gleichzeitig als interaktive Demonstrationsumgebung und als Grundlage für die Editor-Integration.

Der Editor basiert auf **Dear ImGui**.

Aktuell umfasst er:

### Scene Hierarchy

* Anzeige der Entity-Hierarchie
* Auswahl von Entities
* Erstellung neuer Root-Entities
* Löschen von Entities
* Darstellung von Parent-/Child-Beziehungen

### Inspector

Der Inspector ermöglicht die direkte Bearbeitung ausgewählter Entities.

Unterstützt werden aktuell:

* Entity Name
* Position
* Axis-Angle Rotation
* Scale

### Viewport

Die Szene wird in einen Off-Screen-Framebuffer gerendert und anschließend innerhalb des ImGui-Viewports dargestellt.

Der Viewport unterstützt:

* dynamische Größenanpassung
* framebuffer-basierte Darstellung
* fokussierte Kameraeingabe
* Trennung zwischen Editor-UI und Scene Input

---

# Input Handling

Editor und Engine teilen sich dieselbe Input-Infrastruktur.

Dabei wird berücksichtigt, ob ImGui eine Eingabe bereits verarbeitet.

Vereinfacht:

```text
Keyboard / Mouse Input
          │
          ▼
       ImGui
          │
          ├── Input captured → Editor
          │
          └── Input available
                    │
                    ▼
             Engine Controls
```

Dadurch wird verhindert, dass beispielsweise eine Kamera gleichzeitig reagiert, während der Benutzer gerade ein Textfeld oder ein anderes ImGui-Element bedient.

---

# Testing

Die Engine verfügt über eine automatisierte Test-Suite auf Basis von **Catch2**.

Getestet werden unter anderem:

* Core-Systeme
* Logging
* Time
* Math
* ECS
* Entity Lifetime
* Scene Management
* Transform Hierarchien
* Serialization
* Asset Management
* Physics
* Renderer-Infrastruktur

Aktueller Stand:

```text
136 test cases
311 assertions
```

Die Tests sind als eigenständiges `EngineTests`-Target in das CMake-Projekt integriert.

Tests können mit folgendem Befehl ausgeführt werden:

```bash
./build/bin/EngineTests --reporter compact
```

---

# Build & Installation

## Voraussetzungen

* **CMake 3.24+**
* **C++20-kompatibler Compiler**

  * GCC
  * Clang
  * MSVC
* Git
* Internetverbindung beim ersten CMake-Configure
* OpenGL-fähige Grafiktreiber
* Desktop Window System

## Abhängigkeiten

CMake lädt folgende Dependencies automatisch:

* spdlog
* GLFW
* yaml-cpp
* Dear ImGui
* Catch2

Folgende Libraries sind bereits Bestandteil des Repositorys:

* GLAD
* stb_image
* tinyobjloader

---

# Build unter Linux

```bash
git clone https://github.com/bagdanchik007/GameEngine.git
cd GameEngine

cmake -S GameEngine -B build
cmake --build build --parallel
```

Anschließend kann die Sandbox gestartet werden:

```bash
./build/bin/Sandbox
```

---

# Build unter Windows

```powershell
git clone https://github.com/bagdanchik007/GameEngine.git
cd GameEngine

cmake -S GameEngine -B build
cmake --build build --config Release
```

Sandbox starten:

```powershell
.\build\bin\Release\Sandbox.exe
```

---

# Entwicklung

Nach Änderungen an den CMake-Konfigurationen kann das Projekt neu konfiguriert werden:

```bash
cmake -S GameEngine -B build
```

Komplettes Projekt bauen:

```bash
cmake --build build --parallel
```

Tests ausführen:

```bash
./build/bin/EngineTests --reporter compact
```

---

# Projektstruktur

```text
GameEngine/
├── GameEngine/
│   ├── Engine/
│   │   ├── include/Engine/
│   │   │   ├── Assets/          # Asset Handles und Asset Cache
│   │   │   ├── Core/            # Application, Layer, Events, Input, Logging
│   │   │   ├── ECS/             # Registry, Entities, Pools und Views
│   │   │   ├── Editor/          # Editor Panels
│   │   │   ├── Math/            # Vektoren, Matrizen, Quaternions
│   │   │   ├── Physics/         # Physik und Collision Detection
│   │   │   ├── Renderer/        # Renderer Abstractions
│   │   │   ├── Scene/           # Scene Management und Serialization
│   │   │   └── UI/              # Dear ImGui Integration
│   │   │
│   │   ├── src/                 # Engine Implementierungen
│   │   └── vendor/              # Drittanbieter-Libraries
│   │
│   ├── Sandbox/                 # Interaktive Demo / Editor
│   ├── Tests/                   # Automatisierte Tests
│   ├── cmake/                   # Gemeinsame CMake Utilities
│   └── CMakeLists.txt
│
├── README.md
└── GameEngine.sln
```

---

# Entwicklungsziele

Das Projekt wird inkrementell entwickelt. Der Schwerpunkt liegt dabei nicht auf einer möglichst großen Feature-Menge, sondern auf einer **sauberen technischen Basis**, auf der weitere Systeme aufgebaut werden können.

Aktuelle bzw. geplante Erweiterungen:

* [ ] Scene-Asset-Referenzen
* [ ] Save-/Load-Aktionen direkt im Editor
* [ ] Transform-Gizmos im Viewport
* [ ] Erweiterte Collision Shapes
* [ ] Spatial Broad Phase
* [ ] Erweiterte Rendering-Features
* [ ] Ausbau der Editor-Infrastruktur

---

# Designziele

Bei der Entwicklung der Engine stehen insbesondere folgende Prinzipien im Vordergrund:

### Modulare Architektur

Subsysteme sollen klar abgegrenzte Verantwortlichkeiten besitzen und möglichst geringe gegenseitige Abhängigkeiten aufweisen.

### Testbarkeit

Core-Systeme sollen unabhängig von der grafischen Anwendung getestet werden können.

### Erweiterbarkeit

Neue Renderer-Backends, Asset-Typen, Components oder Editor-Funktionen sollen integriert werden können, ohne bestehende Systeme unnötig umzubauen.

### Klare Abstraktionen

Öffentliche Interfaces sollen Implementierungsdetails kapseln und die Abhängigkeiten zwischen Subsystemen reduzieren.

### Modernes C++

Die Engine nutzt moderne C++20-Konzepte und legt Wert auf RAII, Value Semantics, Smart Pointer, Templates und typsichere Interfaces.

---

# Status

**Development Status: Active Development**

Die Engine befindet sich weiterhin in aktiver Entwicklung. APIs und interne Architekturen können sich daher verändern.

Der aktuelle Schwerpunkt liegt auf der Weiterentwicklung von **Scene Management, Editor Workflow, Asset Management und Physics** sowie auf der Verbesserung der bestehenden Testabdeckung.

---

# Lizenz

Dieses Projekt befindet sich derzeit in aktiver Entwicklung. Informationen zur Lizenz werden ergänzt, sobald eine finale Lizenzentscheidung getroffen wurde.
