################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/apps/Lib/dsplib/src/%.obj: ../src/apps/Lib/dsplib/src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-msp430_18.12.6.LTS/bin/cl430" -vmspx --data_model=restricted -O0 --opt_for_speed=0 --use_hw_mpy=F5 --include_path="/Applications/ti/ccs930/ccs/ccs_base/msp430/include" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/src/apps/Lib/dsplib/include" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/src/apps/Lib" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/src/apps/Lib/mspmath" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/src/apps" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/driverlib/MSP430FR5xx_6xx" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/profile" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/src/kernel/inc" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU/simulator/inc" --include_path="/Users/elk/Project/Github/IntOS/ELK-runtime-GNU" --include_path="/Applications/ti/ccs930/ccs/tools/compiler/ti-cgt-msp430_18.12.6.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5994__ -g --printf_support=full --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="src/apps/Lib/dsplib/src/$(basename $(<F)).d_raw" --obj_directory="src/apps/Lib/dsplib/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


