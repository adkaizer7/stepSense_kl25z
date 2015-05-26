/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "BLEDevice.h"

#define NEED_CONSOLE_OUTPUT 1 /* Set this if you need debug messages on the console;
* it will have an impact on code-size and power consumption. */

#if NEED_CONSOLE_OUTPUT
#define DEBUG(...) { printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */


BLEDevice  ble;
DigitalOut led1(LED1);
DigitalOut led2(LED2);

#define TIMER 1
#define PULSE_WIDTH 100
#define PAYLOAD_SIZE 32


#if TIMER
//Creating timer Instance
Ticker ticker;
long int timerCount = 0;
int threshold = 0;
DigitalOut pwmOutLeft(P0_30);
DigitalOut pwmOutRight(P0_29);
DigitalOut lowBattery(P0_28);
//DigitalOut pwmOutRight(P0_29);

#endif

const uint8_t BUTTON_UUID[LENGTH_OF_LONG_UUID] = {
    0x7a, 0x77, 0xbe, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
    0xa9, 0x5e, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

const uint8_t XBEE_UUID[LENGTH_OF_LONG_UUID] = {
    0x7a, 0x77, 0xbe, 0x21, 0x5b, 0x0e, 0x12, 0xe5,
    0xa9, 0x5e, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};
const uint8_t TEST_SERVICE_UUID[LENGTH_OF_LONG_UUID] = {
    0xb0, 0xbb, 0x58, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
    0x93, 0xee, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

const static char DEVICE_NAME[] = "stepSense";


Serial kl25z(P0_9,P0_11);
AnalogIn battery_adc(P0_1);

#define DEBUG_LED 0
#define START_BYTES_LEN 5
#define SAMPLE_BYTE 1
#define LEFT 0x05
#define RIGHT 0x01
#define COUNT_OUT_TIME_RIGHT 60000
#define COUNT_OUT_TIME_LEFT 60000
uint8_t startBytes[START_BYTES_LEN] = { 0x7e, 0x00, 0x0e, 0x83, 0x00};



void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    ble.startAdvertising(); // restart advertising
}



int main(void)
{
    // button initialization
    led1 = 0;
    led2 = 0;
    unsigned int val = 0;
    uint8_t valueRec[12] = {0};
    uint8_t *valueRecLeft = &valueRec[0];
    uint8_t *valueRecRight = &valueRec[6];
    int count = 0;
#if TIMER
    //ticker.attach(periodicCallback, 0.01f);
#endif
    
    // just a simple service example
    // o xbee characteristics, you can read and get notified
    
    
    GattCharacteristic xbee_characteristics(
                                            XBEE_UUID, (uint8_t *)valueRec, sizeof(valueRec), sizeof(valueRec),
                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    
    const uint8_t TEST_SERVICE_UUID[LENGTH_OF_LONG_UUID] = {
        0xb0, 0xbb, 0x58, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
        0x93, 0xee, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b};
    GattCharacteristic *charTable[] = {&xbee_characteristics};
    GattService testService(TEST_SERVICE_UUID, charTable,
                            sizeof(charTable) / sizeof(GattCharacteristic *));
    
    // BLE setup, mainly we add service and callbacks
    ble.init();
    ble.addService(testService);
    ble.onDisconnection(disconnectionCallback);
    
    // setup advertising
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                     GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(1600); /* 1000ms; in multiples of 0.625ms. */
    ble.startAdvertising();
    bool update = true;
    uint8_t addr;
    uint8_t curCh1;
    uint16_t valueLeft, valueRight;
    int state = 0;
    uint32_t countOutLeft = 0;
    uint32_t countOutRight = 0;
    while (true) {
        if (battery_adc < 0.5){
            lowBattery = 1;
        }
        else{
            lowBattery = 0;
        }
        if (kl25z.readable()){
            curCh1 = (uint8_t)kl25z.getc();
            if (state < START_BYTES_LEN)
            {
                if (curCh1 == startBytes[state]){
                    state++;
                    update = 0;
                }
                else
                {
                    state = 0;
                    update = 0;
                }
                
            }
            else
            {
                switch(state){
                        
                    case 5: addr = curCh1;break;
                    case 6: break;
                    case 7: break;
                    case 8: if (curCh1 != SAMPLE_BYTE){
                        //Further error checking in the parser
                        // If not sample byte, then we have to sart parsing again
                        state = 0;
                        update = 0;
                    };break;
                    case 9:break;
                    case 10:break;
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    {
                        
                        if (addr == LEFT){
                            valueRecLeft[state - 11] = curCh1;
                            
                        }
                        else if(addr == RIGHT){
                            valueRecRight[state - 11] = curCh1;
                            
                        }
                        else{
                            //Address recevied is neither left nor right. Error. Start parsing again.
                            state = 0;
                            update = 0;
                        }
                        
                    }break;
                    case 17: update = 1;break;
                    default : {
                        state = 0;
                        update = 0;
                    };break;
                        
                }
                state++;
                if(state >= 16){
                    valueLeft = valueRecLeft[2]<<8 + valueRecLeft[3];
                    //valueLeft++;
                    valueRight = valueRecRight[2]<<8 + valueRecRight[3];
                    ble.updateCharacteristicValue(xbee_characteristics.getValueAttribute().getHandle(),
                                                  (uint8_t*)valueRec, sizeof(valueRec));
                    if((valueRec[2] > 0x00) || (valueRec[3] > 0x00)){
                        led1 = 1;
                        pwmOutLeft = 1;
                    }
                    else{
                        led1 = 0;
                        pwmOutLeft = 0;
                    }
                    
                    if((valueRec[6] > 0x00))
                    {
                        led2 = 1;
                        pwmOutRight = 1;
                    }
                    
                    else{
                        led2 = 0;
                        pwmOutRight = 0;
                    }
                    
                }
            }
        }
    }
}

int main1()
{
    led1 = 1;
    pwmOutLeft = 1;
    while(1){
        
    }
    return 0;
}
