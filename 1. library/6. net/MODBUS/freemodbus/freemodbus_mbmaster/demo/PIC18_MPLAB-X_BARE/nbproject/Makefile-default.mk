#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=port/mbporttimer.c port/mbportevent.c port/mbportother.c port/mbportserial.c demo.c ../../mbmaster/common/mbutils.c ../../mbmaster/ascii/mbmascii.c ../../mbmaster/mbm.c ../../mbmaster/functions/mbmfuncholding.c ../../mbmaster/functions/mbmfuncinput.c ../../mbmaster/rtu/mbmcrc.c ../../mbmaster/rtu/mbmrtu.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/port/mbporttimer.p1 ${OBJECTDIR}/port/mbportevent.p1 ${OBJECTDIR}/port/mbportother.p1 ${OBJECTDIR}/port/mbportserial.p1 ${OBJECTDIR}/demo.p1 ${OBJECTDIR}/_ext/879292125/mbutils.p1 ${OBJECTDIR}/_ext/662634777/mbmascii.p1 ${OBJECTDIR}/_ext/194147881/mbm.p1 ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/port/mbporttimer.p1.d ${OBJECTDIR}/port/mbportevent.p1.d ${OBJECTDIR}/port/mbportother.p1.d ${OBJECTDIR}/port/mbportserial.p1.d ${OBJECTDIR}/demo.p1.d ${OBJECTDIR}/_ext/879292125/mbutils.p1.d ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d ${OBJECTDIR}/_ext/194147881/mbm.p1.d ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/port/mbporttimer.p1 ${OBJECTDIR}/port/mbportevent.p1 ${OBJECTDIR}/port/mbportother.p1 ${OBJECTDIR}/port/mbportserial.p1 ${OBJECTDIR}/demo.p1 ${OBJECTDIR}/_ext/879292125/mbutils.p1 ${OBJECTDIR}/_ext/662634777/mbmascii.p1 ${OBJECTDIR}/_ext/194147881/mbm.p1 ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1

