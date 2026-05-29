# Master-s-Thesis-code
# Master's Thesis Code — Lattice Boltzmann CFD Around a Bottom-Fixed Structure

This repository contains the GPU lattice Boltzmann (LBM) code developed
for a master's thesis on wind flow simulation around offshore
structures, with the long-term goal of providing a real-time turbulent
wind field for crane operation safety. The work is part of a larger
digital twin project for an offshore platform, in collaboration with
industry and research partners.

## What is included

The repository contains the two waLBerla showcases used for numerical
validation of the framework, together with a standalone voxelization
tool that converts STL geometry into a binary obstacle grid.

| Folder                                                             | Contents                                                                                                  |
|--------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------|
| [`SquareCylinder2D_final/`](./SquareCylinder2D_final)              | Two-dimensional showcase for flow around a square cylinder, used for benchmarking against the literature  |
| [`FlowAroundCube3D/`](./FlowAroundCube3D)                          | Three-dimensional showcase for flow around an axis-aligned cube, used for the 3D validation               |
| [`voxelizer/`](./voxelizer)                                        | Standalone Python tool that converts an STL surface mesh into a binary voxel grid                         |

Each folder has its own `README.md` with build instructions, parameter
descriptions, and references to the upstream waLBerla code.

## What is NOT included

The simulation of the full bottom-fixed structure that motivates this
work is intentionally omitted. The geometry of that structure is
proprietary and cannot be released publicly. What is released here is
the validated numerical framework, which can be applied to any
bottom-fixed obstacle that the user has access to.

The [`FlowAroundCube3D`](./FlowAroundCube3D) showcase documents how to
reproduce the methodology used in the thesis. Its README describes:

- How to replace the analytic cube with a voxelized obstacle.
- Which boundary conditions to switch (`NoSlip` instead of
  `QuadraticBounceBack` on the obstacle surface, `ExtrapolationOutflow`
  instead of `FixedDensity` at the outlet).
- How to cut the domain below the obstacle for bottom-fixed
  structures.
- When to enable the Smagorinsky subgrid-scale model for additional
  stability on complex geometries.

Together with the [`voxelizer`](./voxelizer) tool, these instructions
are sufficient to reproduce the methodology of the thesis on any
arbitrary bottom-fixed geometry given as an STL surface.

## What the code is used for

- **Validation.** The 2D and 3D showcases are used to validate the
  lattice Boltzmann framework against experimental and numerical
  reference data from the literature (Strouhal number, drag
  coefficient, wake profiles). The validated configuration is then
  carried over to the proprietary geometry.
- **Methodology demonstration.** The code documents the numerical
  choices made in the thesis (stencil, collision operator, streaming
  pattern, boundary conditions, refinement strategy) in a form that
  is reproducible by others.
- **A starting point for related studies.** Researchers and students
  who want to apply waLBerla to flow around a bottom-fixed structure
  can start from the 3D showcase, replace the cube with their own
  voxelized obstacle, and follow the recommendations in the README.

## How to use this repository

1. Install waLBerla following the upstream instructions:
   [https://i10git.cs.fau.de/walberla/walberla](https://i10git.cs.fau.de/walberla/walberla)
2. Clone this repository.
3. Copy the showcase you want to build into `apps/showcases/` of your
   waLBerla source tree.
4. Follow the per-folder README for build and run instructions.

## License

All code in this repository is licensed under the GNU General Public
License v3.0 (GPL-3.0), to remain compatible with the upstream
waLBerla project from which the showcases are derived. See the
`LICENSE` file inside each folder.

## Attribution

The lattice Boltzmann kernel, streaming algorithm, collision operator,
code generation pipeline, and block-structured adaptive mesh
refinement framework are unchanged from upstream waLBerla:

- waLBerla — [https://i10git.cs.fau.de/walberla/walberla](https://i10git.cs.fau.de/walberla/walberla)
- The showcases in this repository are adapted from the
  `FlowAroundSphere` showcase by Markus Holzer at the Friedrich-Alexander
  University Erlangen-Nürnberg (FAU).

Modifications by the thesis author are limited to the obstacle
geometry, parameter files, and recommendations for application to
bottom-fixed structures.

## AI assistance declaration

Generative AI was used as an aid during the preparation of this
repository, specifically for cleaning up and restructuring code,
and finding documentation. The model used was Anthropic Claude Opus 4.7.

No proprietary information, no part of the confidential
geometry, no internal industry communication, and no thesis text was
shared with the model. The role of the AI was limited to:

- Renaming variables and cleaning up code that had accumulated
  iteration artefacts during development.
- Reseaching and finding the documentation and the repository.
- Refining code and sorting data 

All output produced with AI assistance was reviewed by the thesis author,
compiled, executed, and verified against reference
data before inclusion.

## Author

Magnus Sulen.
