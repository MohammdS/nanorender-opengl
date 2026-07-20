# NanoRender OpenGL

NanoRender OpenGL is a renderer developed incrementally to move a complete
graphics pipeline from CPU-side pixel processing to OpenGL and GLSL. Each
milestone keeps the application buildable, adds one focused GPU capability,
and records its validation and visual evidence here.

## Current Status

### HW2 Task 2 - Normalization and Viewport Fitting

**Goal.** Normalize arbitrary OBJ coordinate ranges with one uniform scale and
center the mesh inside the current framebuffer without changing its
proportions.

**Implementation.** `calculate_mesh_bounds` visits every vertex to find the
minimum and maximum coordinates. `calculate_viewport_fit` derives the center,
size, and largest model dimension, then maps that largest dimension to 40% of
the framebuffer's smaller dimension:

```text
uniform scale = 0.4 * min(viewport width, viewport height)
                / largest model dimension
translation = viewport center - mesh center * uniform scale
fitted vertex = vertex * uniform scale + translation
```

The same scale is applied on all three axes, so the mesh cannot be stretched.
The fit is recalculated whenever the GLFW framebuffer size changes, and the
current pixels-per-model-unit scale is displayed in the window title. This
CPU-side bounds calculation prepares compact transform data that the vertex
shader will consume in the later GPU transformation milestone.

**Validation.** The hidden validation checks the tetrahedron's known bounds,
center, size, scale, and translation. It also transforms every vertex and
verifies that all fitted X/Y coordinates remain inside the framebuffer:

```powershell
.\build\Release\nanorender_opengl.exe --validate hw2-task2
```

Verified Release output at 1280 x 720:

```text
OpenGL 4.1 initialized.
Loaded OBJ task1_tetrahedron.obj: vertices=4 faces=4
HW2 Task 2 viewport fit: bounds_min=(-1.0,-1.0,-1.0) bounds_max=(1.0,1.0,1.0) center=(0.0,0.0,0.0) size=(2.0,2.0,2.0) scale=144.0 translation=(640.0,360.0,0.0) fitted=yes
HW2 Task 2 validation passed.
```

**Limitations.** This task verifies the transformation mathematically but does
not draw the model yet. Task 3 will upload indexed geometry and apply the fit
in the OpenGL wireframe pipeline.

### HW2 Task 1 - Loading and Inspecting 3D Data

**Goal.** Load a small OBJ mesh, store its vertices and triangular faces, and
make the loaded counts visible so they can be checked against the source file.

**Implementation.** `src/mesh.cpp` parses OBJ `v` and `f` records into GLM
positions and zero-based triangle indices. Face tokens with slash-separated
attributes and negative indices are accepted, and polygon faces are
triangulated with a fan. The validation model at
`models/task1_tetrahedron.obj` contains four vertices and four faces. CMake
copies the model directory beside the executable after each build.

In normal mode, the GLFW window title displays the model filename, vertex
count, and face count. The same information is printed to the console. OBJ
parsing intentionally remains CPU-side because asset loading prepares data for
later GPU vertex and index buffers; rasterization will move to OpenGL in Task
3.

**Validation.** The hidden validation checks the expected counts and verifies
that every stored face index references an existing vertex:

```powershell
.\build\Release\nanorender_opengl.exe --validate hw2-task1
```

Verified Release output:

```text
OpenGL 4.1 initialized.
Loaded OBJ task1_tetrahedron.obj: vertices=4 faces=4
HW2 Task 1 OBJ loader: vertices=4 faces=4 valid_indices=yes
HW2 Task 1 validation passed.
```

**Limitations.** This milestone loads positions and triangle topology only.
Texture coordinates, imported normals, GPU buffers, and mesh rendering belong
to later roadmap tasks.

### OpenGL Foundation

The project currently creates an OpenGL 4.1 Core context with GLFW, loads the
required OpenGL functions through GLAD, and maintains a resizable render loop.
The framebuffer is cleared to a fixed dark color, and pressing `Escape` closes
the application.

GLFW, GLM, and MicroUI are pinned through CMake. GLAD is generated for only the
OpenGL 4.1 Core API and stored under `third_party/glad`, so normal builds do not
require Python or a generator package.

No geometry is rendered yet. Task 2 adds mesh normalization and viewport
fitting before the indexed wireframe renderer is introduced in Task 3.

### Validation

The hidden foundation validation creates a real OpenGL context, clears one
frame, reads the center pixel back, and fails if the framebuffer color or
OpenGL state is incorrect:

```powershell
.\build\Release\nanorender_opengl.exe --validate foundation
```

Verified Release output:

```text
OpenGL 4.1 initialized.
Foundation framebuffer: 1280x720, center rgb(14, 19, 28)
Foundation validation passed.
```

## Build and Run

Requirements:

- CMake 3.20 or newer
- A C++20 compiler
- Git
- A graphics driver supporting OpenGL 4.1 or newer

Configure and build:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Run on Windows:

```powershell
.\build\Release\nanorender_opengl.exe
```

## Roadmap

This roadmap follows the engine-related task order in the original HW2-HW5
project. A task is checked only after its implementation, validation,
documentation, focused commit, and push are complete.

### Foundation

- [x] Establish the OpenGL 4.1 Core foundation with GLFW and GLAD

### HW2 - Wireframe Viewer and Geometric Transformations

- [x] Task 0: Integrate GLM (included in the OpenGL foundation)
- [x] Task 1: Load OBJ data and display mesh information
- [x] Task 2: Calculate mesh bounds, normalization, and viewport fitting
- [ ] Task 3: Render an indexed orthographic wireframe mesh
- [ ] Task 4: Add separate local and world transformation controls
- [ ] Task 5: Apply model transformations in the vertex shader
- [ ] Task 6: Add direct keyboard or mouse transformation controls

### HW3 - Virtual Cameras and Projections

- [ ] Task 1: Render GPU debug axes and object bounding boxes
- [ ] Task 2: Add a virtual camera and view matrix
- [ ] Task 3: Add orthographic and perspective projection modes
- [ ] Task 4: Calculate, upload, transform, and visualize mesh normals

### HW4 - Triangle Rasterization and Depth Buffering

- [ ] Task 1: Add a GPU triangle bounding-box debug view
- [ ] Task 2: Add hardware triangle filling and visualize barycentric interpolation
- [ ] Task 3: Add GPU depth testing and depth-buffer visualization

### HW5 - Lighting, Materials, and Shading

- [ ] Task 1: Add light/material properties and ambient lighting
- [ ] Task 2: Add flat diffuse shading
- [ ] Task 3: Add specular lighting and reflection-vector debugging
- [ ] Task 4: Add per-fragment Phong shading

### Final Comparison

- [ ] Document the final CPU-to-GPU architecture, behavior, and performance comparison

Pair-programming extensions are outside this roadmap because they were not part
of the completed task reports used as the reference for this port.

## Project Structure

```text
nanorender-opengl/
|-- assets/         Direct visual evidence
|-- models/         Mesh files
|-- shaders/        GLSL shader programs
|-- src/            Renderer and application source
|-- third_party/    Generated GLAD loader
|-- CMakeLists.txt  Build and dependency configuration
`-- README.md       Documentation and progress log
```

## Development Workflow

Work is divided into small feature milestones. Every milestone must compile,
pass its validation mode, update this README, and receive a focused commit and
push before the next feature begins. Visual milestones include a direct
screenshot or short GIF under `assets/`, whichever demonstrates the result
more clearly; animated evidence is kept below 3 MB.
