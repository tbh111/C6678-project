################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/c6000_7.4.4

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./src/common_test.obj" \
"./src/Keystone_Serdes_init.obj" \
"./src/KeyStone_common.obj" \
"./src/KeyStone_HyperLink_Init.obj" \
"./src/KeyStone_DDR_Init.obj" \
"./src/Hyperlink_Test.obj" \
"./src/HyperLink_vectors.obj" \
"./src/HyperLink_debug.obj" \
"./src/HyperLink_Intc.obj" \
"./src/HyperLink_Edma_Performance.obj" \
"./src/HyperLink_DSP_core_performance.obj" \
"./src/CPU_access_test.obj" \
"./src/CPU_LD_ST_Test.obj" \
-l"libc.a" \
-l"C:\ti\pdk_C6678_1_1_2_5\packages\ti\platform\evmc6678l\platform_lib\lib\debug\ti.platform.evm6678l.ae66" \
"../KeyStone.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include src/subdir_vars.mk
-include subdir_rules.mk
-include src/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: HyperLink.out

# Tool invocations
HyperLink.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C6000 Linker'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --display_error_number --diag_warning=225 -z -m"HyperLink.map" -i"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/lib" -i"C:/ti/pdk_C6678_1_1_2_5/packages/ti/platform/evmc6678l/platform_lib/lib/debug" -i"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --reread_libs --warn_sections --rom_model -o "HyperLink.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C6000_EXECUTABLE_OUTPUTS__QUOTED) "HyperLink.out"
	-$(RM) "src\CPU_access_test.pp" "src\HyperLink_DSP_core_performance.pp" "src\HyperLink_Edma_Performance.pp" "src\HyperLink_Intc.pp" "src\HyperLink_debug.pp" "src\Hyperlink_Test.pp" "src\KeyStone_DDR_Init.pp" "src\KeyStone_HyperLink_Init.pp" "src\KeyStone_common.pp" "src\Keystone_Serdes_init.pp" "src\common_test.pp" 
	-$(RM) "src\CPU_LD_ST_Test.obj" "src\CPU_access_test.obj" "src\HyperLink_DSP_core_performance.obj" "src\HyperLink_Edma_Performance.obj" "src\HyperLink_Intc.obj" "src\HyperLink_debug.obj" "src\HyperLink_vectors.obj" "src\Hyperlink_Test.obj" "src\KeyStone_DDR_Init.obj" "src\KeyStone_HyperLink_Init.obj" "src\KeyStone_common.obj" "src\Keystone_Serdes_init.obj" "src\common_test.obj" 
	-$(RM) "src\CPU_LD_ST_Test.pp" "src\HyperLink_vectors.pp" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
