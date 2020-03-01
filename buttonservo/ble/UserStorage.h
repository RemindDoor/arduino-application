//
// Created by Richard Mullender on 01/03/2020.
//

#ifndef BLE_USERSTORAGE_H
#define BLE_USERSTORAGE_H

const int NAME_SIZE = 31;
const int KEY_SIZE = 16;

struct User {
	char name[NAME_SIZE];
	byte key[KEY_SIZE];
	long long startTime;
	long long endTime;
	bool isAdmin;
};

extern User users[];
const int NUM_USERS = 16;
const long long MAX_LONG = 9223372036854775807LL;
extern int currentNumberOfUsers;

User* getUserByName(const char name[NAME_SIZE]);

User* getUserByKey(const byte key[KEY_SIZE]);

void editName(const char oldName[NAME_SIZE], const char newName[NAME_SIZE]);

void editName(const byte key[KEY_SIZE], const char newName[NAME_SIZE]);

User addUser(const char name[NAME_SIZE], long long startTime, long long endTime, byte key[KEY_SIZE]);

User addUser(const char name[NAME_SIZE], long long startTime, long long endTime);

void deleteUser(const char *name);

#endif //BLE_USERSTORAGE_H
