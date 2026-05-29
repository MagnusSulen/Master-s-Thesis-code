waLBerla_link_files_to_builddir( *.py )
waLBerla_link_files_to_builddir( *.prm )
waLBerla_link_files_to_builddir( *.png )
waLBerla_link_files_to_builddir( *.obj )
waLBerla_link_files_to_builddir( *.stl )
waLBerla_link_files_to_builddir( *.mtl )

waLBerla_generate_target_from_python(NAME FlowAroundCubeCodeGen
        FILE FlowAroundCube.py
        OUT_FILES FlowAroundCubeSweepCollection.h FlowAroundCubeSweepCollection.${CODEGEN_FILE_SUFFIX}
        FlowAroundCubeStorageSpecification.h FlowAroundCubeStorageSpecification.${CODEGEN_FILE_SUFFIX}
        FreeSlip.h FreeSlip.${CODEGEN_FILE_SUFFIX}
        NoSlip.h NoSlip.${CODEGEN_FILE_SUFFIX}
        Obstacle.h Obstacle.${CODEGEN_FILE_SUFFIX}
        UBB.h UBB.${CODEGEN_FILE_SUFFIX}
        Outflow.h Outflow.${CODEGEN_FILE_SUFFIX}
        FixedDensity.h FixedDensity.${CODEGEN_FILE_SUFFIX}
        FlowAroundCubeBoundaryCollection.h
        FlowAroundCubeInfoHeader.h
        FlowAroundCubeStaticDefines.h)

if (WALBERLA_BUILD_WITH_CUDA OR WALBERLA_BUILD_WITH_HIP)
    add_executable ( FlowAroundCube FlowAroundCube.cpp Cube.cpp Evaluation.cpp GridGeneration.h Types.h )
    target_link_libraries ( FlowAroundCube walberla::blockforest walberla::boundary walberla::core walberla::field
            walberla::gpu walberla::lbm_generated walberla::stencil walberla::timeloop walberla::vtk
            FlowAroundCubeCodeGen )
else()
    add_executable ( FlowAroundCube FlowAroundCube.cpp Cube.cpp Evaluation.cpp GridGeneration.h Types.h )
    target_link_libraries ( FlowAroundCube walberla::blockforest walberla::boundary walberla::core walberla::field
            walberla::lbm_generated walberla::stencil walberla::timeloop walberla::vtk FlowAroundCubeCodeGen )

endif(WALBERLA_BUILD_WITH_CUDA OR WALBERLA_BUILD_WITH_HIP)
