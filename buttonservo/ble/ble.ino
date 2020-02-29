/*
  LED
  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED.
  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.
  This example code is in the public domain.
*/
#include "SDPArduino.h"
#include "AES.h"
#include <ArduinoBLE.h>
#include <Wire.h>

#ifndef ARDUINO_SAMD_MKRWIFI1010

#include <HardwareSerial.h>

extern HardwareSerial Serial;
#endif

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEStringCharacteristic stringCharacteristic("19B10001-0001-537E-4F6C-D104768A1214", BLERead | BLEWrite, 512);
byte received[2048] = {};
int position = 0;
byte key[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
AES aes;

void loop() {
	BLE.poll();
}

void blePeripheralConnectHandler(BLEDevice central) {
	Serial.println("Receiving data ...");
}

void printString(byte* pointer, int size) {
	for(int i = 0; i < size; i++){
		Serial.print((char) pointer[i]);
	}
}

int getLengthOfTransmission(byte* pointer) {
	int size = 0;
	int count = 0;
	while (true) {
		pointer++;
		if (*pointer == 255) {
			count++;
			if (count == 8) {
				return size-7;
			}
		} else {
			count = 0;
		}
		size++;
	}
}

void blePeripheralDisconnectHandler(BLEDevice central) {
	Serial.println("Disconnected.");

	byte* plain = new byte[2048];

	aes.set_key(key, 16);
	aes.cbc_decrypt(received+16, plain, 64, received);

	Serial.println("Decrypted.");
	printString(plain, getLengthOfTransmission(plain));

	position = 0;
	memset(received, 0, 2048);
}

void stringCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
	if (stringCharacteristic.written()) {
		String value = stringCharacteristic.value();
		Serial.print("Receiving ... ");

		memcpy(received+position, value.c_str(), value.length());
		position += value.length();
	}
}

void setup() {
	Serial.begin(115200);
	Wire.begin();
	motorStop(1);

	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");
		while (1);
	}
	BLE.setLocalName("RemindDoor");
	BLE.setAdvertisedService(ledService);
	ledService.addCharacteristic(stringCharacteristic);
	BLE.addService(ledService);
	BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
	stringCharacteristic.setEventHandler(BLEWritten, stringCharacteristicWritten);
	stringCharacteristic.writeValue(""); // Ensure the characteristic is blank.
	BLE.advertise();
	Serial.println(("Bluetooth device active, waiting for connections..."));
}