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
extern BLEStringCharacteristic stringCharacteristic;

byte received[maxSize + extraBuffer] = {};
int position = 0;
byte master_key[] = {5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
AES aes;
long long time = 0;
bool userIsMaster = true;

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

User* currentUser;

void validDataReceived(byte* receivedData, byte key[16]) {
	Serial.print("Data received: ");
	int length = getLengthOfTransmission(receivedData);
	printString(receivedData+1, length-1);
	Serial.println();
	Serial.print("Data length:");
	Serial.println(length);

	if (userIsMaster || currentUser->isAdmin) {
		adminRequest(receivedData, currentUser);
	} else {
		guestRequest(receivedData, currentUser);
	}
}

bool shouldRequestBeGranted(byte protocolRequest, long long receivedTime) {
	// Ensure request is properly authenticated.

	if (userIsMaster) {
		Serial.println("This user has used the master QR code.");
		return true;
	}

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


	// TODO currently not working
	/*
	// Is this request valid within the time constraints
	if (time > (currentUser.endTime-millis())) {
		Serial.println("The time was after the end.");
		return false;
	} else if (time < (currentUser.startTime-millis())) {
		Serial.println("The time was before the start");
		return false;
	}
	 */

	// To be a valid request, the time must be within a minute.
	if (abs(time-(receivedTime-millis())) > 60000) {
		Serial.println("You took too long to send the message.");
		return false;
	}

	return true;
}

void printLL(long long ll) {
	uint64_t xx = ll/1000000000ULL;

	if (xx >0) Serial.print((long)xx);
	Serial.print((long)(ll-xx*1000000000));
	Serial.println();
}

bool isValidDecryption(const byte* plain) {
	for (int i = 0; i < sizeOfBookends; i++) {
		if (plain[8+i] != 255) {
			return false;
		}
	}
	return true;
}

byte* tempReceived = new byte[maxSize + extraBuffer];
byte* plain = new byte[maxSize + extraBuffer];

byte * attemptDecryption() {
	memset(plain, 0, maxSize + extraBuffer);
	for (int i = 0; i < maxSize + extraBuffer; i++) {
		tempReceived[i] = received[i];
	}

	for (int i = 0; i < currentNumberOfUsers; i++) {
		Serial.print("Trying user ");
		printString((byte*) users[i].name, NAME_SIZE);
		Serial.print("Key: ");
		printString((byte*) users[i].key, KEY_SIZE);
		Serial.println();


		aes.set_key(users[i].key, KEY_SIZE);
		aes.cbc_decrypt(tempReceived+sizeOfIV, plain, 64, tempReceived);
		aes = AES();

		if (isValidDecryption(plain)) {
			currentUser = &users[i];
			Serial.println("That was the correct key.");
			return plain;
		}
		memset(plain, 0, maxSize + extraBuffer);
		for (int j = 0; j < maxSize + extraBuffer; j++) {
			tempReceived[j] = received[j];
		}
	}

	Serial.println("Trying Master key");
	aes.set_key(master_key, KEY_SIZE);
	aes.cbc_decrypt(tempReceived+sizeOfIV, plain, 64, tempReceived);
	aes = AES();

	if (isValidDecryption(plain)) {
		userIsMaster = true;
		return plain;
	}

	Serial.println("Failed decryption!");
	return nullptr;
}

/*
 * Data format (Once decrypted):
 * 8 bytes of the current time
 * 4 bytes of 255 (To check we have the correct decryption key)
 * N bytes of data where N < 2048
 * 4 bytes of 255 (To mark the end of the transmission)
 */
void receivedData() {
	Serial.println("Attempting decryption...");
	byte* plain = attemptDecryption();
	if (plain != nullptr) {
		// Grab the time from the decrypted text.
		ArrayToLL converter = {plain[0],plain[1],plain[2],plain[3], plain[4], plain[5], plain[6], plain[7]};
		long long receivedTime = converter.ll;

		byte* dataBlock = plain + extraBuffer; // Important data is past this point.
		byte protocolRequest = dataBlock[0]; // The first byte of important data.

		if (shouldRequestBeGranted(protocolRequest, receivedTime)) {
			validDataReceived(dataBlock, master_key);
		} else {
			Serial.println("Request was denied.");
			stringCharacteristic.writeValue("The request was denied.");
		}
	} else {
		Serial.println("Key is invalid.");
		stringCharacteristic.writeValue("The key is invalid.");
	}

	position = 0;
	memset(received, 0, maxSize + extraBuffer);
	userIsMaster = false;
}

void stringArrived(const String& value) {
	Serial.print("Receiving ... ");

	memcpy(received+position, value.c_str(), value.length());
	position += value.length();
}
