################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/CPU_LD_ST_Test.obj: C:/Users/tbh/workspace_v5_2/common/CPU_LD_ST_Test.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/CPU_LD_ST_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/CPU_access_test.obj: C:/Users/tbh/workspace_v5_2/common/CPU_access_test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/CPU_access_test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/HyperLink_DSP_core_performance.obj: ../src/HyperLink_DSP_core_performance.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/HyperLink_DSP_core_performance.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/HyperLink_Edma_Performance.obj: ../src/HyperLink_Edma_Performance.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/HyperLink_Edma_Performance.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/HyperLink_Intc.obj: ../src/HyperLink_Intc.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/HyperLink_Intc.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/HyperLink_debug.obj: ../src/HyperLink_debug.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/HyperLink_debug.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/HyperLink_vectors.obj: ../src/HyperLink_vectors.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/HyperLink_vectors.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Hyperlink_Test.obj: ../src/Hyperlink_Test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/Hyperlink_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_DDR_Init.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_DDR_Init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_DDR_Init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_HyperLink_Init.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_HyperLink_Init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_HyperLink_Init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
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


