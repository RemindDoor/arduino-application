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
		D.concat((char) message[i]);
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

void unlockDoor() {
	Serial.println("Door is unlocked!");
}

void generateNewUser(byte *data) {
	// Generating a new user request. Comes with their name.
	// Sends back their authentication key.

	User user = addUser((char *) data, 0, MAX_LONG);

	sendString(user.key, KEY_SIZE);
}

void guestUnlockDoor(byte *data) {

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

/*
 * Case list:
 * 0: ADMIN - Unlocking door request admin
 * 1: Unlocking door request guest
 * 2: ADMIN - Generate new user request (Generates new key and adds user name)
 *      | --- 32 bytes of name --- |
 * 3: ADMIN - Remove user request (You provide a name).
 *      | --- 32 bytes of name --- |
 * 4: ADMIN - Get all authenticated users.
 *      | --- nothing --- |
 * 5: My name change request.
 *      | --- 32 bytes of new name --- |
 * 6: ADMIN - Others change name request.
 *      | --- 32 bytes of old name --- | | --- 32 bytes of new name --- |
 */

void guestRequest(byte *receivedData, byte key[16]) {
	// At this point the guest has been verified as being in their stay.
	byte protocolRequest = receivedData[0];
	byte *data = receivedData + 1;

	switch (protocolRequest) {
		case 1:
			// Guest unlock door request.
			// Only works if the authentication key is not already in the database.
			// Sends back information including the guest's name, the duration of their stay and their authentication key.
			// Also adds the user to the database.
			guestUnlockDoor(data);
			break;
		case 5:
			// Name change request.
			editName(key, (char*) data);
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
		case 0:
			// Admin unlock door request. Can unlock immediately.
			unlockDoor();
			break;
		case 2:
			// Generating a new user request. Comes with their name.
			// Sends back information including their authentication key.
			generateNewUser(data);
			break;
		case 3:
			// Remove user request.
			deleteUser((char*) data);
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
