//======================================================================================================================
//
//  This file is part of waLBerla. waLBerla is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  waLBerla is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with waLBerla (see COPYING.txt). If not, see <http://www.gnu.org/licenses/>.
//
//! \file Square.cpp
//! \author Magnus Sulen, adapted from the FlowAroundSphere showcase
//!          in waLBerla by Markus Holzer <markus.holzer@fau.de>
//!          https://i10git.cs.fau.de/walberla/walberla
//!
//
//======================================================================================================================

#include "Square.h"

namespace walberla
{

// A point is inside the square if it is inside the AABB
bool Square::contains(const Vector3< real_t >& point) const
{
   return squareAABB_.contains(point);
}

// An AABB is fully inside the square if ALL 8 corners are inside
bool Square::contains(const AABB& aabb) const
{
   Vector3< real_t > p[8];
   p[0].set(aabb.xMin(), aabb.yMin(), aabb.zMin());
   p[1].set(aabb.xMax(), aabb.yMin(), aabb.zMin());
   p[2].set(aabb.xMin(), aabb.yMax(), aabb.zMin());
   p[3].set(aabb.xMax(), aabb.yMax(), aabb.zMin());
   p[4].set(aabb.xMin(), aabb.yMin(), aabb.zMax());
   p[5].set(aabb.xMax(), aabb.yMin(), aabb.zMax());
   p[6].set(aabb.xMin(), aabb.yMax(), aabb.zMax());
   p[7].set(aabb.xMax(), aabb.yMax(), aabb.zMax());
   return contains(p[0]) && contains(p[1]) && contains(p[2]) && contains(p[3]) &&
          contains(p[4]) && contains(p[5]) && contains(p[6]) && contains(p[7]);
}

// delta: fractional distance from fluid cell to cube surface along the line fluid->boundary
// Uses parametric ray-AABB intersection: find t in [0,1] where ray crosses square face
real_t Square::delta(const Vector3< real_t >& fluid, const Vector3< real_t >& boundary) const
{
   WALBERLA_ASSERT(!contains(fluid))
   WALBERLA_ASSERT(contains(boundary))

   // Ray: P(t) = fluid + t * (boundary - fluid), find smallest t in (0,1] where ray enters square
   const Vector3< real_t > dir = boundary - fluid;

   real_t tMin = real_c(0.0);
   real_t tMax = real_c(1.0);

   for (uint_t i = 0; i < 3; ++i)
   {
      if (std::abs(dir[i]) < real_c(1e-12))
      {
         // Ray parallel to slab — fluid must be inside slab, no intersection on this axis
         continue;
      }

      const real_t invD = real_c(1.0) / dir[i];
      real_t t0 = (squareAABB_.min(i) - fluid[i]) * invD;
      real_t t1 = (squareAABB_.max(i) - fluid[i]) * invD;

      if (t0 > t1) std::swap(t0, t1);

      tMin = std::max(tMin, t0);
      tMax = std::min(tMax, t1);
   }

   // tMin is the entry point of the ray into the square
   WALBERLA_CHECK_GREATER_EQUAL(tMin, real_c(0.0))
   WALBERLA_CHECK_LESS_EQUAL(tMin, real_c(1.0))

   return tMin;
}

void Square::setupBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID)
{
   for (auto bIt = sbfs->begin(); bIt != sbfs->end(); ++bIt)
   {
      auto flagField       = bIt->getData< FlagField_T >(flagFieldID);
      const FlagField_T::flag_t inflowFlag  = flagField->registerFlag(setup_.inflowUID);
      const FlagField_T::flag_t outflowFlag = flagField->registerFlag(setup_.outflowUID);
      const FlagField_T::flag_t wallFlag    = flagField->registerFlag(setup_.wallUID);

      const cell_idx_t gls = cell_idx_c(flagField->nrOfGhostLayers()) - cell_idx_c(1);

      CellInterval blockBB(-1, -1, -1,
                           cell_idx_c(setup_.cellsPerBlock[0]),
                           cell_idx_c(setup_.cellsPerBlock[1]),
                           cell_idx_c(setup_.cellsPerBlock[2]));

      // inflow WEST
      if(sbfs->atDomainXMinBorder(*bIt)){
         CellInterval west(blockBB.xMin() - gls, blockBB.yMin() - gls, blockBB.zMin() - gls,
                           blockBB.xMin(), blockBB.yMax() + gls, blockBB.zMax() + gls);
         setBoundaryFromCellInterval(west, inflowFlag, flagField);
      }

      // outflow EAST
      if(sbfs->atDomainXMaxBorder(*bIt)){
         CellInterval east(blockBB.xMax(), blockBB.yMin() - gls, blockBB.zMin() - gls,
                           blockBB.xMax() + gls, blockBB.yMax() + gls, blockBB.zMax() + gls);
         setBoundaryFromCellInterval(east, outflowFlag, flagField);
      }

      // SOUTH
      if(sbfs->atDomainYMinBorder(*bIt)){
         CellInterval south(blockBB.xMin() - gls, blockBB.yMin() - gls, blockBB.zMin() - gls,
                            blockBB.xMax() + gls, blockBB.yMin(), blockBB.zMax() + gls);
         setBoundaryFromCellInterval(south, wallFlag, flagField);
      }

      // NORTH
      if(sbfs->atDomainYMaxBorder(*bIt)){
         CellInterval north(blockBB.xMin() - gls, blockBB.yMax(), blockBB.zMin() - gls,
                            blockBB.xMax() + gls, blockBB.yMax() + gls, blockBB.zMax() + gls);
         setBoundaryFromCellInterval(north, wallFlag, flagField);
      }

      // BOTTOM
      if(sbfs->atDomainZMinBorder(*bIt)){
         CellInterval bottom(blockBB.xMin() - gls, blockBB.yMin() - gls, blockBB.zMin() - gls,
                             blockBB.xMax() + gls, blockBB.yMax() + gls, blockBB.zMin());
         setBoundaryFromCellInterval(bottom, wallFlag, flagField);
      }

      // TOP
      if(sbfs->atDomainZMaxBorder(*bIt)){
         CellInterval top(blockBB.xMin() - gls, blockBB.yMin() - gls, blockBB.zMax(),
                          blockBB.xMax() + gls, blockBB.yMax() + gls, blockBB.zMax() + gls);
         setBoundaryFromCellInterval(top, wallFlag, flagField);
      }
   }

   checkConsistency(sbfs, flagFieldID);
   setupSquareBoundary(sbfs, flagFieldID);
}

