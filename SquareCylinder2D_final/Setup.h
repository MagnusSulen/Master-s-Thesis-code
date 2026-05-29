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
//! \file Setup.h
//! \author Magnus Sulen, adapted from the FlowAroundSphere showcase
//!          in waLBerla by Markus Holzer <markus.holzer@fau.de>
//!          https://i10git.cs.fau.de/walberla/walberla
//!
//
//======================================================================================================================
#pragma once

#include "core/config/Config.h"
#include "core/DataTypes.h"
#include "core/math/Vector3.h"

#include <string>

namespace walberla
{
struct Setup
{

   Setup(const Config::BlockHandle & parameters, const Config::BlockHandle & domainParameters,
         const Config::BlockHandle & logging, const Config::BlockHandle & boundaries,
         std::map<std::string, std::string>& infoMap)
   {
      blockForestFilestem         = domainParameters.getParameter< std::string >("blockForestFilestem", "blockforest");
      numProcesses                = domainParameters.getParameter< uint_t >("numberProcesses");
      const Vector3< real_t > ds  = domainParameters.getParameter< Vector3< real_t > >("domainSize");
      const real_t coarseMeshSize = parameters.getParameter< real_t >("coarseMeshSize");
      cellsPerBlock               = domainParameters.getParameter< Vector3< uint_t > >("cellsPerBlock");

      rootBlocks[0] = uint_c(std::ceil( (ds[0] / coarseMeshSize) / real_c(cellsPerBlock[0])));
      rootBlocks[1] = uint_c(std::ceil( (ds[1] / coarseMeshSize) / real_c(cellsPerBlock[1])));
      rootBlocks[2] = uint_c(std::ceil( (ds[2] / coarseMeshSize) / real_c(cellsPerBlock[2])));

      cells      = Vector3<uint_t>(rootBlocks[0] * cellsPerBlock[0], rootBlocks[1] * cellsPerBlock[1], rootBlocks[2] * cellsPerBlock[2]);
      domainSize = Vector3<real_t>(cells) * coarseMeshSize;

      periodic = domainParameters.getParameter< Vector3< bool > >("periodic");
      refinementLevels = domainParameters.getParameter< uint_t >("refinementLevels");

      squareDiameter = parameters.getParameter< real_t >("diameterSquare") / coarseMeshSize;
      squareHalfSide   = squareDiameter / real_c(2.0);

      squareXPosition = parameters.getParameter< real_t >("SquareXPosition") / coarseMeshSize;
      squareYPosition = real_c(cells[1]) / real_c(2.0);
      squareZPosition = real_c(cells[2]) / real_c(2.0);

      reynoldsNumber    = parameters.getParameter< real_t >("reynoldsNumber");
      referenceVelocity = parameters.getParameter< real_t >("referenceVelocity");
      inflowVelocity    = parameters.getParameter< real_t >("latticeVelocity");

      const real_t speedOfSound = real_c(real_c(1.0) / std::sqrt( real_c(3.0) ));
      machNumber = inflowVelocity / speedOfSound;
      viscosity  = real_c((inflowVelocity * squareDiameter) / reynoldsNumber);
      omega      = real_c(real_c(1.0) / (real_c(3.0) * viscosity + real_c(0.5)));

      rho = real_c(1.0);
      dxC = coarseMeshSize;
      dxF = real_c(coarseMeshSize) / real_c( 1 << refinementLevels );
      dt = inflowVelocity / referenceVelocity * coarseMeshSize;

      resolutionSquare = parameters.getParameter< real_t >("diameterSquare") / dxF;

      timesteps = parameters.getParameter< uint_t >("timesteps");
      numGhostLayers = uint_c(2);
      nbrOfEvaluationPointsForCoefficientExtremas = 100;

      valuesPerCell = (Stencil_T::Q + VelocityField_T::F_SIZE + uint_c(2) * ScalarField_T::F_SIZE);
      memoryPerCell = memory_t(valuesPerCell * sizeof(PdfField_T::value_type) + uint_c(1));
      processMemoryLimit = parameters.getParameter< memory_t >( "processMemoryLimit", memory_t( 512 ) ) * memory_t( 1024 * 1024 );

      stencil          = infoMap["stencil"];
      streamingPattern = infoMap["streamingPattern"];
      collisionModel   = infoMap["collisionOperator"];

      fluidUID    = FlagUID("Fluid");
      obstacleUID = FlagUID(boundaries.getParameter< std::string >("square"));
      inflowUID   = FlagUID(boundaries.getParameter< std::string >("inflow"));
      outflowUID  = FlagUID(boundaries.getParameter< std::string >("outflow"));
      wallUID     = FlagUID(boundaries.getParameter< std::string >("walls"));

      writeSetupForestAndReturn = logging.getParameter< bool >("writeSetupForestAndReturn", false);

   }

   void writeParameterHeader(std::ofstream& file)
   {
      file << "ReynoldsNumber" << "," << "machNumber" << "," << "coarseMeshSize" << "," << "fineMeshSize" << "," << "resolutionSquare" << ",";
      file << "refinementLevels" << "," << "stencil" << "," << "streamingPattern" << "," << "collisionOperator" << "," << "omega-coarse" << "\n";

      file << reynoldsNumber << "," << machNumber << "," << dxC << "," << dxF << "," << resolutionSquare << ",";
      file << refinementLevels << "," << stencil << "," << streamingPattern << "," << collisionModel << "," << omega << "\n";
   }

   std::string blockForestFilestem;
   uint_t numProcesses;

   Vector3<uint_t> rootBlocks;
   Vector3<uint_t> cells;
   Vector3<real_t> domainSize;

   Vector3< bool > periodic;
   uint_t refinementLevels;

   Vector3< uint_t > cellsPerBlock;
   uint_t numGhostLayers;

   real_t squareXPosition;
   real_t squareYPosition;
   real_t squareZPosition;
   real_t squareHalfSide;
   real_t squareDiameter;
   real_t resolutionSquare;

   uint_t nbrOfEvaluationPointsForCoefficientExtremas;

   real_t machNumber;
   real_t reynoldsNumber;

   real_t viscosity;
   real_t omega;
   real_t rho;
   real_t inflowVelocity;
   real_t referenceVelocity;
   real_t dxC;
   real_t dxF;
   real_t dt;

   uint_t timesteps;

   uint_t valuesPerCell;
   memory_t memoryPerCell;
   memory_t processMemoryLimit;

   std::string stencil;
   std::string streamingPattern;
   std::string collisionModel;

   FlagUID fluidUID;
   FlagUID obstacleUID;
   FlagUID inflowUID;
   FlagUID outflowUID;
   FlagUID wallUID;

   bool writeSetupForestAndReturn;
};

}