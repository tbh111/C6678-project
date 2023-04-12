################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/GE_2DSP_Test.obj: ../src/GE_2DSP_Test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_2DSP_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_Interrupts.obj: ../src/GE_Interrupts.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_Interrupts.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_PktDMA_Init.obj: ../src/GE_PktDMA_Init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_PktDMA_Init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_Test.obj: ../src/GE_Test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_debug.obj: ../src/GE_debug.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_debug.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_loopback_Test.obj: ../src/GE_loopback_Test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_loopback_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/GE_vectors.obj: ../src/GE_vectors.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/GE_vectors.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_DDR_Init.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_DDR_Init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_DDR_Init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_GE_Init_drv.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_GE_Init_drv.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_GE_Init_drv.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_Navigator_init_drv.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_Navigator_init_drv.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_Navigator_init_drv.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_common.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_common.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_common.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Keystone_Serdes_init.obj: C:/Users/tbh/workspace_v5_2/common/Keystone_Serdes_init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/Keystone_Serdes_init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common_test.obj: C:/Users/tbh/workspace_v5_2/common/common_test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/common_test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


