#########################################################
# IQ-TREE cmake build definition
#########################################################

cmake_minimum_required(VERSION 2.8)

project(pda)
# The version number.
set (iqtree_VERSION_MAJOR 1)
set (iqtree_VERSION_MINOR 0)
set (iqtree_VERSION_PATCH 3)

set(BUILD_SHARED_LIBS OFF)

if (NOT CMAKE_BUILD_TYPE) 
	set(CMAKE_BUILD_TYPE "Release")
endif()

# Detect platforms, tested for Windows, Mac OS X, and Unix-like
if (WIN32)
	message("Target OS     : Windows")
	# build as static binary to run on most machines
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
elseif (APPLE) 
	message("Target OS     : Mac OS X")
	# to be compatible back to Mac OS X 10.6
	add_definitions("-mmacosx-version-min=10.6") 
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmacosx-version-min=10.6")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
elseif (UNIX) 
	message("Target OS     : Unix")
	# build as static binary to run on most machines
	if (CMAKE_BUILD_TYPE STREQUAL "Release") 
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
	endif()
else()
	# Note that IQ-TREE has NOT been tested on other platforms
	message("Target OS     : Unknown")
endif()

if (CMAKE_COMPILER_IS_GNUCXX) 
	message("Compiler      : GNU Compiler (gcc)")
	#if (CMAKE_BUILD_TYPE STREQUAL "Release") 
	#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")  ## Strip binary
	#endif()
	#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
else()
	message("Compiler      : Unknown")
endif()

# verbose for 32 or 64 bit platform
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message("Target binary : possibly 32-bit")
	#SET(EXE_SUFFIX "-32bit")
elseif(CMAKE_CXX_FLAGS MATCHES "m32")
	message("Target binary : 32-bit")
	#SET(EXE_SUFFIX "-32bit")
else()
	message("Target binary : 64-bit")
	#SET(EXE_SUFFIX "")
endif()

# change the executable name if compiled for OpenMP parallel version
if (CMAKE_CXX_FLAGS MATCHES "openmp")
	message("Parallel      : OpenMP")
	SET(EXE_SUFFIX "-omp")
else()
	message("Parallel      : None")
	SET(EXE_SUFFIX "")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release") 
	message("Builde mode   : Release + Strip")
endif()

# check existence of a few basic functions
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
check_function_exists (gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists (getrusage HAVE_GETRUSAGE)
check_function_exists (GlobalMemoryStatusEx HAVE_GLOBALMEMORYSTATUSEX)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/iqtree_config.h.in"
  "${PROJECT_BINARY_DIR}/iqtree_config.h"
  )

# add the binary tree to the search path for include files
# so that we will find iqtree_config.h
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/zlib-1.2.7")


# add_definitions(-DIQ_TREE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3 -std=c++98 -Wall -Wno-unused-function -Wno-sign-compare -Wno-deprecated -pedantic -D_GNU_SOURCE -D__SIM_SSE3 -D_OPTIMIZED_FUNCTIONS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3 -std=c99 -Wall -Wno-unused-function -Wno-sign-compare -pedantic -D_GNU_SOURCE -D__SIM_SSE3 -D_OPTIMIZED_FUNCTIONS")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -g0")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -g0")

set(CMAKE_CXX_FLAGS_PROFILE "-fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline -O2 -DNDEBUG -fno-omit-frame-pointer -pg")
set(CMAKE_C_FLAGS_PROFILE "-fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -O2 -DNDEBUG -fno-omit-frame-pointer -pg")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

# subdirectories containing necessary libraries for the build 
add_subdirectory(phylolib)
add_subdirectory(ncl)
add_subdirectory(whtest)
add_subdirectory(sprng)
add_subdirectory(zlib-1.2.7)

