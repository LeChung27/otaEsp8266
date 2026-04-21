/* The classic "blink" example
 *
 * This sample code is in the public domain.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/api.h"


#include "mbedtls/config.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"


//#define WEB_SERVER "www.howsmyssl.com"
//#define WEB_PORT "443"
//#define WEB_URL "https://www.howsmyssl.com/a/check"

#define WEB_SERVER "firchessupdate.alwaysdata.net"
#define WEB_PORT "443"
#define WEB_URL "https://firchessupdate.alwaysdata.net/projects/fir/update.php"

#define GET_REQUEST "GET "WEB_URL" HTTP/1.1\nHost: "WEB_SERVER"\nConnection: close\nUser-Agent: ESP8266-http-Update\n\n"



/* Root cert for howsmyssl.com, stored in cert.c */
//extern const char *server_root_cert;

#include "rboot-api.h"
#include "ota-tftp.h"


/*
 1 s:/C=US/O=Let's Encrypt/CN=Let's Encrypt Authority X3
   i:/O=Digital Signature Trust Co./CN=DST Root CA X3
 */
const char *server_root_cert = 
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEVzCCAj+gAwIBAgIRAKp18eYrjwoiCWbTi7/UuqEwDQYJKoZIhvcNAQELBQAw\r\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\r\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\r\n"
"WhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\r\n"
"RW5jcnlwdDELMAkGA1UEAxMCRTcwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARB6AST\r\n"
"CFh/vjcwDMCgQer+VtqEkz7JANurZxLP+U9TCeioL6sp5Z8VRvRbYk4P1INBmbef\r\n"
"QHJFHCxcSjKmwtvGBWpl/9ra8HW0QDsUaJW2qOJqceJ0ZVFT3hbUHifBM/2jgfgw\r\n"
"gfUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcD\r\n"
"ATASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSuSJ7chx1EoG/aouVgdAR4\r\n"
"wpwAgDAfBgNVHSMEGDAWgBR5tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcB\r\n"
"AQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0g\r\n"
"BAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVu\r\n"
"Y3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAjx66fDdLk5ywFn3CzA1w1qfylHUD\r\n"
"aEf0QZpXcJseddJGSfbUUOvbNR9N/QQ16K1lXl4VFyhmGXDT5Kdfcr0RvIIVrNxF\r\n"
"h4lqHtRRCP6RBRstqbZ2zURgqakn/Xip0iaQL0IdfHBZr396FgknniRYFckKORPG\r\n"
"yM3QKnd66gtMst8I5nkRQlAg/Jb+Gc3egIvuGKWboE1G89NTsN9LTDD3PLj0dUMr\r\n"
"OIuqVjLB8pEC6yk9enrlrqjXQgkLEYhXzq7dLafv5Vkig6Gl0nuuqjqfp0Q1bi1o\r\n"
"yVNAlXe6aUXw92CcghC9bNsKEO1+M52YY5+ofIXlS/SEQbvVYYBLZ5yeiglV6t3S\r\n"
"M6H+vTG0aP9YHzLn/KVOHzGQfXDP7qM5tkf+7diZe7o2fw6O7IvN6fsQXEQQj8TJ\r\n"
"UXJxv2/uJhcuy/tSDgXwHM8Uk34WNbRT7zGTGkQRX0gsbjAea/jYAoWv0ZvQRwpq\r\n"
"Pe79D/i7Cep8qWnA+7AE/3B3S/3dEEYmc0lpe1366A/6GEgk3ktr9PEoQrLChs6I\r\n"
"tu3wnNLB2euC8IKGLQFpGtOO/2/hiAKjyajaBP25w1jF0Wl8Bbqne3uZ2q1GyPFJ\r\n"
"YRmT7/OXpmOH/FVLtwS+8ng1cAmpCujPwteJZNcDG0sF2n/sc0+SQf49fdyUK0ty\r\n"
"+VUwFj9tmWxyR/M=\r\n"
"-----END CERTIFICATE-----\r\n";



