// Helper.h

#ifndef _HELPER_h
#define _HELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#endif

#include "LiquidCrystal.h"

class Helper
{
public:
	/* Check methods */


	/* Array methods */
	static void PrintArray(byte obj[], byte length);
	static void PrintArray(char obj[], byte length);
	static void ArraySubstring(char source[], char result[], int pos, int pos2);
	static int ArrayIndexOf(char source[], char key[], int keyLength, int charLength);
	static void ClearArray(byte obj[], int length);
	static void ClearArray(char* obj, int length);
	
	/* Other Methods */
	static void InitalizeScreen(LiquidCrystal* lcd);
	static void InitalizeIO();
	static void DebugWrite(char method[], char message[]);
	static void LcdWrite(const __FlashStringHelper *msg);
	static void LcdWrite(char* msg);
	static void LcdWrite(int x, int y, const __FlashStringHelper *msg);
	static void LcdStatus(byte status);
};
