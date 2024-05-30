################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Externals/d_fft_mayer.c 

OBJS += \
./Core/Externals/d_fft_mayer.o 

C_DEPS += \
./Core/Externals/d_fft_mayer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Externals/%.o Core/Externals/%.su Core/Externals/%.cyclo: ../Core/Externals/%.c Core/Externals/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32_Synth/Leaf_Synth2/Core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Externals

clean-Core-2f-Externals:
	-$(RM) ./Core/Externals/d_fft_mayer.cyclo ./Core/Externals/d_fft_mayer.d ./Core/Externals/d_fft_mayer.o ./Core/Externals/d_fft_mayer.su

.PHONY: clean-Core-2f-Externals

