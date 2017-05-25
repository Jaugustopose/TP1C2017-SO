################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cliente-servidor.c \
../deserializador.c \
../estructurasCompartidas.c \
../serializador.c 

OBJS += \
./cliente-servidor.o \
./deserializador.o \
./estructurasCompartidas.o \
./serializador.o 

C_DEPS += \
./cliente-servidor.d \
./deserializador.d \
./estructurasCompartidas.d \
./serializador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/parser" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


