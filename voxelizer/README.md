# voxelizer

A standalone Python tool that converts a triangulated STL surface mesh
into a binary voxel grid suitable for use as an obstacle field in
waLBerla applications. The tool is independent of the waLBerla source
tree and can be used with any application that consumes the binary
voxel format described below.

## Overview

The script reads an STL file, projects every triangle onto a regular
Cartesian grid at a user-specified resolution, and writes a binary
file containing the obstacle flags. Each cell is marked as obstacle if
any triangle bounding box intersects the cell extent. This produces a
thin-shell representation of the surface, which is sufficient for the
sharp-edged structures encountered in CFD around buildings, vehicles,
and offshore structures at typical resolutions.

The output file format is self-describing (dimensions, origin, and
spacing are written in the header) so the binary can be loaded
without any out-of-band configuration.

## Requirements

- Python 3.9 or later
- numpy >= 1.20
- pyvista >= 0.40

Install with:

    pip install numpy pyvista

## Usage

    python3 voxelize_stl.py model.stl model_voxels.bin --spacing 0.5 --units m --halo 2

### Arguments

| Argument     | Description                                                  |
|--------------|--------------------------------------------------------------|
| stl          | Path to the input STL surface mesh                           |
| output       | Path to the output binary file                               |
| --spacing    | Voxel size in metres (default: 1.0)                          |
| --units      | Length units of the STL file: m or mm (default: m)           |
| --halo       | Voxel cells of padding around the bounding box (default: 2)  |

If the STL was exported in millimetres (a common default in CAD
tools), pass `--units mm` so that the voxel positions are written in
metres.

## Output file format

The output is a little-endian binary file with the following layout:

    int32   nx
    int32   ny
    int32   nz
    float32 spacing       (cell size in metres)
    float32 origin_x      (in metres)
    float32 origin_y      (in metres)
    float32 origin_z      (in metres)
    uint8   data[nx*ny*nz]   (1 = obstacle, 0 = fluid; C order, x fastest)

The voxel at index (i, j, k) covers the world-space cell

    [origin_x + i*spacing, origin_x + (i+1)*spacing)

and equivalently in y and z. The origin refers to the lower corner of
voxel (0, 0, 0).

## Loading the file in C++

A minimal loader fragment for a waLBerla application:

    std::ifstream in("model_voxels.bin", std::ios::binary);
    int32_t nx, ny, nz;
    float spacing, ox, oy, oz;
    in.read(reinterpret_cast<char*>(&nx), 4);
    in.read(reinterpret_cast<char*>(&ny), 4);
    in.read(reinterpret_cast<char*>(&nz), 4);
    in.read(reinterpret_cast<char*>(&spacing), 4);
    in.read(reinterpret_cast<char*>(&ox), 4);
    in.read(reinterpret_cast<char*>(&oy), 4);
    in.read(reinterpret_cast<char*>(&oz), 4);

    std::vector<uint8_t> data(static_cast<size_t>(nx) * ny * nz);
    in.read(reinterpret_cast<char*>(data.data()), data.size());

    auto cellAt = [&](int i, int j, int k) {
        return data[i + nx * (j + ny * k)];
    };

## Choosing a resolution

The voxel size should be small enough that the obstacle is well
represented and that the smallest geometric feature of interest spans
at least two or three voxels. For a 30-metre tall building with
1-metre features the appropriate spacing is around 0.5 metres. For a
small mechanical part with sub-millimetre features the spacing has to
be reduced accordingly.

The voxel grid for the obstacle does not need to match the lattice
spacing in the CFD application. The loader in the waLBerla
application can compare cell centres against the voxel grid in
world-space coordinates, so an obstacle voxelized at one resolution
can be used with any compatible lattice spacing in waLBerla. In
practice the obstacle voxel size should be similar to the finest
lattice spacing in the refinement zone surrounding the obstacle.

## Limitations

- Hollow interiors are not preserved. The voxelization marks all
  voxels intersected by any triangle, which gives a thin-shell
  representation. For very thick walls this is usually fine; for
  hollow structures where the interior would be flooded by the flow,
  a separate ray-casting pass is required to mark interior voxels as
  obstacle (this is not implemented here).
- Sub-cell precision is not preserved. The fractional distance from a
  fluid cell to the wall is unknown, so half-way bounce-back is the
  appropriate boundary condition. QuadraticBounceBack requires either
  an analytic geometry description or a separate signed-distance
  field, neither of which is produced by this tool.

## License

GPL-3.0, compatible with waLBerla.

## Author

Magnus Sulen.
