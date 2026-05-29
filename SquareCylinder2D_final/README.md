# SquareCylinder2D_final

A waLBerla showcase for two-dimensional lattice Boltzmann simulation of
flow around a square cylinder. The application is adapted from the
`FlowAroundSphere` showcase in waLBerla, with the spherical obstacle
replaced by a square cylinder and the configuration reduced to a
quasi-two-dimensional setup (one cell in the spanwise direction with
periodic boundaries).

## Method summary

- Stencil: D3Q27 (quasi-2D, `Nz = 1`, periodic spanwise)
- Collision: Cumulant K17 with fourth-order correction
- Streaming pattern: Esoteric Pull
- Subgrid model: none (implicit LES through K17)
- Code generation: `lbmpy` and `pystencils_walberla`
- GPU execution through CUDA

## Boundary conditions

| Location              | Condition                         |
|-----------------------|-----------------------------------|
| Inlet (`x_min`)       | UBB (uniform lattice velocity)    |
| Outlet (`x_max`)      | Extrapolation outflow             |
| Top/bottom (`y_min/max`) | FreeSlip                       |
| Spanwise (`z_min/max`)| Periodic                          |
| Square surface        | QuadraticBounceBack (no-slip)     |

The shipped configuration uses a single block holding the entire mesh
(`768 x 512 x 1` cells, 16 cells per side length, domain `48L x 32L`).
Forces on the square are extracted by the Momentum Exchange Method
through the `calculate_force_on_boundary` option in `lbmpy`.

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
   mv tmp_thesis/SquareCylinder2D_final apps/showcases/
   rm -rf tmp_thesis
```

2. Register the showcase in `apps/showcases/CMakeLists.txt` by adding
   the following line inside the existing
   `if (WALBERLA_BUILD_WITH_CODEGEN)` block:

```cmake
   add_subdirectory( SquareCylinder2D_final )
```

3. Configure and build:

```bash
   mkdir build && cd build
   cmake .. -DWALBERLA_BUILD_WITH_CODEGEN=ON \
            -DWALBERLA_BUILD_WITH_CUDA=ON \
            -DCMAKE_BUILD_TYPE=Release
   ninja SquareCylinder2DFinal
```

   Refer to the waLBerla documentation for required compiler versions
   and CUDA toolkit setup.

## Running

```bash
cd apps/showcases/SquareCylinder2D_final
./SquareCylinder2DFinal SquareCylinder2DFinal.prm
```

Output:

- `dragSquareRe_<Re>_meshLevels_1.csv` — time series of forces and
  force coefficients on the square
- `VTKSquareRE_<Re>/` — VTK output of the velocity and density fields

## Modifying the case

The default `.prm` runs at Re = 1000 with `latticeVelocity = 0.05`
(Mach number approximately 0.087). To run at a different Reynolds
number, change `reynoldsNumber` in `SquareCylinder2DFinal.prm`. To
change the mesh resolution, modify `coarseMeshSize` and
`cellsPerBlock` consistently so that the domain extents in
`domainSize` are preserved.

## License

Source files derived from waLBerla are distributed under the same
GPL-3 license as the upstream project. See the file headers and the
upstream `COPYING.txt` for details.

## Attribution

The application is adapted from the `FlowAroundSphere` showcase by
Markus Holzer at FAU. The lattice Boltzmann kernel, streaming
algorithm, collision operator, and code-generation pipeline are
unchanged from upstream waLBerla. Modifications are limited to the
obstacle geometry (square cylinder), the parameter file
(quasi-2D configuration, smaller domain), and minor renaming for
clarity.

Upstream:
https://i10git.cs.fau.de/walberla/walberla/-/tree/master/apps/showcases/FlowAroundSphere