/*
	//----------------howmyssl.com-----------------------
	
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFLDCCBBSgAwIBAgISBXs8Vn3CBpdmxtKPPLxraqGSMA0GCSqGSIb3DQEBCwUA\r\n"
"MDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQwwCgYDVQQD\r\n"
"EwNSMTIwHhcNMjYwMjI4MTg1MjQ1WhcNMjYwNTI5MTg1MjQ0WjAcMRowGAYDVQQD\r\n"
"ExF3d3cuaG93c215c3NsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\r\n"
"ggEBALAtq9x74z3opRoMaaIu3fijkO4A1DiPTOosv4JpvEqWwGPkjTv5oTPh+tY3\r\n"
"/lqAf4wZrgEjeYl3Cpn/XssVR1+EKm5nRf+ovnQ2jikbOz43EwYjsybXNuL8/YvG\r\n"
"uVpW1XGMYWDY3bR0g77tO+88mznfAg/LJf4xrwe8rN4TffOcaufTX5DtSrEyjqQ7\r\n"
"Cl2HEZADhrjuENkSjVpQAsSq5l/x5RLaO6BOeFZam5M1Qsys45P/WGcNpEXVAYhB\r\n"
"+CsroORrIzFG6Nk5Gbe28EYnvaMpggVnrfrQl9Dnzodt7iOWGY+0sxR5jRSTJ2Yb\r\n"
"s5jIlCo8rj73kwWfku0NRWmyuRMCAwEAAaOCAk8wggJLMA4GA1UdDwEB/wQEAwIF\r\n"
"oDATBgNVHSUEDDAKBggrBgEFBQcDATAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTo\r\n"
"weO1f2O1zSiqt+/4nRI+xS/hYjAfBgNVHSMEGDAWgBQAtSnyLY5vMeibTK14Pvrc\r\n"
"6QzR0jAzBggrBgEFBQcBAQQnMCUwIwYIKwYBBQUHMAKGF2h0dHA6Ly9yMTIuaS5s\r\n"
"ZW5jci5vcmcvME0GA1UdEQRGMESCDWhvd3NteXNzbC5jb22CDWhvd3NteXRscy5j\r\n"
"b22CEXd3dy5ob3dzbXlzc2wuY29tghF3d3cuaG93c215dGxzLmNvbTATBgNVHSAE\r\n"
"DDAKMAgGBmeBDAECATAuBgNVHR8EJzAlMCOgIaAfhh1odHRwOi8vcjEyLmMubGVu\r\n"
"Y3Iub3JnLzE5LmNybDCCAQsGCisGAQQB1nkCBAIEgfwEgfkA9wB1AMs49xWJfISh\r\n"
"RF9bwd37yW7ymlnNRwppBYWwyxTDFFjnAAABnKXOIeEAAAQDAEYwRAIgSQ7zWP0p\r\n"
"kFQKAd0wkt3gormAL/oLy/PTmSuR3A5ozzUCIGjIhHTUX9Q0WCrC8g9l+9eXxlpJ\r\n"
"xfhzTLew4UczGG/+AH4A4yON8o2iiOCq4Kzw+pDJhfC2v/XSpSewAfwcRFjEtugA\r\n"
"AAGcpc4lKwAIAAAFADQQmjkEAwBHMEUCIQCgtoLruQC4cV16hyeuSBFmiyxsBVyt\r\n"
"dCx9prnPRCWaUgIgGiXVv3bZp+O9JcehCPiOT8PD7bbtmB534zasPBr1N9AwDQYJ\r\n"
"KoZIhvcNAQELBQADggEBADN7QBkNf2JPrSaY5UrhJBDBjuqSE2EBT8qL2NifVFoU\r\n"
"Mb/0pSJiIwF8KvZqinmXXmMo6u+YyKvTQ/C0agCGtNzW5NzTxUjbFt7ONCenJjA9\r\n"
"Ng1GuFujIR6H2Z8FUJhtvnhuyDjs5M0ZX+ZD+vCVc4yvTHwxoBMu6EpsYVAVGSCy\r\n"
"yMPIu6OK6NEtx2hKYPcMpTIZUhvP42I7hyiVxsn2yDH3JZwkO2IaXLkmPKAz5/Bp\r\n"
"4O4TGoYAy7Eo9VWuoHDVxFv/Vdcw1SGWK+sqnUdp1BLVfGZle7pHGtFH0fzpevNO\r\n"
"TsLR/Vv/wd+blDFuV8Ih1RD8I67bdODYtUPUKwu3/RE=\r\n"
"-----END CERTIFICATE-----\r\n";


*/

void http_get_task(void *pvParameters);

const int gpio = 2;
uint16_t adc = 0;
uint32_t cnt = 0;
/* This task uses the high level GPIO API (esp_gpio.h) to blink an LED.
 *
 */
//rboot_config *conf;
uint8_t chunk[4096];

unsigned char buf[256];

