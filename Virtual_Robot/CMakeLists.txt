CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

SET( project_name VirtualMachine )
PROJECT( ${project_name} )

OPTION( USE_QT4 "Use QT4" OFF ) 

IF (USE_QT4)
 SET(CMAKE_AUTOMOC ON)
 SET(CMAKE_AUTORCC ON)

 FIND_PACKAGE( Qt4 REQUIRED )
 SET( QT_USE_QTGUI TRUE )
 SET( QT_USE_QTXML TRUE )
 SET( QT_USE_QTNETWORK TRUE )
 INCLUDE( ${QT_USE_FILE} )
ELSE (USE_QT4)
 SET(CMAKE_AUTOMOC ON)
 SET(CMAKE_AUTORCC ON)
 
 FIND_PACKAGE( Qt5Concurrent REQUIRED )
 FIND_PACKAGE( Qt5Core REQUIRED )
 FIND_PACKAGE( Qt5Widgets REQUIRED )
 FIND_PACKAGE( Qt5Network REQUIRED )
 FIND_PACKAGE( Qt5Xml REQUIRED )
ENDIF (USE_QT4)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF (USE_QT4)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt4::QtCore)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt4::QtGui)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt4::QtNetwork)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt4::QtXml)
ELSE (USE_QT4)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::Concurrent)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::Core)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::Widgets)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::Network)
 SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::Xml)
ENDIF (USE_QT4)

# Boost
SET (Boost_USE_STATIC_LIBS        ON) # only find static libs
SET (Boost_USE_MULTITHREADED      ON)
SET (Boost_USE_STATIC_RUNTIME     OFF)
FIND_PACKAGE(Boost COMPONENTS system chrono date_time thread REQUIRED) 
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${Boost_LIBRARIES})

# add DROBOTCORE library
SET(DROBOTCORE_INCLUDE_DIR         "${CMAKE_INSTALL_PREFIX}/include"           CACHE STRING "Directory with DROBOTCORE include files")
SET(DROBOTCORE_LIBRARIES_RELEASE   "${CMAKE_INSTALL_PREFIX}/lib/dobotCore.lib"  CACHE STRING "DROBOTCORE RELEASE library to link with")
SET(DROBOTCORE_LIBRARIES_DEBUG     "${CMAKE_INSTALL_PREFIX}/lib/drobotCore.lib"  CACHE STRING "DROBOTCORE DEBUG library to link with")

IF(NOT EXISTS "${DROBOTCORE_INCLUDE_DIR}/drobotCore")
  MESSAGE(FATAL_ERROR "DROBOTCORE include directory ${DROBOTCORE_INCLUDE_DIR} is not correct.")
ENDIF()
FOREACH(onelib ${DROBOTCORE_LIBRARIES_RELEASE})
  IF(NOT EXISTS ${onelib})
    MESSAGE(FATAL_ERROR "DROBOTCORE library ${onelib} was not found.")
  ENDIF()
ENDFOREACH()
FOREACH(onelib ${DROBOTCORE_LIBRARIES_DEBUG})
  IF(NOT EXISTS ${onelib})
    MESSAGE(FATAL_ERROR "DROBOTCORE library ${onelib} was not found.")
  ENDIF()
ENDFOREACH()

INCLUDE_DIRECTORIES(${DROBOTCORE_INCLUDE_DIR})
SET (ADDITIONAL_LIBS_RELEASE ${ADDITIONAL_LIBS_RELEASE} ${DROBOTCORE_LIBRARIES_RELEASE})
SET (ADDITIONAL_LIBS_DEBUG ${ADDITIONAL_LIBS_DEBUG} ${DROBOTCORE_LIBRARIES_DEBUG})

# add DFORCESENSORCORE library
SET(DFORCESENSORCORE_INCLUDE_DIR         "${CMAKE_INSTALL_PREFIX}/include"           CACHE STRING "Directory with DFORCESENSORCORE include files")
SET(DFORCESENSORCORE_LIBRARIES_RELEASE   "${CMAKE_INSTALL_PREFIX}/lib/dForceSensorCore.lib"  CACHE STRING "DFORCESENSORCORE RELEASE library to link with")
SET(DFORCESENSORCORE_LIBRARIES_DEBUG     "${CMAKE_INSTALL_PREFIX}/lib/dForceSensorCore.lib"  CACHE STRING "DFORCESENSORCORE DEBUG library to link with")

IF(NOT EXISTS "${DFORCESENSORCORE_INCLUDE_DIR}/dForceSensorCore")
  MESSAGE(FATAL_ERROR "DFORCESENSORCORE include directory ${DFORCESENSORCORE_INCLUDE_DIR} is not correct.")
