add_library(FEMLib)

target_sources(FEMLib PRIVATE src/linalg/fem.cpp
    src/linalg/systemSolve.cpp
    src/linalg/cholesky.cpp
    src/Matrix/CSRMatrix.cpp
    src/Matrix/FEMatrix.cpp
    src/Matrix/COOMatrix.cpp
    src/Matrix/diagMatrix.cpp
    src/Matrix/SKRMatrix.cpp
    src/Mesh/Mesh.cpp
    src/utils/FEMdata.cpp
    src/utils/NavierStokesSolver.cpp
    src/utils/MultiGrid.cpp
    src/linalg/cholesky.cpp)

target_include_directories(FEMLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/linalg>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Matrix>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Mesh>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/utils>
    $<INSTALL_INTERFACE:include/linalg>
    $<INSTALL_INTERFACE:include/Matrix>
    $<INSTALL_INTERFACE:include/Mesh>
    $<INSTALL_INTERFACE:include/utils>
)

find_package(OpenMP REQUIRED)
target_compile_options(FEMLib PRIVATE -ffast-math -fopenmp -O3)

