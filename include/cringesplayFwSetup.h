#pragma once
#include <Arduino.h>

// #define APPEND_MAC_TO_HOSTNAME
// const char* hostname = "???";
// const char* friendlyName = "???";
const char* const mqttHost = "mqtt.hack";

const uint8_t customCharArray[2][8] = {
{
    0b00111,
	0b00100,
	0b00000,
	0b00000,
	0b00000,
	0b00001,
	0b00011,
	0b00110
},
{
    0b01000,
	0b10010,
	0b10100,
	0b01001,
	0b10110,
	0b00000,
	0b00001,
	0b00111
}
};

const char cfw_splash_text_l0[] = "Cringesplay firmware demo :)";
const char cfw_splash_text_l1[] = "              \1\2              ";
