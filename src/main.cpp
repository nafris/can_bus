#include <stdio.h>
#include <string.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];    

spi_device_handle_t spi2;

gpio_num_t CAN0_INT = GPIO_NUM_46;
gpio_num_t CAN0_CS = GPIO_NUM_4;
gpio_num_t SPI_SCK = GPIO_NUM_8;
gpio_num_t SPI_MISO = GPIO_NUM_6;
gpio_num_t SPI_MOSI = GPIO_NUM_7;

MCP_CAN CAN0(&spi2, CAN0_CS); 


esp_err_t init_spi(){
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.mosi_io_num = SPI_MOSI;
    buscfg.miso_io_num = SPI_MISO;
    buscfg.sclk_io_num = SPI_SCK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 8;
    return spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
} 
esp_err_t add_spi_device(){
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.mode = 0;
    devcfg.clock_speed_hz = 10000000;
    //devcfg.spics_io_num = CAN0_CS;
    devcfg.spics_io_num = -1;
    devcfg.queue_size = 1;
    return spi_bus_add_device(SPI2_HOST, &devcfg, &spi2);
}

extern "C"{
    void app_main();
}
void app_main() {
    
    printf("Can bus project...\n"); 
    ESP_ERROR_CHECK(init_spi());
    ESP_ERROR_CHECK(add_spi_device());

    printf("MCP2515 Library Receive Example...\n");
    
    if(CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) == CAN_OK)
        printf("MCP2515 initialization successful! \n");
    else
        printf("Error initializing MCP2515! \n");
    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    gpio_set_direction(CAN0_INT, GPIO_MODE_INPUT);
    
    while(1){
        if(!gpio_get_level(CAN0_INT)){                         // If CAN0_INT pin is low, read receive buffer
            CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
            
            if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
                printf("Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
            else
                printf("Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);

            if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
                printf(" REMOTE REQUEST FRAME");
            }else{
                for(uint8_t i = 0; i < len; i++){
                    printf(" 0x%.2X", rxBuf[i]);
                }
            }        
            printf("\n");
        }
    }
}