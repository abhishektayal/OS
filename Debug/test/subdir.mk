################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../test/carljr.o \
../test/halt.o \
../test/maintest.o \
../test/matmult.o \
../test/sort.o \
../test/start.o \
../test/test1.o \
../test/test10.o \
../test/test11.o \
../test/test2.o \
../test/test3.o \
../test/test4.o \
../test/test5.o \
../test/test6.o \
../test/test7.o \
../test/test8.o \
../test/test9.o \
../test/testfiles.o 

C_SRCS += \
../test/carljr.c \
../test/halt.c \
../test/maintest.c \
../test/matmult.c \
../test/sort.c \
../test/test1.c \
../test/test10.c \
../test/test11.c \
../test/test12.c \
../test/test2.c \
../test/test3.c \
../test/test4.c \
../test/test5.c \
../test/test6.c \
../test/test7.c \
../test/test8.c \
../test/test9.c \
../test/testfiles.c 

OBJS += \
./test/carljr.o \
./test/halt.o \
./test/maintest.o \
./test/matmult.o \
./test/sort.o \
./test/test1.o \
./test/test10.o \
./test/test11.o \
./test/test12.o \
./test/test2.o \
./test/test3.o \
./test/test4.o \
./test/test5.o \
./test/test6.o \
./test/test7.o \
./test/test8.o \
./test/test9.o \
./test/testfiles.o 

C_DEPS += \
./test/carljr.d \
./test/halt.d \
./test/maintest.d \
./test/matmult.d \
./test/sort.d \
./test/test1.d \
./test/test10.d \
./test/test11.d \
./test/test12.d \
./test/test2.d \
./test/test3.d \
./test/test4.d \
./test/test5.d \
./test/test6.d \
./test/test7.d \
./test/test8.d \
./test/test9.d \
./test/testfiles.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


