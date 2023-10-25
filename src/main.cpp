#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern "C"{
    void app_main();
}
void app_main() {
    printf("Can bus project...\n");
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_15, 0);
    vTaskDelay((TickType_t)30000);
    gpio_set_level(GPIO_NUM_15, 1);
    
    SPI.begin(SCK, MISO, MOSI, CS);
    
      // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
        printf("MCP2515 initialization successful! \n")
    else
        printf("Error initializing MCP2515! \n");
    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    gpio_set_direction(GPIO_NUM_16, GPIO_MODE_INPUT);
    Serial.println("MCP2515 Library Receive Example...");
    
    while(1){
    if(!digitalRead(CAN0_INT)){                         // If CAN0_INT pin is low, read receive buffer
        CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
        
        if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
            sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
        else
            sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
        Serial.print(msgString);

        if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
            sprintf(msgString, " REMOTE REQUEST FRAME");
            Serial.print(msgString);
        }else{
            for(byte i = 0; i<len; i++){
                sprintf(msgString, " 0x%.2X", rxBuf[i]);
                Serial.print(msgString);
            }
        }        
        printf("\n");
    }
}