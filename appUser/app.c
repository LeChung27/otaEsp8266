/* The classic "blink" example
 *
 * This sample code is in the public domain.
 */
#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "rboot-api.h"

const int gpio = 2;
uint16_t adc = 0;

void blinkenTask(void *pvParameters)
{

    gpio_enable(gpio, GPIO_OUTPUT);
    gpio_enable(0, 0);
    
	printf("[SYSTEM ]Day la application user \n");
	rboot_config rboot_config = rboot_get_config();
	uint8_t num_rom_current = rboot_config.current_rom;
	/* Validate the OTA slot parameter */
	printf("ROM hien tai la: %d \n", num_rom_current);
	printf("Dia chi rom hien tai la 0x%08X\n", rboot_config.roms[num_rom_current]);
	
    while(1) {
        gpio_write(gpio, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_write(gpio, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
		// READ ADC
		adc = sdk_system_adc_read();
		printf("adc value : %d\n", adc);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		unsigned char i = gpio_read(0);
		printf("satus pin 0: %d\n", i);
		vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void user_init(void)
{
    uart_set_baud(0, 115200);
    xTaskCreate(blinkenTask, "blinkenTask", 256, NULL, 2, NULL);
	//xTaskCreate(receivebutton, "receivebutton", 256, NULL, 3, NULL);
}
