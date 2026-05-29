# FlowAroundCube3D

A waLBerla showcase for three-dimensional lattice Boltzmann simulation
of flow around a single cube, with block-structured adaptive mesh
refinement (AMR). The application is adapted from the
`FlowAroundSphere` showcase in upstream waLBerla, with the spherical
obstacle replaced by an axis-aligned cube. The lattice Boltzmann
kernel, streaming algorithm, collision operator, and code-generation
pipeline are unchanged from upstream waLBerla.

## Method summary

- Stencil: D3Q27
- Collision: Cumulant K17 with fourth-order correction
- Streaming pattern: Esoteric Pull
- Subgrid model: toggleable Smagorinsky (see below); K17 alone
  already provides implicit LES behaviour
- Code generation: `lbmpy` and `pystencils_walberla`
- GPU execution through CUDA
- Block-structured AMR

## Smagorinsky toggle

The top of `FlowAroundCube.py` exposes:

```python
USE_SMAGORINSKY = True
SMAGORINSKY_CS  = 0.12
```

Set `USE_SMAGORINSKY = False` for pure implicit LES through the K17
collision operator. This is suitable for the validation runs at
moderate Reynolds numbers (Re up to about 10^6 with a single cube).

Set `USE_SMAGORINSKY = True` when additional dissipation is needed for
stability at high Reynolds numbers or on complex voxelized geometries
where sharp edges generate strong local gradients. The Smagorinsky
constant `C_s = 0.12` is the value used by the default waLBerla
implementation.

Note that the generated CUDA kernel is different for the two settings.
After changing the flag the application must be rebuilt with
`ninja FlowAroundCube` so that the code generation pipeline runs again.

## Default boundary conditions

| Location                  | Condition                         |
|---------------------------|-----------------------------------|
| Inlet (`x_min`)           | UBB (uniform lattice velocity)    |
| Outlet (`x_max`)          | FixedDensity                      |
| Top/bottom/lateral walls  | FreeSlip                          |
| Cube surface              | QuadraticBounceBack (no-slip)     |

Forces on the cube are extracted directly through the Momentum
Exchange Method, integrated into the QuadraticBounceBack boundary
condition via the `calculate_force_on_boundary` option in `lbmpy`.

## Requirements

