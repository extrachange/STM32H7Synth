################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/leaf/Src/leaf-analysis.c \
../Core/leaf/Src/leaf-delay.c \
../Core/leaf/Src/leaf-distortion.c \
../Core/leaf/Src/leaf-dynamics.c \
../Core/leaf/Src/leaf-effects.c \
../Core/leaf/Src/leaf-electrical.c \
../Core/leaf/Src/leaf-envelopes.c \
../Core/leaf/Src/leaf-filters.c \
../Core/leaf/Src/leaf-instruments.c \
../Core/leaf/Src/leaf-math.c \
../Core/leaf/Src/leaf-mempool.c \
../Core/leaf/Src/leaf-midi.c \
../Core/leaf/Src/leaf-oscillators.c \
../Core/leaf/Src/leaf-physical.c \
../Core/leaf/Src/leaf-reverb.c \
../Core/leaf/Src/leaf-sampling.c \
../Core/leaf/Src/leaf-tables.c \
../Core/leaf/Src/leaf-vocal.c \
../Core/leaf/Src/leaf.c 

OBJS += \
./Core/leaf/Src/leaf-analysis.o \
./Core/leaf/Src/leaf-delay.o \
./Core/leaf/Src/leaf-distortion.o \
./Core/leaf/Src/leaf-dynamics.o \
./Core/leaf/Src/leaf-effects.o \
./Core/leaf/Src/leaf-electrical.o \
./Core/leaf/Src/leaf-envelopes.o \
./Core/leaf/Src/leaf-filters.o \
./Core/leaf/Src/leaf-instruments.o \
./Core/leaf/Src/leaf-math.o \
./Core/leaf/Src/leaf-mempool.o \
./Core/leaf/Src/leaf-midi.o \
./Core/leaf/Src/leaf-oscillators.o \
./Core/leaf/Src/leaf-physical.o \
./Core/leaf/Src/leaf-reverb.o \
./Core/leaf/Src/leaf-sampling.o \
./Core/leaf/Src/leaf-tables.o \
./Core/leaf/Src/leaf-vocal.o \
./Core/leaf/Src/leaf.o 

C_DEPS += \
./Core/leaf/Src/leaf-analysis.d \
./Core/leaf/Src/leaf-delay.d \
./Core/leaf/Src/leaf-distortion.d \
./Core/leaf/Src/leaf-dynamics.d \
./Core/leaf/Src/leaf-effects.d \
./Core/leaf/Src/leaf-electrical.d \
./Core/leaf/Src/leaf-envelopes.d \
./Core/leaf/Src/leaf-filters.d \
./Core/leaf/Src/leaf-instruments.d \
./Core/leaf/Src/leaf-math.d \
./Core/leaf/Src/leaf-mempool.d \
./Core/leaf/Src/leaf-midi.d \
./Core/leaf/Src/leaf-oscillators.d \
./Core/leaf/Src/leaf-physical.d \
./Core/leaf/Src/leaf-reverb.d \
./Core/leaf/Src/leaf-sampling.d \
./Core/leaf/Src/leaf-tables.d \
./Core/leaf/Src/leaf-vocal.d \
./Core/leaf/Src/leaf.d 


# Each subdirectory must supply rules for building sources it contributes
Core/leaf/Src/%.o Core/leaf/Src/%.su Core/leaf/Src/%.cyclo: ../Core/leaf/Src/%.c Core/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-leaf-2f-Src

clean-Core-2f-leaf-2f-Src:
	-$(RM) ./Core/leaf/Src/leaf-analysis.cyclo ./Core/leaf/Src/leaf-analysis.d ./Core/leaf/Src/leaf-analysis.o ./Core/leaf/Src/leaf-analysis.su ./Core/leaf/Src/leaf-delay.cyclo ./Core/leaf/Src/leaf-delay.d ./Core/leaf/Src/leaf-delay.o ./Core/leaf/Src/leaf-delay.su ./Core/leaf/Src/leaf-distortion.cyclo ./Core/leaf/Src/leaf-distortion.d ./Core/leaf/Src/leaf-distortion.o ./Core/leaf/Src/leaf-distortion.su ./Core/leaf/Src/leaf-dynamics.cyclo ./Core/leaf/Src/leaf-dynamics.d ./Core/leaf/Src/leaf-dynamics.o ./Core/leaf/Src/leaf-dynamics.su ./Core/leaf/Src/leaf-effects.cyclo ./Core/leaf/Src/leaf-effects.d ./Core/leaf/Src/leaf-effects.o ./Core/leaf/Src/leaf-effects.su ./Core/leaf/Src/leaf-electrical.cyclo ./Core/leaf/Src/leaf-electrical.d ./Core/leaf/Src/leaf-electrical.o ./Core/leaf/Src/leaf-electrical.su ./Core/leaf/Src/leaf-envelopes.cyclo ./Core/leaf/Src/leaf-envelopes.d ./Core/leaf/Src/leaf-envelopes.o ./Core/leaf/Src/leaf-envelopes.su ./Core/leaf/Src/leaf-filters.cyclo ./Core/leaf/Src/leaf-filters.d ./Core/leaf/Src/leaf-filters.o ./Core/leaf/Src/leaf-filters.su ./Core/leaf/Src/leaf-instruments.cyclo ./Core/leaf/Src/leaf-instruments.d ./Core/leaf/Src/leaf-instruments.o ./Core/leaf/Src/leaf-instruments.su ./Core/leaf/Src/leaf-math.cyclo ./Core/leaf/Src/leaf-math.d ./Core/leaf/Src/leaf-math.o ./Core/leaf/Src/leaf-math.su ./Core/leaf/Src/leaf-mempool.cyclo ./Core/leaf/Src/leaf-mempool.d ./Core/leaf/Src/leaf-mempool.o ./Core/leaf/Src/leaf-mempool.su ./Core/leaf/Src/leaf-midi.cyclo ./Core/leaf/Src/leaf-midi.d ./Core/leaf/Src/leaf-midi.o ./Core/leaf/Src/leaf-midi.su ./Core/leaf/Src/leaf-oscillators.cyclo ./Core/leaf/Src/leaf-oscillators.d ./Core/leaf/Src/leaf-oscillators.o ./Core/leaf/Src/leaf-oscillators.su ./Core/leaf/Src/leaf-physical.cyclo ./Core/leaf/Src/leaf-physical.d ./Core/leaf/Src/leaf-physical.o ./Core/leaf/Src/leaf-physical.su ./Core/leaf/Src/leaf-reverb.cyclo ./Core/leaf/Src/leaf-reverb.d ./Core/leaf/Src/leaf-reverb.o ./Core/leaf/Src/leaf-reverb.su ./Core/leaf/Src/leaf-sampling.cyclo ./Core/leaf/Src/leaf-sampling.d ./Core/leaf/Src/leaf-sampling.o ./Core/leaf/Src/leaf-sampling.su ./Core/leaf/Src/leaf-tables.cyclo ./Core/leaf/Src/leaf-tables.d ./Core/leaf/Src/leaf-tables.o ./Core/leaf/Src/leaf-tables.su ./Core/leaf/Src/leaf-vocal.cyclo ./Core/leaf/Src/leaf-vocal.d ./Core/leaf/Src/leaf-vocal.o ./Core/leaf/Src/leaf-vocal.su ./Core/leaf/Src/leaf.cyclo ./Core/leaf/Src/leaf.d ./Core/leaf/Src/leaf.o ./Core/leaf/Src/leaf.su

.PHONY: clean-Core-2f-leaf-2f-Src

