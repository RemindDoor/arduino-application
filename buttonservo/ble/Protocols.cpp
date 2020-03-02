//
// Created by Richard Mullender on 01/03/2020.
//

#include "Protocols.h"
#include "UserStorage.h"
#include "SDPArduino.h"
#include "AES.h"
#include <ArduinoBLE.h>
#include <Wire.h>
#ifndef ARDUINO_SAMD_MKRWIFI1010
#include <HardwareSerial.h>
extern HardwareSerial Serial;
#endif
extern BLEStringCharacteristic stringCharacteristic;

void sendString(const byte* message, int size) {
	String D = "";
	for (int i = 0; i < size; i++) {
//		if (message[i] == 0) {
//			D.concat("ZERO");
//		} else {
			D.concat((char) message[i]);
//		}
	}
	stringCharacteristic.writeValue(D);
	Serial.print("Sent back to phone: ");
	Serial.println(D);
}

bool ignoresTimeFrame(byte protocolNum) {
	if (protocolNum == 1) {
		return true;
	}
	return false;
}

bool currentlyLocked = false;

void unlockDoor() {
	Serial.println("Door is unlocked!");
	if (currentlyLocked) {   // any value other than 0
		Serial.println("Motor Forward");
		motorForward(0, 100);
		delay(2000);
		motorStop(0);
		currentlyLocked = !currentlyLocked;
	} else {                              // a 0 value
		Serial.println("Motor Backward");
		motorBackward(0, 100);
		delay(2000);
		motorStop(0);
		currentlyLocked = !currentlyLocked;
	}
}

void generateNewUser(byte *data) {
	// Generating a new user request. Comes with their name.
	// Sends back their authentication key.

	User* user = addUser((char *) data, 0, MAX_LONG, true);

	sendString(user->key, KEY_SIZE);
}

void generateTempGuest(byte *data) {
	long long startTime = *(data + NAME_SIZE);
	long long endTime = *(data + NAME_SIZE + 8);
	User* user = addUser((char*) data, startTime, endTime, false);
	user->temporaryKey = true;

	sendString(user->key, KEY_SIZE);
}

void sendUserList() {
	String userList = "";
	for (int i = 0; i < currentNumberOfUsers; i++) {
		for (int j = 0; j < NAME_SIZE; j++) {
			userList.concat(users[i].name[j]);
		}
		userList.concat('|');
	}
	sendString((byte*) userList.c_str(), userList.length());
}

void makeNewGuestKey(byte key[16]) {
	User* user = getUserByKey(key);

	// One of two places this happens. Change both if changing.
	byte newKey[KEY_SIZE] = {};
	for (unsigned char & i : newKey) {
		i = random(0, 255);
	}

	memcpy(user->key, newKey, KEY_SIZE);

	sendString(newKey, KEY_SIZE);
}

/*
 * Case list:
 * 0: Unlocking door request
 * 1: ADMIN - Create temporary guest account (Logs it for replacement and adds timestamps)
 *      | --- 32 bytes of name --- | | --- 8 bytes of start time --- | | --- 8 bytes of end time --- |
 * 2: ADMIN - Generate new user request (Generates new key and adds user name)
 * 3: ADMIN - Remove user request (You provide a name).
 *      | --- 32 bytes of name --- |
 * 4: ADMIN - Get all authenticated users.
 *      | --- nothing --- |
 * 5: My name change request.
 *      | --- 32 bytes of new name --- |
 * 6: ADMIN - Others change name request.
 *      | --- 32 bytes of old name --- | | --- 32 bytes of new name --- |
 * 7: Swaps out guest key for new random key and gives to guest.
 */

void guestRequest(byte *receivedData, byte key[16]) {
	// At this point the guest has been verified as being in their stay.
	byte protocolRequest = receivedData[0];
	byte *data = receivedData + 1;

	switch (protocolRequest) {
		case 0:
			// unlock door request. Can unlock immediately.
			unlockDoor();
			break;
		case 5:
			// Name change request.
			editName(key, (char*) data);
			break;
		case 7:
			makeNewGuestKey(key);
			break;
		default:
			// Error. This request was invalid.
			break;
	}
}

void adminRequest(byte *receivedData, byte key[16]) {
	byte protocolRequest = receivedData[0];
	byte *data = receivedData + 1;
	switch (protocolRequest) {
		case 1:
			// Create temporary guest account (Logs it for replacement and adds timestamps)
			generateTempGuest(data);
			break;
		case 2:
			// Generating a new user request. Comes with their name.
			// Sends back information including their authentication key.
			generateNewUser(data);
			break;
		case 3:
			// Remove user request.
			deleteUser((char*) data);
			sendUserList();
			break;
		case 4:
			// Get full user list.
			sendUserList();
			break;
		case 6:
			// Change another's name.
			editName((char*) data, (char*) data + NAME_SIZE);
			break;
		default:
			// Fall back on guest protocols.
			guestRequest(receivedData, key);
			break;
	}
}
