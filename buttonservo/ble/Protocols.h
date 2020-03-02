//
// Created by Richard Mullender on 01/03/2020.
//

#ifndef BLE_PROTOCOLS_H
#define BLE_PROTOCOLS_H
#include <ArduinoBLE.h>

void guestRequest(byte* receivedData, void* user);
void adminRequest(byte* receivedData, void* user);
bool ignoresTimeFrame(byte protocolNum);

#endif //BLE_PROTOCOLS_H
