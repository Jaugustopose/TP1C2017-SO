################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cliente-servidor.c \
../deserializador.c \
../estructurasCompartidas.c \
../logger.c \
../serializador.c 

OBJS += \
./cliente-servidor.o \
./deserializador.o \
./estructurasCompartidas.o \
./logger.o \
./serializador.o 

C_DEPS += \
./cliente-servidor.d \
./deserializador.d \
./estructurasCompartidas.d \
./logger.d \
./serializador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2017-1c-No-Se-Recursa/commons" -I"/home/utnso/tp-2017-1c-No-Se-Recursa/parser-ansisop" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


