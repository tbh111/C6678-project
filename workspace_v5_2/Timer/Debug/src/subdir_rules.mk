################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/KeyStone_common.obj: C:/Users/tbh/workspace_v5_2/common/KeyStone_common.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/KeyStone_common.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/timer_main.obj: ../src/timer_main.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/timer_main.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/timer_vectors.obj: ../src/timer_vectors.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -O3 --symdebug:none --include_path="C:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Users/tbh/workspace_v5_2/common" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages/ti/csl" --include_path="C:/ti/pdk_C6678_1_1_2_5/packages" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="src/timer_vectors.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


