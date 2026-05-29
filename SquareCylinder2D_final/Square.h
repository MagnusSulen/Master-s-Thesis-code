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
//! \file Square.h
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

class Square
{
 public:
   Square(const Setup& setup) : setup_(setup)
   {
      // Square center same as square center
      const real_t px = setup_.squareXPosition * setup_.dxC;
      const real_t py = setup_.squareYPosition * setup_.dxC;
      const real_t pz = setup_.squareZPosition * setup_.dxC;

      // Half-side length equals square half-side (same bounding size)
      halfSide_ = setup_.squareHalfSide * setup_.dxC;

      // Build AABB from center +/- halfSide
      squareAABB_ = AABB(px - halfSide_, py - halfSide_, pz - halfSide_,
                       px + halfSide_, py + halfSide_, pz + halfSide_);
   }

   bool operator()(const Vector3< real_t >& point) const { return contains(point); }

   bool contains(const Vector3< real_t >& point) const;
   bool contains(const AABB& aabb) const;

   real_t delta(const Vector3< real_t >& fluid, const Vector3< real_t >& boundary) const;
   Setup getSetup(){ return setup_; }
   AABB getSquareAABB() const { return squareAABB_; }

   void setupBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void checkConsistency(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void setupSquareBoundary(const std::shared_ptr< StructuredBlockForest >& sbfs, const BlockDataID flagFieldID);
   void setBoundaryFromCellInterval(CellInterval& cells, const FlagField_T::flag_t flag, FlagField_T* flagField);

 private:
   Setup setup_;
   AABB squareAABB_;
   real_t halfSide_;

}; // class Square


class SquareRefinementSelection
{
 public:
   SquareRefinementSelection(const Square& cube, const uint_t level)
      : square_(cube), level_(level)
   {
      auto setup = square_.getSetup();
      const real_t px = setup.squareXPosition * setup.dxC;
      const real_t py = setup.squareYPosition * setup.dxC;
      const real_t pz = setup.squareZPosition * setup.dxC;
      const real_t halfSide = setup.squareHalfSide * setup.dxC;
      const real_t bufferDistance = setup.dxF;
      const real_t d = halfSide + bufferDistance;

      // Refinement zone 1: tight around square
      squareBoundingBox1_ = AABB(px - d, py - d, pz - d,
                               px + d + real_c(2.5) * halfSide, py + d, pz + d);

      // Refinement zone 2: extended wake behind square
      squareBoundingBox2_ = AABB(px - d, py - d, pz - d,
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
   Square square_;
   uint_t level_;
   AABB squareBoundingBox1_;
   AABB squareBoundingBox2_;

}; // class SquareRefinementSelection


class SquareBlockExclusion
{
 public:
   SquareBlockExclusion(const Square& cube) : square_(cube) {}

   bool operator()(const blockforest::SetupBlock& block)
   {
      const AABB aabb = block.getAABB();
      return static_cast< bool >(square_.contains(aabb));
   }

 private:
   Square square_;

}; // class SquareBlockExclusion


class wallDistanceSquare
{
 public:
   wallDistanceSquare(const Square& cube) : square_(cube) {}

   real_t operator()(const Cell& fluidCell, const Cell& boundaryCell,
                     const shared_ptr< StructuredBlockForest >& SbF, IBlock& block) const;

 private:
   Square square_;

}; // class wallDistanceSquare

} // namespace walberla
