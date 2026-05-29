waLBerla_link_files_to_builddir( *.py )
waLBerla_link_files_to_builddir( *.prm )
waLBerla_link_files_to_builddir( *.png )
waLBerla_link_files_to_builddir( *.obj )
waLBerla_link_files_to_builddir( *.stl )
waLBerla_link_files_to_builddir( *.mtl )

waLBerla_generate_target_from_python(NAME SquareCylinder2DFinalCodeGen
        FILE SquareCylinder2DFinal.py
        OUT_FILES SquareCylinder2DFinalSweepCollection.h SquareCylinder2DFinalSweepCollection.${CODEGEN_FILE_SUFFIX}
        SquareCylinder2DFinalStorageSpecification.h SquareCylinder2DFinalStorageSpecification.${CODEGEN_FILE_SUFFIX}
        FreeSlip.h FreeSlip.${CODEGEN_FILE_SUFFIX}
        NoSlip.h NoSlip.${CODEGEN_FILE_SUFFIX}
        Obstacle.h Obstacle.${CODEGEN_FILE_SUFFIX}
        UBB.h UBB.${CODEGEN_FILE_SUFFIX}
        Outflow.h Outflow.${CODEGEN_FILE_SUFFIX}
        FixedDensity.h FixedDensity.${CODEGEN_FILE_SUFFIX}
        SquareCylinder2DFinalBoundaryCollection.h
        SquareCylinder2DFinalInfoHeader.h
        SquareCylinder2DFinalStaticDefines.h)

if (WALBERLA_BUILD_WITH_CUDA OR WALBERLA_BUILD_WITH_HIP)
    add_executable ( SquareCylinder2DFinal SquareCylinder2DFinal.cpp Square.cpp Evaluation.cpp GridGeneration.h Types.h )
    target_link_libraries ( SquareCylinder2DFinal walberla::blockforest walberla::boundary walberla::core walberla::field
            walberla::gpu walberla::lbm_generated walberla::stencil walberla::timeloop walberla::vtk
            SquareCylinder2DFinalCodeGen )
else()
    add_executable ( SquareCylinder2DFinal SquareCylinder2DFinal.cpp Square.cpp Evaluation.cpp GridGeneration.h Types.h )
    target_link_libraries ( SquareCylinder2DFinal walberla::blockforest walberla::boundary walberla::core walberla::field
            walberla::lbm_generated walberla::stencil walberla::timeloop walberla::vtk SquareCylinder2DFinalCodeGen )

endif(WALBERLA_BUILD_WITH_CUDA OR WALBERLA_BUILD_WITH_HIP)
