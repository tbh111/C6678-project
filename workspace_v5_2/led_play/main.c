/*
 * main.c
 */
#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ti\platform\platform.h"
#include "ti\platform\resource_mgr.h"

// OSAL functions for Platform Library
uint8_t *Osal_platformMalloc(uint32_t num_bytes, uint32_t alignment) {
	return malloc(num_bytes);
}

void Osal_platformFree(uint8_t *dataPtr, uint32_t num_bytes) {
	// free up the memory
	if(dataPtr) {
		free(dataPtr);
	}
}

void Osal_platformSpiCsEnter(void) {
	// get the hardware semaphore
	// acquire multi-core CPPI synchronization lock
	while((CSL_semAcquireDirect (PLATFORM_SPI_HW_SEM)) == 0);
	return;
}

void Osal_platformSpiCsExit(void) {
	// release the hardware semaphore
	// release multi-core lock
	CSL_semReleaseSemaphore (PLATFORM_SPI_HW_SEM);
	return;
}

void main(void) {
	platform_init_flags init_flags;
	platform_init_config init_config;
	platform_info p_info;
	uint32_t led_no = 0;
	char message[] = "\r\nHello World......\r\n";
	uint32_t length = strlen((char*)message);
	uint32_t i;

	// initialize platform with default values
	memset(&init_flags, 0x01, sizeof(platform_init_flags));
	memset(&init_config, 0, sizeof(platform_init_config));
	if(platform_init(&init_flags, &init_config) != Platform_EOK) {
		return;
	}

	platform_uart_init();
	platform_uart_set_baudrate(115200);
	platform_get_info(&p_info);

	// write to the UART
	for(i = 0; i < length; i++) {
		if(platform_uart_write(message[i] != Platform_EOK)) {
			return;
		}
	}
	
	// led play forever
	while(1) {
		platform_led(led_no, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
		platform_delay(30000);
		platform_led(led_no, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
		led_no = (++led_no) % p_info.led[PLATFORM_USER_LED_CLASS].count;
	}
}
