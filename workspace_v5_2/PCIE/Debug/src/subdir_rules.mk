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

src/KeyStone_DDR_Init.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_DDR_Init.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_DDR_Init.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/KeyStone_PCIE_Init_drv.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_PCIE_Init_drv.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_PCIE_Init_drv.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
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

src/PCIE_DSP_core_performance.obj: ../src/PCIE_DSP_core_performance.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/PCIE_DSP_core_performance.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/PCIE_Edma_Performance.obj: ../src/PCIE_Edma_Performance.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/PCIE_Edma_Performance.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/PCIE_Intc.obj: ../src/PCIE_Intc.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/PCIE_Intc.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/PCIE_Test.obj: ../src/PCIE_Test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/PCIE_Test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/PCIE_vectors.obj: ../src/PCIE_vectors.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/PCIE_vectors.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common_test.obj: C:/Users/tbh/workspace_v5_2/common/common_test.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/common_test.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


