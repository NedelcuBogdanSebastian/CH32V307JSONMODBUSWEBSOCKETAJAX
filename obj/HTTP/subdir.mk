################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HTTP/HTTPS.c 

OBJS += \
./HTTP/HTTPS.o 

C_DEPS += \
./HTTP/HTTPS.d 


# Each subdirectory must supply rules for building sources it contributes
HTTP/%.o: ../HTTP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\NetLib" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\HTTP" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\CRC16" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\Core" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\Debug" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\Peripheral\inc" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\User" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\sha1" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\base64" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\websocket" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\Modbus" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


