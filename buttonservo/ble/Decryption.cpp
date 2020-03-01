//
// Created by Richard Mullender on 29/02/2020.
//

#include "SDPArduino.h"
#include "AES.h"
#include "Protocols.h"
#include "UserStorage.h"
#include <ArduinoBLE.h>
#include <Wire.h>
#include "Decryption.h"
#ifndef ARDUINO_SAMD_MKRWIFI1010
#include <HardwareSerial.h>

extern HardwareSerial Serial;
#endif

const int maxSize = 2048;
const int sizeOfBookends = 4;
const int extraBuffer = sizeOfBookends + 8; // The 8 for the long long time.
const int sizeOfIV = 16;

byte received[maxSize + extraBuffer] = {};
int position = 0;
byte master_key[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
AES aes;
long long time = 0;

void printString(byte* pointer, int size) {
	for(int i = 0; i < size; i++){
		Serial.print((char) pointer[i]);
	}
}

union ArrayToLL {
	byte array[8];
	uint64_t ll;
};

int getLengthOfTransmission(byte* pointer) {
	int size = 0;
	int count = 0;
	while (true) {
		pointer++;
		if (*pointer == 255) {
			count++;
			if (count == sizeOfBookends) {
				return size-sizeOfBookends+1;
			}
		} else {
			count = 0;
		}
		size++;
	}
}


void validDataReceived(byte* receivedData, byte key[16]) {
	Serial.print("Data received: ");
	printString(receivedData+1, getLengthOfTransmission(receivedData));
	Serial.println();

	// We'll assume everyone is an admin for now.
	adminRequest(receivedData, key);
}

bool shouldRequestBeGranted(byte protocolRequest, long long receivedTime) {
	// Ensure request is properly authenticated.

	// If this is a guest request
	if (ignoresTimeFrame(protocolRequest)) {
		return true;
	}

	// If the time's not been set yet we can't doubt the request.
	// TODO potential security issue.
	if (time == 0) {
		time = receivedTime-millis();
		return true;
	}

	// To be a valid request, the time must be within a minute.
	if (abs(time-(receivedTime-millis())) < 60000) {
		return true;
	}

	// Invalid request.
	return false;
}

/*
 * Data format (Once decrypted):
 * 8 bytes of the current time
 * 4 bytes of 255 (To check we have the correct decryption key)
 * N bytes of data where N < 2048
 * 4 bytes of 255 (To mark the end of the transmission)
 */
void receivedData() {
	byte* plain = new byte[maxSize + extraBuffer];

	for (int i = 0; i < currentNumberOfUsers; i++) {

	}
	aes.set_key(master_key, sizeOfIV);
	aes.cbc_decrypt(received+sizeOfIV, plain, 64, received);

	Serial.println("Decrypted.");

	// Grab the time from the decrypted text.
	ArrayToLL converter = {plain[0],plain[1],plain[2],plain[3], plain[4], plain[5], plain[6], plain[7]};
	long long receivedTime = converter.ll;

	byte* dataBlock = plain + extraBuffer; // Important data is past this point.
	byte protocolRequest = dataBlock[0]; // The first byte of important data.

	if (shouldRequestBeGranted(protocolRequest, receivedTime)) {
		validDataReceived(dataBlock, master_key);
	}

	position = 0;
	memset(received, 0, maxSize + extraBuffer);
}

void stringArrived(const String& value) {
	Serial.print("Receiving ... ");

	memcpy(received+position, value.c_str(), value.length());
	position += value.length();
}
