################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/leaf/Externals/d_fft_mayer.c 

OBJS += \
./Core/leaf/Externals/d_fft_mayer.o 

C_DEPS += \
./Core/leaf/Externals/d_fft_mayer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/leaf/Externals/%.o Core/leaf/Externals/%.su Core/leaf/Externals/%.cyclo: ../Core/leaf/Externals/%.c Core/leaf/Externals/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-leaf-2f-Externals

clean-Core-2f-leaf-2f-Externals:
	-$(RM) ./Core/leaf/Externals/d_fft_mayer.cyclo ./Core/leaf/Externals/d_fft_mayer.d ./Core/leaf/Externals/d_fft_mayer.o ./Core/leaf/Externals/d_fft_mayer.su

.PHONY: clean-Core-2f-leaf-2f-Externals

