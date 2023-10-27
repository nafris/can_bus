#include <stdio.h>
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

gpio_num_t CAN0_INT = GPIO_NUM_18;
gpio_num_t CAN0_CS = GPIO_NUM_19;
gpio_num_t SPI_SCK = GPIO_NUM_17;
gpio_num_t SPI_MISO = GPIO_NUM_16;
gpio_num_t SPI_MOSI = GPIO_NUM_15;

MCP_CAN CAN0(&spi2, CAN0_CS); 

extern "C"{
    void app_main();
}
void app_main() {
    
    printf("Can bus project...\n");
    
    //resetting MCP with a 1second pulse
    //don't have hardware support for this yet
    //gpio_set_direction(CAN0_INT, GPIO_MODE_OUTPUT);
    //gpio_set_level(GPIO_NUM_15, 0);
    //vTaskDelay((TickType_t)30000);
    //gpio_set_level(GPIO_NUM_15, 1);
    

    //spi settings should double this:
    //mcpSPI->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    //frequency = 1MHz x
    //dataOrder = MSBFIRST x
    ///datamode = SPI_MODE0 x
    spi_bus_config_t buscfg={
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    esp_err_t ret;
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    //SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CAN0_CS);
    ESP_ERROR_CHECK(ret);
    spi_device_interface_config_t devcfg={
        //.command_bits = NULL,
        //.address_bits = NULL,
        //.dummy_bits = NULL,
        .mode = 0,                  //SPI mode 0
        //.clock_source = SPI_CLK_SRC_DEFAULT,
        //.duty_cycle_pos = NULL,
        //.cs_ena_pretrans = NULL,
        //.cs_ena_posttrans = NULL,
        .clock_speed_hz = 1000000,  // 1 MHz
        //.input_delay_ns = NULL,
        .spics_io_num = CAN0_CS,
        //.flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi2));
    
    printf("MCP2515 Library Receive Example...\n");
    while(1){
    printf("MCP2515 Library Receive Example...\n");
    vTaskDelay((TickType_t)100);
    
    }

    
      // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
        printf("MCP2515 initialization successful! \n");
    else
        printf("Error initializing MCP2515! \n");
    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    gpio_set_direction(CAN0_INT, GPIO_MODE_INPUT);
    printf("MCP2515 Library Receive Example...\n");
    
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