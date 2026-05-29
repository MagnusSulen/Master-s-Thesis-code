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
//! \file Cube.h
//! \author Magnus Sulen, adapted from the FlowAroundSphere showcase
//!          in waLBerla by Markus Holzer <markus.holzer@fau.de>
//!          https://i10git.cs.fau.de/walberla/walberla
//!
//
//======================================================================================================================
#pragma once

#include "blockforest/SetupBlock.h"
#include "blockforest/SetupBlockForest.h"
#include "blockforest/StructuredBlockForest.h"

#include "domain_decomposition/IBlock.h"

#include "field/FlagUID.h"

#include "core/DataTypes.h"
#include "core/math/AABB.h"
#include "core/math/Vector3.h"
#include "core/cell/Cell.h"

#include "stencil/D3Q7.h"
#include "stencil/D3Q27.h"

#include "Types.h"
#include "Setup.h"

namespace walberla
{

class Cube
{
 public:
   Cube(const Setup& setup) : setup_(setup)
   {
      // Cube center same as sphere center
      const real_t px = setup_.sphereXPosition * setup_.dxC;
      const real_t py = setup_.sphereYPosition * setup_.dxC;
      const real_t pz = setup_.sphereZPosition * setup_.dxC;

      // Half-side length equals sphere radius (same bounding size)
      halfSide_ = setup_.sphereRadius * setup_.dxC;

      // Build AABB from center +/- halfSide
      cubeAABB_ = AABB(px - halfSide_, py - halfSide_, pz - halfSide_,
                       px + halfSide_, py + halfSide_, pz + halfSide_);
   }

   bool operator()(const Vector3< real_t >& point) const { return contains(point); }

   bool contains(const Vector3< real_t >& point) const;
   bool contains(const AABB& aabb) const;

   real_t delta(const Vector3< real_t >& fluid, const Vector3< real_t >& boundary) const;
   Setup getSetup(){ return setup_; }
   AABB getCubeAABB() const { return cubeAABB_; }

   void setupBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void checkConsistency(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void setupCubeBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void setBoundaryFromCellInterval(CellInterval& cells, const FlagField_T::flag_t flag, FlagField_T* flagField);

 private:
   Setup setup_;
   AABB cubeAABB_;
   real_t halfSide_;

}; // class Cube


class CubeRefinementSelection
{
 public:
   CubeRefinementSelection(const Cube& cube, const uint_t level)
      : cube_(cube), level_(level)
   {
      auto setup = cube_.getSetup();
      const real_t px = setup.sphereXPosition * setup.dxC;
      const real_t py = setup.sphereYPosition * setup.dxC;
      const real_t pz = setup.sphereZPosition * setup.dxC;
      const real_t halfSide = setup.sphereRadius * setup.dxC;
      const real_t bufferDistance = setup.dxF;
      const real_t d = halfSide + bufferDistance;

      // Refinement zone 1: tight around cube
      cubeBoundingBox1_ = AABB(px - d, py - d, pz - d,
                               px + d + real_c(2.5) * halfSide, py + d, pz + d);

      // Refinement zone 2: extended wake behind cube
      cubeBoundingBox2_ = AABB(px - d, py - d, pz - d,
                               px + d + real_c(5.0) * halfSide, py + d, pz + d);
   }

   void operator()(SetupBlockForest& forest)
   {
      if(level_ == 0) return;
      for(auto block = forest.begin(); block != forest.end(); ++block)
      {
         const AABB& aabb = block->getAABB();

         { AABB zone_L2(real_c(8),real_c(8),real_c(8),real_c(16),real_c(12),real_c(12)); if(block->getLevel()<uint_c(2)&&zone_L2.intersects(aabb)) block->setMarker(true); }

      }
   }
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   

 private:
   Cube cube_;
   uint_t level_;
   AABB cubeBoundingBox1_;
   AABB cubeBoundingBox2_;

}; // class CubeRefinementSelection


class CubeBlockExclusion
{
 public:
   CubeBlockExclusion(const Cube& cube) : cube_(cube) {}

   bool operator()(const blockforest::SetupBlock& block)
   {
      const AABB aabb = block.getAABB();
      return static_cast< bool >(cube_.contains(aabb));
   }

 private:
   Cube cube_;

}; // class CubeBlockExclusion


class wallDistanceCube
{
 public:
   wallDistanceCube(const Cube& cube) : cube_(cube) {}

   real_t operator()(const Cell& fluidCell, const Cell& boundaryCell,
                     const shared_ptr< StructuredBlockForest >& SbF, IBlock& block) const;

 private:
   Cube cube_;

}; // class wallDistanceCube

} // namespace walberla
