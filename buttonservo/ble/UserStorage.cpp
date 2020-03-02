//
// Created by Richard Mullender on 01/03/2020.
//

#include <Arduino.h>
#include "UserStorage.h"
#ifndef ARDUINO_SAMD_MKRWIFI1010
#include <HardwareSerial.h>

extern HardwareSerial Serial;
#endif

User users[NUM_USERS] = {};
int currentNumberOfUsers = 0;

User *getUserByName(const char *name) {
	for (int i = 0; i < currentNumberOfUsers; i++) {
		if (memcmp(name, users[i].name, NAME_SIZE) == 0) {
			return users + i;
		}
	}
	return nullptr;
}

User *getUserByKey(const byte *key) {
	for (int i = 0; i < currentNumberOfUsers; i++) {
		if (memcmp(key, users[i].key, KEY_SIZE) == 0) {
			return users + i;
		}
	}
	return nullptr;
}

void editName(const byte *key, const char *newName) {
	User* user = getUserByKey(key);
	memcpy(user->name, newName, NAME_SIZE);
}

void editName(const char *oldName, const char *newName) {
	User* user = getUserByName(oldName);
	memcpy(user->name, newName, NAME_SIZE);
}

void deleteUser(const char *name) {
	User* user = getUserByName(name);

	memcpy(user, &users[currentNumberOfUsers-1], sizeof(User));
	memset(&users[currentNumberOfUsers-1], 0, sizeof(User));

	currentNumberOfUsers--;
}

User addUser(const char *name, long long startTime, long long endTime, bool admin, byte *key) {
	User* oldUser = getUserByName(name);
	if (oldUser != nullptr) {
		return *oldUser;
	}

	User user = User();
	memcpy(user.name, name, NAME_SIZE);
	user.startTime = startTime;
	user.endTime = endTime;
	user.isAdmin = admin;
	memcpy(user.key, key, KEY_SIZE);

	users[currentNumberOfUsers] = user;
	currentNumberOfUsers++;

	return user;
}

User addUser(const char *name, long long startTime, long long endTime, bool admin) {
	byte key[KEY_SIZE] = {};
	for (unsigned char & i : key) {
		i = random(0, 255);
	}
	return addUser(name, startTime, endTime, admin, key);
}
