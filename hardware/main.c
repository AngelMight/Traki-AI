	 
	 
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
		
	#include "pico/stdlib.h"
	#include "pico/binary_info.h"
	#include "pico/stdlib.h"
	#include "pico/cyw43_arch.h"

	#include "hardware/gpio.h"
	#include "hardware/adc.h"
	#include "hardware/uart.h"

	#include "lwip/pbuf.h"
	#include "lwip/udp.h"


	//#define WIFI_SSID "asdas"
	#define UDP_PORT 4444
	#define BEACON_MSG_LEN_MAX 127
	#define BEACON_TARGET "255.255.255.255"
	#define BEACON_INTERVAL_MS 1000
	#define ADC_NUM 0	
	#define ADC_PIN (26 + ADC_NUM)
	#define ADC_VREF 3.3
	#define ADC_RANGE (1 << 12)
	#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))


	void run_udp_beacon() {
		 struct udp_pcb* pcb = udp_new();

		 ip_addr_t addr;
		 ipaddr_aton(BEACON_TARGET, &addr);

		 int counter = 0;
		 
		 
		 
	  adc_init();
    adc_gpio_init( ADC_PIN);
    adc_select_input( ADC_NUM);

    uint adc_raw;
	 
		 while (true) {
			 
        adc_raw = adc_read(); // raw voltage from ADC
        printf("%.2f\n", adc_raw * ADC_CONVERT);			 
			 
			  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
			  char *req = (char *)p->payload;
			  memset(req, 0, BEACON_MSG_LEN_MAX+1);
			  snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);
			  err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
			  pbuf_free(p);
			  if (er != ERR_OK) {
					printf("Failed to send UDP packet! error=%d", er);
			  } else {
					printf("Sent packet %d\n", counter);
					counter++;
			  }


			  // Note in practice for this simple UDP transmitter,
			  // the end result for both background and poll is the same

	#if PICO_CYW43_ARCH_POLL
			  // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
			  // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
			  cyw43_arch_poll();
			  sleep_ms(BEACON_INTERVAL_MS);
	#else
			  // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
			  // is done via interrupt in the background. This sleep is just an example of some (blocking)
			  // work you might be doing.
			  sleep_ms(BEACON_INTERVAL_MS);
	#endif
		 }
		 
		 
	}




	void led_set( int val ){
		cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, val ); }

	void led_toggle( void ){
		 static int val = 0;
		 val = 1 - val;
		 led_set( val ); }


	int main() {
		
		 stdio_init_all();

		 if( cyw43_arch_init() )
			  return 1;

		 while (!stdio_usb_connected()) {  // Wait for USB CDC connection
			  sleep_ms(100);
			  led_toggle(); }

		 led_set( 1 );
		 printf( "Hello. \n" );


		 cyw43_arch_enable_sta_mode();
		 printf("Connecting to Wi-Fi...\n");
		 if (cyw43_arch_wifi_connect_timeout_ms( "Voltron", "11240167", 
			//CYW43_AUTH_WPA2_AES_PSK
			//CYW43_AUTH_WPA_TKIP_PSK
			CYW43_AUTH_WPA2_MIXED_PSK 
			, 30000)) {
			  printf("failed to connect.\n");
			  return 1;
		 } else {
			  printf("Connected.\n");
		 }
		 run_udp_beacon();
		 cyw43_arch_deinit();
		 return 0;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
