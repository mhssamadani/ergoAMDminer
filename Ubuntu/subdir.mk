################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../conversion.cc \
../cryptography.cc \
../definitions.cc \
../easylogging++.cc \
../processing.cc \
../request.cc 

CPP_SRCS += \
../ErgoOpenCL.cpp \
../clMining.cpp \
../clPreHash.cpp \
../cl_warpper.cpp \
../ergoAutolykos.cpp 

C_SRCS += \
../jsmn.c 

CC_DEPS += \
./conversion.d \
./cryptography.d \
./definitions.d \
./easylogging++.d \
./processing.d \
./request.d 

OBJS += \
./ErgoOpenCL.o \
./clMining.o \
./clPreHash.o \
./cl_warpper.o \
./conversion.o \
./cryptography.o \
./definitions.o \
./easylogging++.o \
./ergoAutolykos.o \
./jsmn.o \
./processing.o \
./request.o 

CPP_DEPS += \
./ErgoOpenCL.d \
./clMining.d \
./clPreHash.d \
./cl_warpper.d \
./ergoAutolykos.d 

C_DEPS += \
./jsmn.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/opt/AMDAPPSDK-3.0/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/opt/AMDAPPSDK-3.0/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/opt/AMDAPPSDK-3.0/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


