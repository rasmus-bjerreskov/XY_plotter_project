################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cr_cpp_config.cpp \
../src/cr_startup_lpc15xx.cpp \
../src/test.cpp \
../src/test2.cpp 

C_SRCS += \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/cr_cpp_config.o \
./src/cr_startup_lpc15xx.o \
./src/crp.o \
./src/sysinit.o \
./src/test.o \
./src/test2.o 

CPP_DEPS += \
./src/cr_cpp_config.d \
./src/cr_startup_lpc15xx.d \
./src/test.d \
./src/test2.d 

C_DEPS += \
./src/crp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -D__NEWLIB__ -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\test2\inc" -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\lpc_chip_15xx\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m3 -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -D__NEWLIB__ -DCORE_M3 -D__USE_LPCOPEN -DCPP_USE_HEAP -D__LPC15XX__ -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\lpc_board_nxp_lpcxpresso_1549\inc" -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\test2\inc" -I"C:\Users\Rasmus\Documents\MCUXpressoIDE_11.2.0_4120\XY_plotter_project\lpc_chip_15xx\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m3 -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


