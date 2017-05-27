################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libfunciones.so

dependents:
	-cd /home/utnso/workspace/parser-ansisop/Debug && $(MAKE) all
	-cd /home/utnso/tp-2017-1c-No-Se-Recursa/commons/Debug && $(MAKE) all

# Tool invocations
libfunciones.so: $(OBJS) $(USER_OBJS) /home/utnso/workspace/parser-ansisop/Debug/libparser-ansisop.so /home/utnso/tp-2017-1c-No-Se-Recursa/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2017-1c-No-Se-Recursa/commons/Debug" -L"/home/utnso/workspace/parser-ansisop/Debug" -shared -o "libfunciones.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libfunciones.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/parser-ansisop/Debug/libparser-ansisop.so:
/home/utnso/tp-2017-1c-No-Se-Recursa/commons/Debug/libcommons.so:

-include ../makefile.targets