# the main executable
add_executable(iqtree
alignment.cpp
alignmentpairwise.cpp
circularnetwork.cpp
eigendecomposition.cpp
greedy.cpp
gss.cpp
gtrmodel.cpp
guidedbootstrap.cpp
gurobiwrapper.cpp
gzstream.cpp
hashsplitset.cpp
iqtree.cpp
maalignment.cpp
matree.cpp
mexttree.cpp
modelbin.cpp
modeldna.cpp
modelfactory.cpp
modelnonrev.cpp
modelprotein.cpp
modelset.cpp
modelsubst.cpp
mpdablock.cpp
msetsblock.cpp
msplitsblock.cpp
mtree.cpp
mtreeset.cpp
ncbitree.cpp
ngs.cpp
node.cpp
optimization.cpp
parsmultistate.cpp
partitionmodel.cpp
pattern.cpp
pda.cpp
pdnetwork.cpp
pdtree.cpp
pdtreeset.cpp
phyloanalysis.cpp
phylonode.cpp
phylosupertree.cpp
phylotree.cpp
phylotreesse.cpp
pruning.cpp
rategamma.cpp
rategammainvar.cpp
rateheterogeneity.cpp
rateinvar.cpp
ratemeyerdiscrete.cpp
ratemeyerhaeseler.cpp
ratekategory.cpp
split.cpp
splitgraph.cpp
splitset.cpp
stoprule.cpp
superalignment.cpp
superalignmentpairwise.cpp
supernode.cpp
tinatree.cpp
tools.cpp
whtest_wrapper.cpp
lpwrapper.c
#modeltest_wrapper.c
fmemopen.c
nnisearch.c
modelcodon.cpp
phylosupertreeplen.cpp
phylotesting.cpp
ecopd.cpp
ecopdmtreeset.cpp
graph.cpp
)


# link special lib for WIN32
if (WIN32)
	target_link_libraries(iqtree phylolib ncl whtest zlibstatic sprng ws2_32)
else (WIN32)
	target_link_libraries(iqtree phylolib ncl whtest zlibstatic sprng)
endif (WIN32)

# setup the executable name properly
set_target_properties(iqtree PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}${EXE_SUFFIX}")

# strip the release build
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
	ADD_CUSTOM_COMMAND(TARGET iqtree POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:iqtree>)
endif()

##############################################################
# add the install targets
##############################################################
install (TARGETS iqtree DESTINATION bin)
#install (FILES "${PROJECT_SOURCE_DIR}/examples/example.phy" DESTINATION examples)
install (FILES "${PROJECT_SOURCE_DIR}/examples/test.nex" DESTINATION examples)
install (FILES "${PROJECT_SOURCE_DIR}/examples/test.budget" DESTINATION examples)
install (FILES "${PROJECT_SOURCE_DIR}/examples/test.area.nex" DESTINATION examples)
install (FILES "${PROJECT_SOURCE_DIR}/examples/test.area.budget" DESTINATION examples)
if (WIN32)
	if (EXE_SUFFIX MATCHES "omp")
		install(FILES  "${PROJECT_SOURCE_DIR}/lib/pthreadGC2.dll" DESTINATION bin)
		install(FILES  "${PROJECT_SOURCE_DIR}/lib/pthreadGC2_64.dll" DESTINATION bin)
	endif()
endif()
#install (FILES "${PROJECT_BINARY_DIR}/iqtree_config.h"        
#         DESTINATION include)

##############################################################
# build a CPack driven installer package
##############################################################
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${iqtree_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${iqtree_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${iqtree_VERSION_PATCH}")
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_SOURCE_GENERATOR "ZIP")
else(WIN32)
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_SOURCE_GENERATOR "TGZ")
endif(WIN32)
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
  "/build.*/;/debug.*/;/examples/;/manual/;/.bzr/;~$;/\\\\.svn/;/\\\\.git/;${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_PACKAGE_FILE_NAME 
	"${CMAKE_PROJECT_NAME}${EXE_SUFFIX}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_NAME}")

set(CPACK_STRIP_FILES TRUE)

include (CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
