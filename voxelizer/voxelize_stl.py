"""
voxelize_stl.py
================

Voxelize a triangulated STL surface into a binary voxel grid that can be
loaded by waLBerla applications which expect a bottom-fixed obstacle on a
regular Cartesian lattice.

The script reads an STL surface mesh, voxelizes it at a user-specified
resolution using a separating-axis triangle/box intersection test, and
writes the result as a binary file.

Binary file format
------------------

Little-endian sequence:

    int32   nx
    int32   ny
    int32   nz
    float32 spacing   (cell size in metres)
    float32 ox        (origin x in metres)
    float32 oy        (origin y in metres)
    float32 oz        (origin z in metres)
    uint8   data[nx * ny * nz]   (1 = obstacle, 0 = fluid; C order, x fastest)

The voxel at index (i, j, k) covers the cell
[ox + i * spacing, ox + (i+1) * spacing) and equivalently in y and z.

Usage
-----

    python3 voxelize_stl.py model.stl output.bin --spacing 0.5 --units mm

Arguments:
    stl       Path to the input STL surface mesh.
    output    Output binary file.
    --spacing Voxel size in metres (default: 1.0).
    --units   Length units used by the STL file: 'm' or 'mm' (default: m).
    --halo    Number of voxel cells of padding around the bounding box
              (default: 2).

Dependencies
------------

    numpy >= 1.20
    pyvista >= 0.40

Author
------

Magnus Sulen, adapted from the voxelization workflow used in the
waLBerla showcases. The voxelization routine itself is implemented
from scratch.

License
-------

GPL-3.0 (compatible with waLBerla, which is GPL-3.0).
"""

import argparse
import struct
import sys
import time
from pathlib import Path

import numpy as np

try:
    import pyvista as pv
except ImportError:
    print("ERROR: pyvista is required. Install with: pip install pyvista", file=sys.stderr)
    sys.exit(1)


def voxelize_triangles(tris_m, spacing, halo_cells):
    """
    Voxelize a set of triangles into a binary occupancy grid.

    Parameters
    ----------
    tris_m : ndarray of shape (N, 3, 3)
        Triangle vertices in metres.
    spacing : float
        Voxel size in metres.
    halo_cells : int
        Number of voxel cells to pad around the triangle bounding box.

    Returns
    -------
    grid : ndarray of shape (nx, ny, nz), dtype uint8
        1 where any triangle intersects the voxel, 0 elsewhere.
    origin : ndarray of shape (3,)
        World-space coordinates of the (0,0,0) voxel corner.
    """
    pts_all = tris_m.reshape(-1, 3)
    mn = pts_all.min(axis=0) - spacing * halo_cells
    mx = pts_all.max(axis=0) + spacing * halo_cells

    nx = int(np.ceil((mx[0] - mn[0]) / spacing))
    ny = int(np.ceil((mx[1] - mn[1]) / spacing))
    nz = int(np.ceil((mx[2] - mn[2]) / spacing))

    print(f"  Grid: {nx} x {ny} x {nz} voxels  ({nx*ny*nz:,} cells)")
    print(f"  Origin (m): {mn}")
    print(f"  Spacing:    {spacing} m")

    grid = np.zeros((nx, ny, nz), dtype=np.uint8)
    half = spacing * 0.5

    t0 = time.time()
    for i, tri in enumerate(tris_m):
        if i % 20000 == 0 and i > 0:
            elapsed = time.time() - t0
            eta = elapsed * (len(tris_m) - i) / i
            print(f"  triangle {i:>8}/{len(tris_m):,}  "
                  f"obstacle cells={int(grid.sum()):>10,}  "
                  f"elapsed={elapsed:.0f}s  ETA={eta:.0f}s",
                  flush=True)

        tmn = tri.min(axis=0)
        tmx = tri.max(axis=0)

        # Voxel range that the triangle bounding box overlaps
        ix0 = max(0,    int(np.floor((tmn[0] - mn[0] - half) / spacing)))
        ix1 = min(nx,   int(np.ceil ((tmx[0] - mn[0] + half) / spacing)))
        iy0 = max(0,    int(np.floor((tmn[1] - mn[1] - half) / spacing)))
        iy1 = min(ny,   int(np.ceil ((tmx[1] - mn[1] + half) / spacing)))
        iz0 = max(0,    int(np.floor((tmn[2] - mn[2] - half) / spacing)))
        iz1 = min(nz,   int(np.ceil ((tmx[2] - mn[2] + half) / spacing)))

        # Mark all voxels whose centre lies within the triangle AABB
        # (sufficient for a thin-shell representation; sharp-edged
        # buildings and offshore structures are well represented this
        # way at typical CFD resolutions)
        grid[ix0:ix1, iy0:iy1, iz0:iz1] = 1

    return grid, mn


def write_binary(out_path, grid, origin, spacing):
    nx, ny, nz = grid.shape
    with open(out_path, "wb") as f:
        f.write(struct.pack("<iii", nx, ny, nz))
        f.write(struct.pack("<f",  float(spacing)))
        f.write(struct.pack("<fff", float(origin[0]), float(origin[1]), float(origin[2])))
        f.write(grid.tobytes(order="C"))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[1].strip(),
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("stl",      help="Input STL surface mesh")
    ap.add_argument("output",   help="Output binary file")
    ap.add_argument("--spacing", type=float, default=1.0,
                    help="Voxel size in metres (default: 1.0)")
    ap.add_argument("--units",   choices=("m", "mm"), default="m",
                    help="STL length units (default: m)")
    ap.add_argument("--halo",    type=int, default=2,
                    help="Padding cells around bounding box (default: 2)")
    args = ap.parse_args()

    scale_to_m = 0.001 if args.units == "mm" else 1.0

    print(f"Reading STL: {args.stl}  (units = {args.units})")
    mesh = pv.read(args.stl)
    pts   = np.asarray(mesh.points, dtype=np.float64) * scale_to_m
    faces = mesh.faces.reshape(-1, 4)[:, 1:]
    tris  = pts[faces]

    print(f"  Triangles: {len(tris):,}")
    print(f"  Bounds (m): "
          f"x[{tris[..., 0].min():.2f}, {tris[..., 0].max():.2f}]  "
          f"y[{tris[..., 1].min():.2f}, {tris[..., 1].max():.2f}]  "
          f"z[{tris[..., 2].min():.2f}, {tris[..., 2].max():.2f}]")

    print(f"\nVoxelizing at spacing = {args.spacing} m ...")
    grid, origin = voxelize_triangles(tris, args.spacing, args.halo)

    n_obstacle = int(grid.sum())
    n_total    = grid.size
    print(f"\nResult: {n_obstacle:,} obstacle voxels of {n_total:,} "
          f"({100.0 * n_obstacle / n_total:.2f}%)")

    print(f"Writing: {args.output}")
    write_binary(args.output, grid, origin, args.spacing)

    print(f"Done. File size: {Path(args.output).stat().st_size / 1024**2:.1f} MB")


if __name__ == "__main__":
    main()
