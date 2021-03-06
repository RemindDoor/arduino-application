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
#include "Decryption.h"
#include <ArduinoBLE.h>
#include <Wire.h>

#ifndef ARDUINO_SAMD_MKRWIFI1010

#include <HardwareSerial.h>

extern HardwareSerial Serial;
#endif

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEStringCharacteristic stringCharacteristic("19B10001-0001-537E-4F6C-D104768A1214", BLERead | BLEWrite, 512);
unsigned long timeFirstConnected = 0;
bool amConnected = false;

void loop() {
	BLE.poll();

	if (millis() - timeFirstConnected > 5000L && amConnected) {
		BLE.disconnect();
		Serial.println("Force disconnected.");
		amConnected = false;
	}
}

void blePeripheralConnectHandler(BLEDevice central) {
	Serial.println("Connected.");
	timeFirstConnected = millis();
	amConnected = true;
}

void blePeripheralDisconnectHandler(BLEDevice central) {
	Serial.println("Disconnected.");
	amConnected = false;
}

void stringCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
	String string = stringCharacteristic.value();
	if (string.length() == 0) {
		Serial.println("Received data.");
		receivedData();
	} else {
		stringArrived(string);
	}
}

void setup() {
	Serial.begin(115200);
	Wire.begin();
	motorStop(1);
	randomSeed(analogRead(0));

	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");
		while (true);
	}
	BLE.setLocalName("RemindDoor");
	BLE.setDeviceName("RemindDoor");
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