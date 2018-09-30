/*
* CR ANKA PROJECT
* --------------------------------------
* Developer : Caner Sarıgül
*       D&T : 25.09.2015 22:20
*     Board : Arduino MEGA
*
*   Version : 1.0.RC1
*      Code : Mysterious Backpack
*
*       Reference : http://www.tutorialspoint.com/http/http_methods.htm
*       Reference : http://blog.startingelectronics.com/disabling-the-ethernet-chip-and-sd-card-on-an-arduino-ethernet-shield/
*       Reference : http://www.freeformatter.com/mime-types-list.html
*		Reference : https://www3.ntu.edu.sg/home/ehchua/programming/webprogramming/HTTP_Basics.html
*/

//#include "inttypes.h"

#include "SPI.h"
#include "Ethernet.h"
#include "SD.h"
#include "LiquidCrystal.h"

#include "Helper.h"
#include "EthernetProtocol.h"
#include "Common.h"

/* LiquidCrystal(rs, enable, d4, d5, d6, d7) */
LiquidCrystal lcd(3, 5, 6, 7, 8, 9);
EthernetProtocol protocol;

unsigned long timeDifferent = 0;
unsigned long beatDifferent = 0;
unsigned long dataDifferent = 0;
bool lcdSwitcher = false;

void setup() {
	/* Initalize Serial port */
	DebugPort.begin(DebugBaudRate);
	ComPort.begin(DebugBaudRate); /* Fix this sometime */

	/* Initalize LCD Screen (16x2) */
	Helper::InitalizeScreen(&lcd);

	/* Set the pins */
	pinMode(PIN_Receive, OUTPUT);
	pinMode(PIN_Process, OUTPUT);
	pinMode(PIN_Config, INPUT);

	/* Give attention the device version by debug port */
	DebugPort.println(PRODUCT_Name);
	DebugPort.println(PRODUCT_Code);
	DebugPort.println(PRODUCT_Version);

	/* Ethernet Protocol Start */
	Helper::LcdWrite(F("Ethernet ..."));
	protocol.Start();

	/* Set I/O ports */
	Helper::InitalizeIO();

	/* Refresh the input status but not report them. */
	protocol.CheckInputs(false);

	/* Set the time differents for first initialization */
	timeDifferent = millis();
	beatDifferent = millis();
	dataDifferent = millis();
}

void loop() {
	/* Time calculation */
	unsigned long different = millis() - timeDifferent;
	if (different > 1000) {
		/* Loop beat ! */
		if (lcdSwitcher) {
			Helper::LcdStatus(CR_BEAT_LOOP_START);
		}
		else {
			Helper::LcdStatus(CR_BEAT_LOOP_END);
		}

		/* Change the beat type */
		lcdSwitcher = !lcdSwitcher;

		/* Refresh the time */
		timeDifferent = millis();
	}

	unsigned long different2 = millis() - beatDifferent;
	if (different2 > 1000 * 60) { /* TODO : * 60 */
		protocol.SendKnock();

		/* Refresh the time */
		beatDifferent = millis();
	}

	unsigned long different3 = millis() - dataDifferent;
	if (different3 > 1000 * 10) {
		/* Report if log exists */
		protocol.CheckLogData();

		/* Refresh the time */
		dataDifferent = millis();
	}

	/* Check input status if is there any changes exists, report the server */
	protocol.CheckInputs(true);

	/* Check RS232 messages, It takes minumum 5 sec. */
	protocol.CheckCOM();

	/* Check the client or api requests */
	protocol.Check();
}