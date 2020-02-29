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
#include <ArduinoBLE.h>
#include <Wire.h>
#ifndef ARDUINO_SAMD_MKRWIFI1010
#include <HardwareSerial.h>

extern HardwareSerial Serial;
#endif

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
const int ledPin = 13; // pin to use for the LED
bool currentlyLocked = false;

void setup() {
	Serial.begin(9600);
	Wire.begin();
	motorStop(1);
	// set LED pin to output mode
	pinMode(ledPin, OUTPUT);
	// begin initialization
	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");
		while (1);
	}
	// set advertised local name and service UUID:
	BLE.setLocalName("LED");
	BLE.setAdvertisedService(ledService);
	// add the characteristic to the service
	ledService.addCharacteristic(switchCharacteristic);
	// add service
	BLE.addService(ledService);
	// set the initial value for the characeristic:
	switchCharacteristic.writeValue(0);
	// start advertising
	BLE.advertise();
	Serial.println("BLE LED Peripheral");
}

void loop() {
	// listen for BLE peripherals to connect:
	BLEDevice central = BLE.central();
	// if a central is connected to peripheral:
	if (central) {
		Serial.print("Connected to central: ");
		// print the central's MAC address:
		Serial.println(central.deviceName());

		if (currentlyLocked) {   // any value other than 0
			Serial.println("Motor Forward");
			motorForward(0, 100);
			digitalWrite(ledPin, HIGH);         // will turn the LED on
			delay(2000);
			motorStop(0);
			currentlyLocked = !currentlyLocked;
		} else {                              // a 0 value
			Serial.println(F("LED off"));
			motorBackward(0, 100);
			delay(2000);
			motorStop(0);
			digitalWrite(ledPin, LOW);          // will turn the LED off
			currentlyLocked = !currentlyLocked;
		}

		// while the central is still connected to peripheral:
		while (central.connected()) {
			// if the remote device wrote to the characteristic,
			// use the value to control the LED:
			/*
			if (switchCharacteristic.written()) {
			  if (switchCharacteristic.value()) {   // any value other than 0
				Serial.println("Motor Forward");
				motorForward(0, 100);
				digitalWrite(ledPin, HIGH);         // will turn the LED on
				delay(2000);
				motorStop(0);
			  } else {                              // a 0 value
				Serial.println(F("LED off"));
				motorBackward(0, 100);
				delay(2000);
				motorStop(0);
				digitalWrite(ledPin, LOW);          // will turn the LED off
			  }
			}
			*/
		}
		// when the central disconnects, print it out:
		Serial.print(F("Disconnected from central: "));
		Serial.println(central.address());
	}
}
