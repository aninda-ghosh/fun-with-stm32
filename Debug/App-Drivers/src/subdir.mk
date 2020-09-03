################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App-Drivers/src/iic.c \
../App-Drivers/src/sysclock.c 

OBJS += \
./App-Drivers/src/iic.o \
./App-Drivers/src/sysclock.o 

C_DEPS += \
./App-Drivers/src/iic.d \
./App-Drivers/src/sysclock.d 


# Each subdirectory must supply rules for building sources it contributes
App-Drivers/src/iic.o: ../App-Drivers/src/iic.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L152xC -DDEBUG -c -I../Core/Inc -I../App-Libraries/inc -I../App-Drivers/inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"App-Drivers/src/iic.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
App-Drivers/src/sysclock.o: ../App-Drivers/src/sysclock.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L152xC -DDEBUG -c -I../Core/Inc -I../App-Libraries/inc -I../App-Drivers/inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"App-Drivers/src/sysclock.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

