	 
	 
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
		
	#include "pico/stdlib.h"
	#include "pico/binary_info.h"
	#include "pico/stdlib.h"
	#include "pico/cyw43_arch.h"

	#include "hardware/gpio.h"
	#include "hardware/adc.h"
	#include "hardware/dma.h"
	#include "hardware/uart.h"

	#include "lwip/pbuf.h"
	#include "lwip/udp.h"


	#define WIFI_SSID "Voltron"
	#define WIFI_PASS "11240167"

	#define SAMPLE_RATE 44100
	#define NUM_CHANNELS 2
	#define BUFFER_SIZE 1024


	// -- UDP --------------------------------------------------------------

	#define UDP_TARGET "255.255.255.255"
	#define UDP_PORT 4444


	int send_udp( void *buf ){

		ip_addr_t addr;
		ipaddr_aton( UDP_TARGET, &addr );
		struct udp_pcb* pcb = udp_new();		
		struct pbuf *p = pbuf_alloc( PBUF_TRANSPORT, BUFFER_SIZE, PBUF_RAM );

		memcpy( p->payload, buf, BUFFER_SIZE );			
		
		cyw43_arch_lwip_begin();
		err_t er = udp_sendto( pcb, p, &addr, UDP_PORT );
		cyw43_arch_lwip_end();
		
		pbuf_free( p );
		udp_remove( pcb );

		if( er != ERR_OK )
			printf("Failed to send UDP packet! error=%d", er);
		else
			printf( "." );

	}

	// -- ADC DMA --------------------------------------------------------------

	#define ADC_NUM 0	
	#define ADC_PIN (26 + ADC_NUM)
	#define ADC_VREF 3.3
	#define ADC_RANGE (1 << 12)
	#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))
	
	uint16_t buffer0[BUFFER_SIZE/2];
	uint16_t buffer1[BUFFER_SIZE/2];

	int dma_chan_a, dma_chan_b;

	void dma_handler() {
		 // Проверка кой канал е завършил и обработка на съответния буфер
		 if( dma_hw->ints0 & (1u << dma_chan_a) ){
			  dma_hw->ints0 = (1u << dma_chan_a); // Изчистване на прекъсването
			  dma_channel_set_write_addr( dma_chan_a, buffer0, false );
			  send_udp( buffer0 );
		 } else if ( dma_hw->ints0 & (1u << dma_chan_b) ){
			  dma_hw->ints0 = (1u << dma_chan_b);
			  dma_channel_set_write_addr( dma_chan_b, buffer1, false );
			  send_udp( buffer1 );
		 }
	}

	void setup_adc_dma() {
		 adc_init();
		 adc_gpio_init( 26 ); // ADC0
		 adc_gpio_init( 27 ); // ADC1
		 adc_select_input( 0 );
		 adc_set_round_robin( 0x03 ); // Семплиране на ADC0 и ADC1 последователно
		 adc_fifo_setup( true, true, 1, false, false );
		 adc_set_clkdiv( 48000000.0f / (SAMPLE_RATE * NUM_CHANNELS) - 1.0f ); // Делител за 44.1kHz стерео (общо 88.2kHz)

		 dma_chan_a = dma_claim_unused_channel( true );
		 dma_chan_b = dma_claim_unused_channel( true );

		 // Конфигурация на Канал A
		 dma_channel_config cfg_a = dma_channel_get_default_config( dma_chan_a );
		 channel_config_set_transfer_data_size( &cfg_a, DMA_SIZE_16 );
		 channel_config_set_read_increment( &cfg_a, false );
		 channel_config_set_write_increment( &cfg_a, true );
		 channel_config_set_dreq( &cfg_a, DREQ_ADC );
		 channel_config_set_chain_to( &cfg_a, dma_chan_b ); // Прехвърли към B след края
		 dma_channel_configure( dma_chan_a, &cfg_a, buffer0, &adc_hw->fifo, BUFFER_SIZE/2, false );

		 // Конфигурация на Канал B (аналогично)
		 dma_channel_config cfg_b = dma_channel_get_default_config( dma_chan_b );
		 channel_config_set_transfer_data_size( &cfg_b, DMA_SIZE_16 );
		 channel_config_set_read_increment( &cfg_b, false );
		 channel_config_set_write_increment( &cfg_b, true );
		 channel_config_set_dreq( &cfg_b, DREQ_ADC );
		 channel_config_set_chain_to( &cfg_b, dma_chan_a );
		 dma_channel_configure( dma_chan_b, &cfg_b, buffer1, &adc_hw->fifo, BUFFER_SIZE/2, false );

		 // Настройка на прекъсвания
		 dma_channel_set_irq0_enabled( dma_chan_a, true );
		 dma_channel_set_irq0_enabled( dma_chan_b, true );
		 irq_set_exclusive_handler( DMA_IRQ_0, dma_handler );
		 irq_set_enabled( DMA_IRQ_0, true );

		 adc_run( true );
		 dma_channel_start( dma_chan_a );
		 dma_channel_start( dma_chan_b ); // ?
	}


	// -- LED -------------------------------------------------------------------

	void led_set( int val ){
		cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, val ); }

	void led_toggle( void ){
		 static int val = 0;
		 val = 1 - val;
		 led_set( val ); }


	// -- MAIN ------------------------------------------------------------------
	
	int main() {

		stdio_init_all();
		cyw43_arch_init();

		/*
		 while (!stdio_usb_connected()) {  // Wait for USB CDC connection
			  sleep_ms(100);
			  led_toggle(); }
	   */

		led_set( 1 );
		printf( "Hello. \n" );
		sleep_ms(500);
		led_set( 0 );

		cyw43_arch_enable_sta_mode();
		printf("Connecting to Wi-Fi...\n");

		if( cyw43_arch_wifi_connect_timeout_ms( WIFI_SSID, WIFI_PASS,
			//CYW43_AUTH_WPA2_AES_PSK
			//CYW43_AUTH_WPA_TKIP_PSK
			CYW43_AUTH_WPA2_MIXED_PSK 
			, 30000) ){
				
		  printf("failed to connect.\n");
		  return 1;
		  
		} else {
			
		  led_set( 1 );
		  printf("Connected.\n");

		}

		setup_adc_dma();

		while( true )
			sleep_ms( 10 );
			//__wfi();

		cyw43_arch_deinit(); // unreachable
		return 0;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
