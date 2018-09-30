// Developer : Caner Sarýgül
//		 D&T : 25.09.2015 19:55
// 
// This class contains helper methods

#include "Helper.h"
#include "Common.h"

static LiquidCrystal* Lcd;

void Helper::InitalizeScreen(LiquidCrystal* lcd) {
	Lcd = lcd;
	Lcd->begin(16, 2);

	Lcd->clear();
	Lcd->setCursor(0, 0);
	Lcd->print(F(" CR"));
}

void Helper::InitalizeIO() {
	/* Inputs | 22 => 37 */
	for (int i = PIN_INPUT_Start; i <= PIN_INPUT_End; i++)
	{
		pinMode(i, INPUT);
	}

	/* Outputs | 38 => 53 */
	for (int o = PIN_OUTPUT_Start; o <= PIN_OUTPUT_End; o++)
	{
		pinMode(o, OUTPUT);
	}
}

void Helper::PrintArray(char obj[], byte length)
{
	for (byte i = 0; i < length; i++) {
		DebugPort.print(obj[i]);
	}
}

void Helper::PrintArray(byte obj[], byte length)
{
	for (byte i = 0; i < length - 1; i++) {
		DebugPort.print(obj[i]);
		DebugPort.print(F("."));
	}
	DebugPort.print(obj[length - 1]);
}

void Helper::ClearArray(byte obj[], int length) {
	for (int i = 0; i < length; i++) {
		obj[i] = 0;
	}
}

void Helper::ClearArray(char* obj, int length) {
	for (int i = 0; i < length; i++) {
		obj[i] = '\0';
	}
}

int Helper::ArrayIndexOf(char source[], char key[], int keyLength, int charLength) {
	unsigned int pos = -1;

	bool isSuccess = true;
	int searchIndex = 0;
	for (int i = 0; i <= charLength; i++) {
		isSuccess = true;

		for (int sIndex = 0; sIndex < keyLength; sIndex++) {
			if (source[i + sIndex] != key[sIndex]) {
				isSuccess = false;
				break;
			}
		}

		if (isSuccess) {
			pos = searchIndex;
			break;
		}

		if (searchIndex + keyLength > charLength) {
			break;
		}
		searchIndex++;
	}
	return pos;
}

void Helper::ArraySubstring(char source[], char result[], int start, int stop) {
	for (int cIndex = start; cIndex < stop; cIndex++) {
		result[(cIndex - start)] = source[cIndex];
	}
}

void Helper::DebugWrite(char method[], char message[]) {
	int configPin = digitalRead(A1);
	if (configPin == HIGH) {
		Lcd->clear();
		Lcd->setCursor(0, 0);
		Lcd->print(method);
		Lcd->setCursor(0, 1);
		Lcd->print(message);
	}

	memset(method, 0, sizeof(method));
	memset(message, 0, sizeof(message));
}

void Helper::LcdWrite(const __FlashStringHelper *msg) {
	Lcd->setCursor(0, 1);
	Lcd->print(msg);
}

void Helper::LcdWrite(int x, int y, const __FlashStringHelper *msg) {
	Lcd->setCursor(y - 1, x);
	Lcd->print(msg);
}

void Helper::LcdWrite(char* msg) {
	/* Clear the lcd row */
	Lcd->setCursor(0, 1);
	Lcd->print(F("                "));

	/* Reposition and write the message */
	Lcd->setCursor(0, 1);
	Lcd->print(msg);
}

void Helper::LcdStatus(byte status) {
	Lcd->setCursor(6, 0);
	switch (status)
	{
	case CR_UP:
		Lcd->print(F("Up"));
		break;

	case CR_DOWN:
		Lcd->print(F("Down"));
		break;

	case CR_BEAT:
		Lcd->print(F("Beat"));
		break;

	case CR_READY:
		Lcd->print(F("HAZIR"));
		break;

	case CR_SERVER:
		Lcd->print(F("Server"));
		break;

	case CR_SERIAL:
		Lcd->write(127);
		Lcd->print(F(" COM"));
		break;

	case CR_BEAT_LOOP_START:
		Lcd->setCursor(0, 0);
		Lcd->print(' ');
		break;

	case CR_BEAT_LOOP_END:
		Lcd->setCursor(0, 0);
		Lcd->write(246);
		break;

	case CR_SD_WRONG:
		Lcd->print("SD");
		Helper::LcdWrite(F("File Not Found"));
		break;

	case CR_SD_UNAVALIABLE:
		Lcd->print("SD");
		Helper::LcdWrite(F("SD Unavailable"));
		break;

	case CR_SETTING_DHCP:
		Lcd->print(F("Dhcp"));
		break;

	case CR_SETTING_NOIP:
		Lcd->print("No-IP");
		break;

	case CR_NO_HOST:
		Lcd->print("SERVER PRB");
		break;

	case CR_POWER_PROBLEM:
		Lcd->print("GUC PRB");
		break;

	default:
		break;
	}
}