void Square::setBoundaryFromCellInterval(CellInterval& cells, const FlagField_T::flag_t flag, FlagField_T* flagField)
{
   for (auto cell = cells.begin(); cell != cells.end(); ++cell){
      if(flagField->get(cell->x(), cell->y(), cell->z()) == FlagField_T::flag_t(0))
         flagField->addFlag(cell->x(), cell->y(), cell->z(), flag);
   }
}

void Square::checkConsistency(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID)
{
   const uint_t depth = sbfs->getDepth();
   for (auto bIt = sbfs->begin(); bIt != sbfs->end(); ++bIt)
   {
      Block& b       = dynamic_cast< Block& >(*bIt);
      auto flagField = b.getData< FlagField_T >(flagFieldID);
      if (sbfs->getLevel(b) < depth){
         for( auto it = flagField->beginWithGhostLayer(1); it != flagField->end(); ++it ){
            Vector3< real_t > cellCenter = sbfs->getBlockLocalCellCenter(b, it.cell());
            sbfs->mapToPeriodicDomain(cellCenter);
            WALBERLA_CHECK(!contains(cellCenter), "The square must be completely located on the finest level")
         }
      }
   }
}

void Square::setupSquareBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID)
{
   const uint_t depth = sbfs->getDepth();
   for (auto bIt = sbfs->begin(); bIt != sbfs->end(); ++bIt)
   {
      Block& b       = dynamic_cast< Block& >(*bIt);
      auto flagField = b.getData< FlagField_T >(flagFieldID);
      uint8_t obstacleFlag = flagField->registerFlag(setup_.obstacleUID);

      if (sbfs->getLevel(b) == depth){
         for( auto it = flagField->beginWithGhostLayer(1); it != flagField->end(); ++it ){
            Vector3< real_t > cellCenter = sbfs->getBlockLocalCellCenter(b, it.cell());
            sbfs->mapToPeriodicDomain(cellCenter);
            if (contains(cellCenter)) { flagField->addFlag(it.x(), it.y(), it.z(), obstacleFlag); }
         }
      }
   }
}

real_t wallDistanceSquare::operator()(const Cell& fluidCell, const Cell& boundaryCell,
                                    const shared_ptr< StructuredBlockForest >& SbF, IBlock& block) const
{
   Vector3< real_t > boundary = SbF->getBlockLocalCellCenter(block, boundaryCell);
   Vector3< real_t > fluid    = SbF->getBlockLocalCellCenter(block, fluidCell);
   SbF->mapToPeriodicDomain(boundary);
   SbF->mapToPeriodicDomain(fluid);

   WALBERLA_CHECK(!square_.contains(fluid),
      "fluid cell is contained in square (" << fluid[0] << ", " << fluid[1] << ", " << fluid[2] << ")")
   WALBERLA_CHECK(square_.contains(boundary),
      "boundary cell is not contained in square (" << boundary[0] << ", " << boundary[1] << ", " << boundary[2] << ")")

   return square_.delta(fluid, boundary);
}

} // namespace walberla
