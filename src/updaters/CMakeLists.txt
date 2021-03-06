if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_definitions(-DDEBUGFLAGUP)
endif(${CMAKE_BUILD_TYPE} MATCHES "Debug")

include_directories(${CMAKE_SOURCE_DIR}/inc ${CUDA_INCLUDE_DIRS} ../simulation)
add_library(updater
    baseUpdater.cpp
    velocityVerlet.cpp
    vectorialVicsek.cpp
    sphericalVectorialVicsek.cpp
    sphericalSelfPropelledParticle.cpp
    noseHooverNVT.cpp
    energyMinimizerFIRE.cpp
    energyMinimizerAdam.cpp
    brownianDynamics.cpp
    )
cuda_add_library(updaterGPU
    velocityVerlet.cu
    vectorialVicsek.cu
    energyMinimizerAdam.cu
    energyMinimizerFIRE.cu
    noseHooverNVT.cu
    brownianDynamics.cu
    )