- waLBerla, built with code generation, GPU support, and lbmpy
  ([https://i10git.cs.fau.de/walberla/walberla](https://i10git.cs.fau.de/walberla/walberla))
- A CUDA-capable GPU
- Python 3 with `lbmpy`, `pystencils`, and `pystencils_walberla`
  (waLBerla pulls these in as build dependencies when CodeGen is enabled)

## Build instructions

1. Clone waLBerla and place this directory inside `apps/showcases/`:

```bash
   git clone https://i10git.cs.fau.de/walberla/walberla.git
   cd walberla
   git clone https://github.com/MagnusSulen/Master-s-Thesis-code.git tmp_thesis
   mv tmp_thesis/FlowAroundCube3D apps/showcases/FlowAroundCube
   rm -rf tmp_thesis
```

2. Register the showcase in `apps/showcases/CMakeLists.txt`. Add the
   following inside the existing `if (WALBERLA_BUILD_WITH_CODEGEN)`
   block:

```cmake
   add_subdirectory( FlowAroundCube )
```

3. Configure and build:

```bash
   mkdir build && cd build
   cmake .. -DWALBERLA_BUILD_WITH_CODEGEN=ON \
            -DWALBERLA_BUILD_WITH_CUDA=ON \
            -DCMAKE_BUILD_TYPE=Release
   ninja FlowAroundCube
```

## Running

```bash
cd apps/showcases/FlowAroundCube
./FlowAroundCube FlowAroundCube.prm
```

Output:

- `dragSphereRe_<Re>_meshLevels_<N>.csv` (legacy filename retained from
  the upstream showcase) contains the time series of drag and lift
  forces on the cube.
- `VTKCube_Re<Re>/` contains the VTK output of the velocity and
  density fields at the frequency configured in `VTKWriter`.

## Domain and grid layout

The default `.prm` defines a rectangular domain `40L x 20L x 20L`,
where `L` is the side length of the cube. The cube is positioned 12L
from the inlet, centred laterally and vertically, leaving 28L of wake
behind it. Five refinement levels are applied, with the finest mesh
covering a zone surrounding the cube and its near wake. The base
block has `cellsPerBlock = 64 x 64 x 64`. At the finest level the
resolution corresponds to 32 cells per cube side length.

The cube itself is defined analytically through `Cube.h` and
`Cube.cpp`, which provide:

- An axis-aligned bounding box (AABB) for the cube.
- The `delta` function used by the QuadraticBounceBack boundary
  condition to compute the fractional distance from a fluid cell to
  the cube surface.
- The block exclusion functor that prevents blocks lying entirely
  inside the cube from being created.
- The refinement-selection functor that marks the cube and its near
  wake for refinement.

## Replacing the cube with a complex bottom-fixed obstacle

The application is structured so that the obstacle representation is
isolated from the lattice Boltzmann kernel. To simulate flow around a
custom bottom-fixed geometry (a building, an offshore structure,
furniture, a vehicle on a flat plane) the following changes are
required.

### 1. Voxelize the geometry

Use the script in `voxelizer/voxelize_stl.py` (in the parent
repository) to convert a triangulated STL surface mesh into a binary
voxel grid:

```bash
python3 voxelizer/voxelize_stl.py model.stl model_voxels.bin \
    --spacing 0.5 --units m
```

The output binary file contains the grid dimensions, voxel size,
origin, and an obstacle/fluid flag per cell. See
`voxelizer/README.md` for the file format.

### 2. Replace Cube.cpp / Cube.h with a voxelgrid loader

Write a class that:

a. Reads the binary voxel file at startup.
b. Stores the obstacle flags in an array indexed by world-space cells.
c. Implements a `contains(point)` method that returns true if a given
   point lies inside the voxel grid and the corresponding cell is
   flagged as obstacle.
d. Implements a `delta(fluid, boundary)` method that returns 0.5 as a
   default fractional distance (suitable for half-way bounce-back).
   A higher-order distance estimate requires the original STL.

The existing `Cube` class header gives the required signature and is
the simplest starting point. The functions `setupBoundary`,
`checkConsistency`, and the refinement-selection class can be reused
with minor changes (the AABB used for refinement selection should be
expanded to cover the full voxel grid extent).

### 3. Cut the domain below the obstacle

For bottom-fixed structures (sea surface, ground, foundation slab) the
domain should typically not extend below the base of the obstacle.
Two options exist:

a. **Domain trimming.** Set `domainSize` in the `.prm` so that the
   y-range starts at the obstacle base. The full domain is then air,
   and no cells are wasted on representing the ground. This is the
   approach used in the upstream FlowAroundSphere showcase, where the
   sphere sits on a free-slip ground plane.

b. **Solid filling below the obstacle.** Voxelize so that all cells
   below the obstacle base are flagged as solid. The
   `BlockExclusion` functor then excludes those blocks at the
   block-forest level, leaving a step at the lower domain boundary.
   This costs one block of solid cells along the bottom but is the
   simplest way to represent a complex foundation with a flat base.

For a flat foundation, option (a) is preferred because it saves
memory. For an irregular foundation that interacts with the flow
(jacket legs, columns, pillars), option (b) is required.

### 4. Recommended boundary conditions for complex geometries

The boundary conditions used for a single cube are not optimal for a
complex voxelized obstacle. Two changes are recommended.

**Obstacle surface:** switch from `QuadraticBounceBack` to `NoSlip`.
`QuadraticBounceBack` requires knowing the position of the wall
between lattice nodes, which is well defined for an analytic cube but
not for an arbitrary voxel grid. `NoSlip` falls back to half-way
bounce-back at the cell face, which is first-order accurate at the
wall but robust on any flag-field representation. To switch, change
the obstacle BC entry in `.prm` from `Obstacle` (the QuadraticBounceBack
flag) to the corresponding NoSlip flag, and adjust the boundary
generator in `FlowAroundCube.py` accordingly.

A consequence of switching to NoSlip is that
`calculate_force_on_boundary` is not available, and force time series
on the obstacle are no longer produced. Integrated drag and lift
coefficients on the obstacle must then be replaced by wake-field
metrics (velocity deficit, turbulent kinetic energy, sampled velocity
probes).

**Outflow:** switch from `FixedDensity` to `ExtrapolationOutflow`.
`FixedDensity` clamps the pressure at the outlet to a fixed value, which
generates spurious reflections when the wake fluctuates strongly. An
extrapolation-type outflow lets the flow leave the domain without
imposing a pressure, which gives cleaner wake statistics for complex
obstacles. The boundary generator is named `Outflow` in
`FlowAroundCube.py` and exposes both choices through the
`ExtrapolationOutflow` class from `lbmpy.boundaries`.

**Subgrid model:** for complex obstacles set `USE_SMAGORINSKY = True`
in `FlowAroundCube.py`. Sharp edges in the voxelized representation
generate strong local strain rates that can destabilise the K17
operator on its own.

### 5. Refinement zone

The refinement-selection functor in `Cube.h` defines two AABBs
surrounding the cube. For a complex obstacle, replace the cube
half-side with a representative dimension (typically the obstacle
height or the largest horizontal extent) and re-centre the AABBs on
the obstacle bounding box.

## License

Source files derived from waLBerla are distributed under the same
GPL-3 license as the upstream project. See the file headers and the
`LICENSE` file at the root of the repository.

## Attribution

The application is adapted from the `FlowAroundSphere` showcase by
Markus Holzer at FAU. The lattice Boltzmann kernel, streaming
algorithm, collision operator, code-generation pipeline, and the
block-structured AMR framework are unchanged from upstream waLBerla.
Modifications are limited to:

- Replacing the spherical obstacle with an axis-aligned cube
  (`Cube.h`, `Cube.cpp`).
- Adding the `USE_SMAGORINSKY` toggle in `FlowAroundCube.py`.
- Minor renaming for clarity.

Upstream:
https://i10git.cs.fau.de/walberla/walberla/-/tree/master/apps/showcases/FlowAroundSphere
