#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Paging optional component
############################################################

PROJECT(OgrePaging)

# define header and source files for the library
set (HEADER_FILES
	include/OgreGrid2DPageStrategy.h
	include/OgreGrid3DPageStrategy.h
	include/OgrePage.h
	include/OgrePageConnection.h
	include/OgrePageContent.h
	include/OgrePageContentCollection.h
	include/OgrePageContentCollectionFactory.h
	include/OgrePageContentFactory.h
	include/OgrePagedWorld.h
	include/OgrePagedWorldSection.h
	include/OgrePageFileFormats.h
	include/OgrePageManager.h
	include/OgrePageStrategy.h
	include/OgrePaging.h
	include/OgrePagingPrerequisites.h
	include/OgreSimplePageContentCollection.h
)

set (SOURCE_FILES
	src/OgreGrid2DPageStrategy.cpp
	src/OgreGrid3DPageStrategy.cpp
	src/OgrePage.cpp
	src/OgrePageContent.cpp
	src/OgrePageContentCollection.cpp
	src/OgrePagedWorld.cpp
	src/OgrePagedWorldSection.cpp
	src/OgrePageManager.cpp
	src/OgreSimplePageContentCollection.cpp
)


# Add needed definitions 
add_definitions(-DOGRE_PAGING_EXPORTS -D_MT -D_USRDLL)

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)

# setup target
ogre_add_library(OgrePaging ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgrePaging PROPERTIES VERSION ${OGRE_SOVERSION})
target_link_libraries(OgrePaging OgreMain)
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgrePaging ${OGRE_THREAD_LIBRARIES})
endif ()


# install 
ogre_config_component(OgrePaging)

ogre_config_framework(OgrePaging)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Paging
)

