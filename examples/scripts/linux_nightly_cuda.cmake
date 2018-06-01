set(GRIT_TEST_ROOT_DIRECTORY "$ENV{HOME}/grit/workspace/nightly-cuda")
set(CTEST_SOURCE_DIRECTORY "${GRIT_TEST_ROOT_DIRECTORY}/grit/src")
set(CTEST_BINARY_DIRECTORY "${GRIT_TEST_ROOT_DIRECTORY}/build")

SITE_NAME(HOSTNAME)
set(CTEST_SITE ${HOSTNAME})
set(CTEST_BUILD_NAME "linux-cuda")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_BUILD_FLAGS "-j4")
SET(ENV{CUDA_VISIBLE_DEVICES} "0")

include(/usr/share/modules/init/cmake)
module(use /usr/local/modules)
module(load cuda/9.0)

find_program(CTEST_GIT_COMMAND NAMES git)
set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")
set(CTEST_CONFIGURE_COMMAND "${CMAKE_COMMAND} -DGRIT_USE_CUDA:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Debug -DKOKKOS_DIR=/opt/sw/Kokkos/BuildCuda9.0_e01945d -DCMAKE_CXX_FLAGS=-expt-extended-lambda -DSILO_DIR=/opt/sw/silo/4.10.2 ${CTEST_SOURCE_DIRECTORY}")

ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
ctest_start("Nightly")
ctest_update()
ctest_configure()
ctest_build()
ctest_test()
ctest_submit()