# Source Files
SOURCEFILES=port/mbporttimer.c port/mbportevent.c port/mbportother.c port/mbportserial.c demo.c ../../mbmaster/common/mbutils.c ../../mbmaster/ascii/mbmascii.c ../../mbmaster/mbm.c ../../mbmaster/functions/mbmfuncholding.c ../../mbmaster/functions/mbmfuncinput.c ../../mbmaster/rtu/mbmcrc.c ../../mbmaster/rtu/mbmrtu.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F4520
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/port/mbporttimer.p1: port/mbporttimer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbporttimer.p1.d 
	@${RM} ${OBJECTDIR}/port/mbporttimer.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbporttimer.p1 port/mbporttimer.c 
	@-${MV} ${OBJECTDIR}/port/mbporttimer.d ${OBJECTDIR}/port/mbporttimer.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbporttimer.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportevent.p1: port/mbportevent.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportevent.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportevent.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportevent.p1 port/mbportevent.c 
	@-${MV} ${OBJECTDIR}/port/mbportevent.d ${OBJECTDIR}/port/mbportevent.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportevent.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportother.p1: port/mbportother.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportother.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportother.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportother.p1 port/mbportother.c 
	@-${MV} ${OBJECTDIR}/port/mbportother.d ${OBJECTDIR}/port/mbportother.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportother.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportserial.p1: port/mbportserial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportserial.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportserial.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportserial.p1 port/mbportserial.c 
	@-${MV} ${OBJECTDIR}/port/mbportserial.d ${OBJECTDIR}/port/mbportserial.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportserial.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/demo.p1: demo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/demo.p1.d 
	@${RM} ${OBJECTDIR}/demo.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/demo.p1 demo.c 
	@-${MV} ${OBJECTDIR}/demo.d ${OBJECTDIR}/demo.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/demo.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/879292125/mbutils.p1: ../../mbmaster/common/mbutils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/879292125" 
	@${RM} ${OBJECTDIR}/_ext/879292125/mbutils.p1.d 
	@${RM} ${OBJECTDIR}/_ext/879292125/mbutils.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/879292125/mbutils.p1 ../../mbmaster/common/mbutils.c 
	@-${MV} ${OBJECTDIR}/_ext/879292125/mbutils.d ${OBJECTDIR}/_ext/879292125/mbutils.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/879292125/mbutils.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/662634777/mbmascii.p1: ../../mbmaster/ascii/mbmascii.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/662634777" 
	@${RM} ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d 
	@${RM} ${OBJECTDIR}/_ext/662634777/mbmascii.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/662634777/mbmascii.p1 ../../mbmaster/ascii/mbmascii.c 
	@-${MV} ${OBJECTDIR}/_ext/662634777/mbmascii.d ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/194147881/mbm.p1: ../../mbmaster/mbm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/194147881" 
	@${RM} ${OBJECTDIR}/_ext/194147881/mbm.p1.d 
	@${RM} ${OBJECTDIR}/_ext/194147881/mbm.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/194147881/mbm.p1 ../../mbmaster/mbm.c 
	@-${MV} ${OBJECTDIR}/_ext/194147881/mbm.d ${OBJECTDIR}/_ext/194147881/mbm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/194147881/mbm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1: ../../mbmaster/functions/mbmfuncholding.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1359164771" 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 ../../mbmaster/functions/mbmfuncholding.c 
	@-${MV} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.d ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1: ../../mbmaster/functions/mbmfuncinput.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1359164771" 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 ../../mbmaster/functions/mbmfuncinput.c 
	@-${MV} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.d ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1938956741/mbmcrc.p1: ../../mbmaster/rtu/mbmcrc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1938956741" 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 ../../mbmaster/rtu/mbmcrc.c 
	@-${MV} ${OBJECTDIR}/_ext/1938956741/mbmcrc.d ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1938956741/mbmrtu.p1: ../../mbmaster/rtu/mbmrtu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1938956741" 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1 ../../mbmaster/rtu/mbmrtu.c 
	@-${MV} ${OBJECTDIR}/_ext/1938956741/mbmrtu.d ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/port/mbporttimer.p1: port/mbporttimer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbporttimer.p1.d 
	@${RM} ${OBJECTDIR}/port/mbporttimer.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbporttimer.p1 port/mbporttimer.c 
	@-${MV} ${OBJECTDIR}/port/mbporttimer.d ${OBJECTDIR}/port/mbporttimer.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbporttimer.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportevent.p1: port/mbportevent.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportevent.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportevent.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportevent.p1 port/mbportevent.c 
	@-${MV} ${OBJECTDIR}/port/mbportevent.d ${OBJECTDIR}/port/mbportevent.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportevent.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportother.p1: port/mbportother.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportother.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportother.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportother.p1 port/mbportother.c 
	@-${MV} ${OBJECTDIR}/port/mbportother.d ${OBJECTDIR}/port/mbportother.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportother.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/port/mbportserial.p1: port/mbportserial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/port" 
	@${RM} ${OBJECTDIR}/port/mbportserial.p1.d 
	@${RM} ${OBJECTDIR}/port/mbportserial.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/port/mbportserial.p1 port/mbportserial.c 
	@-${MV} ${OBJECTDIR}/port/mbportserial.d ${OBJECTDIR}/port/mbportserial.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/port/mbportserial.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/demo.p1: demo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/demo.p1.d 
	@${RM} ${OBJECTDIR}/demo.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/demo.p1 demo.c 
	@-${MV} ${OBJECTDIR}/demo.d ${OBJECTDIR}/demo.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/demo.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/879292125/mbutils.p1: ../../mbmaster/common/mbutils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/879292125" 
	@${RM} ${OBJECTDIR}/_ext/879292125/mbutils.p1.d 
	@${RM} ${OBJECTDIR}/_ext/879292125/mbutils.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/879292125/mbutils.p1 ../../mbmaster/common/mbutils.c 
	@-${MV} ${OBJECTDIR}/_ext/879292125/mbutils.d ${OBJECTDIR}/_ext/879292125/mbutils.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/879292125/mbutils.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/662634777/mbmascii.p1: ../../mbmaster/ascii/mbmascii.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/662634777" 
	@${RM} ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d 
	@${RM} ${OBJECTDIR}/_ext/662634777/mbmascii.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/662634777/mbmascii.p1 ../../mbmaster/ascii/mbmascii.c 
	@-${MV} ${OBJECTDIR}/_ext/662634777/mbmascii.d ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/662634777/mbmascii.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/194147881/mbm.p1: ../../mbmaster/mbm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/194147881" 
	@${RM} ${OBJECTDIR}/_ext/194147881/mbm.p1.d 
	@${RM} ${OBJECTDIR}/_ext/194147881/mbm.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/194147881/mbm.p1 ../../mbmaster/mbm.c 
	@-${MV} ${OBJECTDIR}/_ext/194147881/mbm.d ${OBJECTDIR}/_ext/194147881/mbm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/194147881/mbm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1: ../../mbmaster/functions/mbmfuncholding.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1359164771" 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1 ../../mbmaster/functions/mbmfuncholding.c 
	@-${MV} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.d ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1359164771/mbmfuncholding.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1: ../../mbmaster/functions/mbmfuncinput.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1359164771" 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1 ../../mbmaster/functions/mbmfuncinput.c 
	@-${MV} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.d ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1359164771/mbmfuncinput.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1938956741/mbmcrc.p1: ../../mbmaster/rtu/mbmcrc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1938956741" 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1 ../../mbmaster/rtu/mbmcrc.c 
	@-${MV} ${OBJECTDIR}/_ext/1938956741/mbmcrc.d ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1938956741/mbmcrc.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1938956741/mbmrtu.p1: ../../mbmaster/rtu/mbmrtu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1938956741" 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -DXPRJ_default=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits $(COMPARISON_BUILD)  -gdwarf-3 -mstack=compiled     -o ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1 ../../mbmaster/rtu/mbmrtu.c 
	@-${MV} ${OBJECTDIR}/_ext/1938956741/mbmrtu.d ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1938956741/mbmrtu.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.map  -D__DEBUG=1  -DXPRJ_default=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits -gdwarf-3 -mstack=compiled        $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.map  -DXPRJ_default=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1    -fno-short-double -fno-short-float -memi=wordwrite -O0 -fasmfile -maddrqual=ignore -xassembler-with-cpp -I"port" -I"../../mbmaster/include" -I"../../mbmaster/include/common" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx032 -Wl,--data-init -mno-keep-startup -mno-download -mdefault-config-bits -gdwarf-3 -mstack=compiled     $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/PIC18_MPLAB-X_BARE.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default