uint8_t data_flash_test[16] = { 0xea, 0x04, 0x00, 0x40, 0x3c, 0x02, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x6d, 0x03, 0x00};
void blinkenTask(void *pvParameters)
{
    gpio_enable(gpio, GPIO_OUTPUT);
    gpio_enable(0, GPIO_INPUT);
    
    /* Doi WiFi ket noi */
    //while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
        //printf("Waiting for WiFi...\n");
        //vTaskDelay(500 / portTICK_PERIOD_MS);
    //}
    //printf("WiFi connected, got IP\n");
    
	rboot_config rboot_config = rboot_get_config();
	
	/* Validate the OTA slot parameter */
	printf("ROM hien tai la: %d \n", rboot_config.current_rom);
	printf("So luong ROM: %d \n", rboot_config.count);
	printf("Dia chi rom [0] %lu\n", rboot_config.roms[0]);
	printf("Dia chi rom [1] %lu\n", rboot_config.roms[1]);
	
    while(1) {
        gpio_write(gpio, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_write(gpio, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
	        
		
        cnt++;
        if (cnt >= 2){
			
			//uint32_t offset = 0;
			//uint32_t write_offs = 0xE2000;
			//uint32_t status_erase_sector = sdk_spi_flash_erase_sector(write_offs / SECTOR_SIZE);
			//printf("ket qua xoa sector %lu \n", status_erase_sector);
			
			
			//uint32_t first_word;
			//memcpy(chunk, data_flash_test, 16);
			
			//do {
				//memcpy(&first_word, chunk, 4);
				//sdk_spi_flash_write(write_offs+offset, &first_word, 4);
				//offset += 4;
			//} while 
			
			//uint8_t slot = 1;
			
			//uint32_t length;
			//bool valid = rboot_verify_image(rboot_config.roms[slot], &length, NULL);
			//printf("valid = %d\n", valid);
			
			//printf("SHA256 Matches. Rebooting into slot %d...\n", slot);
			//rboot_set_current_rom(slot);
			//sdk_system_restart();
			
			
			//rboot_rtc_data memrtc;
			//memrtc.next_mode = MODE_TEMP_ROM;
			//memrtc.last_mode = MODE_TEMP_ROM;
			//memrtc.last_rom = 0;
			//memrtc.temp_rom = 1;

			//rboot_set_rtc_data(&memrtc);
			//printf("RTC ROM truoc do: %d \n", memrtc.last_rom);
			//printf("RTC ROM tiep theo: %d \n", memrtc.temp_rom);
			//sdk_system_restart();
		}
        // uint32_t pin_value = gpio_read(12); 
        // printf("pin 12 status: %d\n", pin_value);

		// READ ADC
		//	adc = sdk_system_adc_read();
		//	printf("adc value : %d\n", adc);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void receivebutton(void *pvParameters)
{
	gpio_enable(0, GPIO_INPUT);
	while(1){
		unsigned char i = gpio_read(0);
		printf("satus pin 0: %d\n", i);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
//esptool.py -p /dev/ttyUSB0 --baud 460800 write_flash -fs 1m -fm dio -ff 40m 0x0 ../../bootloader/firmware_prebuilt/rboot.bin 0x1000 ../../bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/blink.bin
//esptool.py -p /dev/ttyUSB0 --baud 921600 write_flash -fs 16m -fm qio -ff 40m \
	
/* This task demonstrates an alternative way to use raw register
   operations to blink an LED.

   The step that sets the iomux register can't be automatically
   updated from the 'gpio' constant variable, so you need to change
   the line that sets IOMUX_GPIO2 if you change 'gpio'.

   There is no significant performance benefit to this way over the
   blinkenTask version, so it's probably better to use the blinkenTask
   version.

   NOTE: This task isn't enabled by default, see the commented out line in user_init.
*/
void user_init(void)
{
    uart_set_baud(0, 115200);
    
     struct sdk_station_config config = {
         .ssid = "Coffee Soc 1",
        .password = "168168168",
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    
    
    xTaskCreate(blinkenTask, "blinkenTask", 512, NULL, 2, NULL);
	//xTaskCreate(http_get_task,  "https",  3072, NULL, 1, NULL);
 
}

uint8_t buff_rev[16];
uint32_t fir_length = 1;
uint32_t header_len = 0;
char header_buf[1024];

uint8_t StatusReadHeader = 0;
uint32_t total_byte_firmware = 0;

void http_get_task(void *pvParameters)
{
    int successes = 0, failures = 0, ret;
    printf("HTTP get task starting...\n");

    uint32_t flags;

    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    printf("\n  . Seeding the random number generator...");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    (const unsigned char *) pers,
                                    strlen(pers))) != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        abort();
    }
    
	
	
    printf(" ok\n");

    /*0. Initialize certificates   */
    printf("  . Loading the CA root certificate ...");
    ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1);
    if(ret < 0)
    {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        abort();
    }
    printf(" ok (%d skipped)\n", ret);
    /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        abort();
    }
    /* 2. Setup stuff  */
    printf("  . Setting up the SSL/TLS structure...");
    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0){  
		printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret); goto exit;
    }
    
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	
	
    if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0){
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret); goto exit;
    }

    printf("Waiting for server DNS to resolve... ");
    err_t dns_err;
    ip_addr_t host_ip;
    do {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        dns_err = netconn_gethostbyname(WEB_SERVER, &host_ip);
    } while(dns_err != ERR_OK);
    printf("done.\n");

    while(1) {
		printf("[SYSTEM] Bat dau gui request ,heap = %u\n", xPortGetFreeHeapSize());
        mbedtls_net_init(&server_fd);
        printf("top of loop, free heap = %u\n", xPortGetFreeHeapSize());
        /* 1. Start the connection   */
        printf("  . Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
        if((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0){
            printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
            goto exit;
        }
        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
        /* 4. Handshake */
        printf("  . Performing the SSL/TLS handshake...");
        while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE){
                printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret); goto exit;
            }
            vTaskDelay(1);  // <<< FEED WDT
        }
        /* 5. Verify the server certificate */
        printf("  . Verifying peer X.509 certificate...");
        if((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            char vrfy_buf[512];
            printf(" failed\n");
            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
            printf("%s\n", vrfy_buf);
        }
        else
            printf(" ok\n");

        /* 3. Write the GET request */
        printf("  > Write to server:");
        int len = sprintf((char *) buf, GET_REQUEST);
        while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
        {
            if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE){
                printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret); goto exit;
            }
        }
        len = ret;
        printf(" %d bytes written\n\n%s", len, (char *) buf);
        /* 7. Read the HTTP response    */
        printf("  < Read from server: \n");
        StatusReadHeader = 1; // need read header response
        do
        {
            //len = sizeof(buf) - 1;
            memset(buf, 0, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf));

            if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE){
                vTaskDelay(1);   // <<< importance
                continue;
			}
            if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {ret = 0; break; }
            if(ret <= 0){
                printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
                break;
            }
            if (ret > 0){
				if (StatusReadHeader)
				{
					printf("[HTTP] Bat dau xu ly header \n");
					printf("[DEBUG] header_len: %lu \n", header_len);
					uint32_t header_size = 0;
					uint32_t body_len = 0;
					
					//if (header_len + ret >= sizeof(header_buf)) {
						//printf("Header too large\n");
						//break;
					//}
					memcpy(header_buf + header_len, buf, ret);
					//header_buf[header_len] = '\0';
					header_len += ret;
					printf("[DEBUG] header_len: %lu \n", header_len);
					//printf("[DEBUG] sizeof(header_buf): %lu \n", sizeof(header_buf) ); // 1024 da debug
					
					char *end = strstr(header_buf, "\r\n\r\n");
					if (end) {
						header_size = end - header_buf + 4;
						printf("[DEBUG] header_size: %lu \n", header_size);
						// parse Content-Length
						char *cl = strstr(header_buf, "Content-Length:");
						if (cl) {
							fir_length = strtoul(cl + 15, NULL, 10);
						}
						body_len = header_len - header_size;
						StatusReadHeader = 0;
						header_len = 0;
						memset(header_buf, 0, sizeof(header_buf));
						//dia chi bat dau cua firmware la header_buf + header_size, do dai la body_len
						total_byte_firmware = body_len;
						printf("[DEBUG] fir_length: %lu \n", fir_length);
						printf("[DEBUG] body_len: %lu \n", body_len);
						printf("[DEBUG] total_byte_firmware: %lu \n", total_byte_firmware);
						//printf("\n%s",(char *) header_buf);
					} 
				} else { //xu ly phan body firmware
					
					total_byte_firmware += ret;
					//printf("\n%s",(char *) buf);
					//printf("[SYSTEM]ret = %d, total_byte_firmware = %lu\n", ret, total_byte_firmware);
					//printf("[SYSTEM] heap = %u\n", xPortGetFreeHeapSize());
					
					//if (total_byte_firmware > (fir_length / 2)) 
						//printf("[DEBUG] Da Doc duoc 1 nua firmware\n");
						
					//if (total_byte_firmware >= fir_length) { printf("[DEBUG] total_byte_firmware: %lu \n", total_byte_firmware); break;}
				}
					// phần dư sau header = body bắt đầu
				//printf("So luong byte file firmware: %lu \n", fir_length);
			}
            vTaskDelay(1);       // <<< FEED WDT
        } while(1);

        mbedtls_ssl_close_notify(&ssl);
    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if(ret != 0)
        {
            char error_buf[100];
            mbedtls_strerror(ret, error_buf, 100);
            printf("\n\nLast error was: %d - %s\n\n", ret, error_buf);
            failures++;
        } else {
            successes++;
        }
		printf("[SYSTEM]Sau khi gui request ,heap = %u\n", xPortGetFreeHeapSize());
        printf("HTTPS stack watermark = %lu\n", uxTaskGetStackHighWaterMark(NULL));
        printf("[HTTP] successes = %d, failures = %d \n", successes, failures);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
        printf("\nStarting again!\n");
    }
}