ENDIF()
FOREACH(onelib ${DFORCESENSORCORE_LIBRARIES_RELEASE})
  IF(NOT EXISTS ${onelib})
    MESSAGE(FATAL_ERROR "DFORCESENSORCORE library ${onelib} was not found.")
  ENDIF()
ENDFOREACH()
FOREACH(onelib ${DFORCESENSORCORE_LIBRARIES_DEBUG})
  IF(NOT EXISTS ${onelib})
    MESSAGE(FATAL_ERROR "DFORCESENSORCORE library ${onelib} was not found.")
  ENDIF()
ENDFOREACH()

find_package(RL COMPONENTS MDL REQUIRED)
FIND_LIBRARY(${RL_LIBRARIES})
SET (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${RL_LIBRARIES})

INCLUDE_DIRECTORIES(${DFORCESENSORCORE_INCLUDE_DIR})
SET (ADDITIONAL_LIBS_RELEASE ${ADDITIONAL_LIBS_RELEASE} ${DFORCESENSORCORE_LIBRARIES_RELEASE})
SET (ADDITIONAL_LIBS_DEBUG ${ADDITIONAL_LIBS_DEBUG} ${DFORCESENSORCORE_LIBRARIES_DEBUG})

SET(app_src_h 
src/representations/JointAnglesVariableRepresentation.h
src/representations/MachineRepresentation.h
src/representations/NumericVariableRepresentation.h
src/representations/ParameterRepresentation.h
src/representations/PoseVariableRepresentation.h
src/representations/RobotRepresentation.h
src/representations/StringVariableRepresentation.h
src/representations/ForceSensorRepresentation.h
src/gui/MainWindow.h
src/gui/RobotWidget.h
src/ApplicationError.h
src/Controller.h
src/MachineConfiguration.h
src/MachineManager.h
src/Robot.h
src/Robot_MITSUBISHI_RV2F.h
src/RobotJointAngles.h
src/RobotPose.h
src/XMLMachineConfigurationReader.h
src/Server.h
src/ForceSensor.h
src/ForceSensorMeasurement.h
src/ForceSensorServer.h
src/ForceSensor_OptoForce_v18.h
#src/VirtualPLC.h
)

SET(app_src_cpp
src/representations/JointAnglesVariableRepresentation.cpp
src/representations/MachineRepresentation.cpp
src/representations/NumericVariableRepresentation.cpp
src/representations/ParameterRepresentation.cpp
src/representations/PoseVariableRepresentation.cpp
src/representations/RobotRepresentation.cpp
src/representations/StringVariableRepresentation.cpp
src/representations/ForceSensorRepresentation.cpp
src/gui/MainWindow.cpp
src/gui/RobotWidget.cpp
src/ApplicationError.cpp
src/Controller.cpp
src/Main.cpp
src/MachineConfiguration.cpp
src/MachineManager.cpp
src/Robot.cpp
src/Robot_MITSUBISHI_RV2F.cpp
src/RobotJointAngles.cpp
src/RobotPose.cpp
src/XMLMachineConfigurationReader.cpp
src/server.cpp
src/ForceSensor.cpp
src/ForceSensorMeasurement.cpp
src/ForceSensorServer.cpp
src/ForceSensor_OptoForce_v18.cpp
#src/VirtualPLC.cpp
)

SET(app_qt_resources 
#VirtualMachine.qrc
)

IF (USE_QT4)
  IF(WIN32) 
    IF(MSVC)
      ADD_DEFINITIONS (/Zc:wchar_t-) #Do not treat wchar_t as built in type.
    ENDIF (MSVC)
  ENDIF (WIN32)
ELSE (USE_QT4)
  IF(WIN32) 
    IF(MSVC)
      ADD_DEFINITIONS (/Zc:wchar_t) #Treat wchar_t as built in type.
    ENDIF (MSVC)
  ENDIF (WIN32)
ENDIF (USE_QT4)

ADD_EXECUTABLE(${project_name} ${app_src_h} ${app_src_cpp} ${app_qt_resources})
TARGET_LINK_LIBRARIES( ${project_name} ${ADDITIONAL_LIBS})
TARGET_LINK_LIBRARIES( ${project_name} ${ADDITIONAL_LIBS} )
FOREACH(onelib ${ADDITIONAL_LIBS_RELEASE})
 TARGET_LINK_LIBRARIES( ${project_name} optimized ${onelib} )
ENDFOREACH()
FOREACH(onelib ${ADDITIONAL_LIBS_DEBUG})
 TARGET_LINK_LIBRARIES( ${project_name} debug ${onelib} )
ENDFOREACH()


INSTALL(TARGETS ${project_name} DESTINATION .)