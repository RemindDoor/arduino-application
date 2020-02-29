cmake_minimum_required(VERSION 3.14)
project(ble)

set(CMAKE_CXX_STANDARD 14)

include_directories("C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\cores\\arduino"
        "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\libraries\\Wire\\src"
        "C:\\Users\\Richard Mullender\\Documents\\Arduino\\libraries\\ArduinoBLE\\src"
        "C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\avr\\include"
        "C:\\Users\\Richard Mullender\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\samd\\1.8.4\\variants\\mkrwifi1010"
        )

add_executable(ble
        ble.ino
        SDPArduino.cpp
        SDPArduino.h)
