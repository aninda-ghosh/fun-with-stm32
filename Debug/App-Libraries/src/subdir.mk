################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App-Libraries/src/mpu.c 

OBJS += \
./App-Libraries/src/mpu.o 

C_DEPS += \
./App-Libraries/src/mpu.d 


# Each subdirectory must supply rules for building sources it contributes
App-Libraries/src/mpu.o: ../App-Libraries/src/mpu.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L152xC -DDEBUG -c -I../Core/Inc -I../App-Libraries/inc -I../App-Drivers/inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"App-Libraries/src/mpu.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

