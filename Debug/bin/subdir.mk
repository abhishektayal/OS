################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../bin/coff2noff.o 

C_SRCS += \
../bin/coff2flat.c \
../bin/coff2noff.c \
../bin/d.c \
../bin/disasm.c \
../bin/execute.c \
../bin/main.c \
../bin/opstrings.c \
../bin/out.c \
../bin/system.c 

OBJS += \
./bin/coff2flat.o \
./bin/coff2noff.o \
./bin/d.o \
./bin/disasm.o \
./bin/execute.o \
./bin/main.o \
./bin/opstrings.o \
./bin/out.o \
./bin/system.o 

C_DEPS += \
./bin/coff2flat.d \
./bin/coff2noff.d \
./bin/d.d \
./bin/disasm.d \
./bin/execute.d \
./bin/main.d \
./bin/opstrings.d \
./bin/out.d \
./bin/system.d 


# Each subdirectory must supply rules for building sources it contributes
bin/%.o: ../bin/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


