################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include websocket/subdir.mk
-include sha1/subdir.mk
-include base64/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include NetLib/subdir.mk
-include Modbus/subdir.mk
-include HTTP/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include CRC16/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
CH32V307JSONMODBUSWEBSOCKETAJAX.hex \

SECONDARY_LIST += \
CH32V307JSONMODBUSWEBSOCKETAJAX.lst \

SECONDARY_SIZE += \
CH32V307JSONMODBUSWEBSOCKETAJAX.siz \


# All Target
all: CH32V307JSONMODBUSWEBSOCKETAJAX.elf secondary-outputs

# Tool invocations
CH32V307JSONMODBUSWEBSOCKETAJAX.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU RISC-V Cross C Linker'
	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"C:\Users\Bogdan\mrs_community_workspace3\CH32V307JSONMODBUSWEBSOCKETAJAX\NetLib" -Wl,-Map,"CH32V307JSONMODBUSWEBSOCKETAJAX.map" --specs=nano.specs --specs=nosys.specs -o "CH32V307JSONMODBUSWEBSOCKETAJAX.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

CH32V307JSONMODBUSWEBSOCKETAJAX.hex: CH32V307JSONMODBUSWEBSOCKETAJAX.elf
	@echo 'Invoking: GNU RISC-V Cross Create Flash Image'
	riscv-none-embed-objcopy -O ihex "CH32V307JSONMODBUSWEBSOCKETAJAX.elf"  "CH32V307JSONMODBUSWEBSOCKETAJAX.hex"
	@echo 'Finished building: $@'
	@echo ' '

CH32V307JSONMODBUSWEBSOCKETAJAX.lst: CH32V307JSONMODBUSWEBSOCKETAJAX.elf
	@echo 'Invoking: GNU RISC-V Cross Create Listing'
	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "CH32V307JSONMODBUSWEBSOCKETAJAX.elf" > "CH32V307JSONMODBUSWEBSOCKETAJAX.lst"
	@echo 'Finished building: $@'
	@echo ' '

CH32V307JSONMODBUSWEBSOCKETAJAX.siz: CH32V307JSONMODBUSWEBSOCKETAJAX.elf
	@echo 'Invoking: GNU RISC-V Cross Print Size'
	riscv-none-embed-size --format=berkeley "CH32V307JSONMODBUSWEBSOCKETAJAX.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) CH32V307JSONMODBUSWEBSOCKETAJAX.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
