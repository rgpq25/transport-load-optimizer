#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/GRASPOptimizer.o \
	${OBJECTDIR}/SAGAOptimizer.o \
	${OBJECTDIR}/alphaReactive.o \
	${OBJECTDIR}/bin3D.o \
	${OBJECTDIR}/block.o \
	${OBJECTDIR}/chromosome.o \
	${OBJECTDIR}/client.o \
	${OBJECTDIR}/delivery.o \
	${OBJECTDIR}/deliveryUtils.o \
	${OBJECTDIR}/dispatch.o \
	${OBJECTDIR}/dispatchUtils.o \
	${OBJECTDIR}/globalExecutionTracker.o \
	${OBJECTDIR}/input.o \
	${OBJECTDIR}/layerCandidate.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/maximalSpace.o \
	${OBJECTDIR}/order.o \
	${OBJECTDIR}/route.o \
	${OBJECTDIR}/routePoint.o \
	${OBJECTDIR}/timeSlot.o \
	${OBJECTDIR}/timeSlotUtils.o \
	${OBJECTDIR}/transportUnit.o \
	${OBJECTDIR}/vehiclePattern.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/main_algorithms.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/main_algorithms.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/main_algorithms ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/GRASPOptimizer.o: GRASPOptimizer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GRASPOptimizer.o GRASPOptimizer.cpp

${OBJECTDIR}/SAGAOptimizer.o: SAGAOptimizer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SAGAOptimizer.o SAGAOptimizer.cpp

${OBJECTDIR}/alphaReactive.o: alphaReactive.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/alphaReactive.o alphaReactive.cpp

${OBJECTDIR}/bin3D.o: bin3D.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/bin3D.o bin3D.cpp

${OBJECTDIR}/block.o: block.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/block.o block.cpp

${OBJECTDIR}/chromosome.o: chromosome.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chromosome.o chromosome.cpp

${OBJECTDIR}/client.o: client.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client.o client.cpp

${OBJECTDIR}/delivery.o: delivery.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/delivery.o delivery.cpp

${OBJECTDIR}/deliveryUtils.o: deliveryUtils.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/deliveryUtils.o deliveryUtils.cpp

${OBJECTDIR}/dispatch.o: dispatch.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dispatch.o dispatch.cpp

${OBJECTDIR}/dispatchUtils.o: dispatchUtils.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dispatchUtils.o dispatchUtils.cpp

${OBJECTDIR}/globalExecutionTracker.o: globalExecutionTracker.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/globalExecutionTracker.o globalExecutionTracker.cpp

${OBJECTDIR}/input.o: input.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/input.o input.cpp

${OBJECTDIR}/layerCandidate.o: layerCandidate.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/layerCandidate.o layerCandidate.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/maximalSpace.o: maximalSpace.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/maximalSpace.o maximalSpace.cpp

${OBJECTDIR}/order.o: order.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/order.o order.cpp

${OBJECTDIR}/route.o: route.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/route.o route.cpp

${OBJECTDIR}/routePoint.o: routePoint.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/routePoint.o routePoint.cpp

${OBJECTDIR}/timeSlot.o: timeSlot.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/timeSlot.o timeSlot.cpp

${OBJECTDIR}/timeSlotUtils.o: timeSlotUtils.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/timeSlotUtils.o timeSlotUtils.cpp

${OBJECTDIR}/transportUnit.o: transportUnit.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/transportUnit.o transportUnit.cpp

${OBJECTDIR}/vehiclePattern.o: vehiclePattern.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/vehiclePattern.o vehiclePattern.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
