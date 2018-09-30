/*
 * Developer : Caner Sarıgül
 *       D&T : 25.09.2015 23:19
 *		File : EthernetProtocol.cpp
 *
 * Some suggestion here one developer to another, I wrote them when I drunk so keep it that way.
 */

#include "avr/wdt.h"
#include "Ethernet.h"
#include "SD.h"

#include "EthernetProtocol.h"
#include "Helper.h"
#include "Common.h"

EthernetServer server(80);
EthernetClient client;
byte addr_device[] = { 0, 0, 0, 0 };
byte addr_server[] = { 0,0,0,0 };
byte addr_mac[] = { 0, 0, 0, 0, 0, 0 };
unsigned int addr_id = 0;
unsigned int baudRate = 0;

bool isConfigured = false;
bool inputStatus[(PIN_INPUT_End - PIN_INPUT_Start) + 1]; /* It contains input status */

void EthernetProtocol::Start() {
	/* Activate the ethernet */
	pinMode(PIN_Ethernet, OUTPUT);
	digitalWrite(PIN_Ethernet, HIGH);

	if (SD.begin(PIN_SD)) {
		File setting = SD.open(L_Setting);
		if (setting) {
			/* Parse the settings from the file */
			EthernetProtocol::LoadDeviceConfiguration();

			/* Close the file */
			setting.close();

			/* Start ethernet device */
			if (addr_device[0] == 0 && addr_device[1] == 0) {
				Helper::LcdStatus(CR_SETTING_DHCP);

				/* DHCP */
				if (Ethernet.begin(addr_mac) == 0) {
					Helper::LcdStatus(CR_SETTING_NOIP);
					return;
				}
			}
			else {
				DebugPort.print("IP LOADING !");

				/* Start with known ip address */
				Ethernet.begin(addr_mac, addr_device);

				/* Next version check the it's ready or not */
				isConfigured = true;

				/* Show the ip address is static */
				Helper::LcdWrite(2, 16, F("#"));
			}

			/* Start the server */
			server.begin();

			delay(300);

			/* It will delete api folder */
			Helper::LcdStatus(CR_SETTING_CLEAR);

			/* Print the started configuration by debug port */
			DebugPort.println(F("STAGE 2 - API"));

#pragma region Prepare the api methods
			/* Delete and create api folder */
			SD.rmdir(L_API_Folder);
			SD.mkdir(L_API_Folder);

			SD.remove(L_API_IP);

			/* Open or create the gIP.cs file */
			File gIP = SD.open(L_API_IP, O_CREAT | O_WRITE);

			DebugPort.print(F("IP: "));

			IPAddress ip = Ethernet.localIP();
			for (int i = 0; i < 4; i++)
			{
				gIP.print(ip[i]);

				DebugPort.print(ip[i]);

				if (i != 3) {
					gIP.print('.');
					DebugPort.print('.');
				}
			}
			gIP.close();
			memset(&ip, sizeof(ip), 0);

			/* Open or create the gMAC file */
			SD.remove(L_API_MAC);

			DebugPort.println();
			DebugPort.print(F("MAC: "));

			/* Open or create gMac.cs file */
			File gMAC = SD.open(L_API_MAC, O_CREAT | O_WRITE);
			for (int i = 0; i < 7; i++)
			{
				gMAC.print(addr_mac[i], HEX);
				DebugPort.print(addr_mac[i], HEX);

				if (i != 6) {
					gMAC.print(':');
					DebugPort.print(':');
				}
			}
			gMAC.close();

			EthernetProtocol::CopyFile(L_ID, L_API_ID);
			EthernetProtocol::CopyFile(L_Descript, L_API_Descript);
			EthernetProtocol::CopyFile(L_Server, L_API_Server);
			EthernetProtocol::CopyFile(L_BaudRate, L_API_BaudRate);
#pragma endregion

			/* Show the board is ready for battle */
			Helper::LcdStatus(CR_READY);

			/* Default the lcd screen (Show the ip address) */
			EthernetProtocol::LcdDefault();
		}
		else {
			DebugPort.println(F("SD CR_SD_WRONG"));
			Helper::LcdStatus(CR_SD_WRONG);
		}
	}
	else {
		DebugPort.println(F("SD CR_SD_UNAVALIABLE"));
		Helper::LcdStatus(CR_SD_UNAVALIABLE);
	}
}

void EthernetProtocol::Check() {
	if (!isConfigured) {
		return;
	}

	EthernetClient client = server.available();
	if (client) { /* has a client ? */
		char cache[bufferSize] = "";

		/* Check first 3 bits */
		if (client.available() > 3)
		{
			/* Set the timeout for 10 sec */
			//client.setTimeout(10000);

			/* Check the pin status */
			//int configStatus = digitalRead(A0);

			/* Retrieve the ip address */
			//byte clientIP[] = { 0, 0, 0, 0 };
			//client.getRemoteIP(clientIP);

			char method[] = "   ";
			method[0] = client.read();
			method[1] = client.read();
			method[2] = client.read();

			bool isGet = (strcmp(method, "GET") == 0);
			bool isPost = (strcmp(method, "POS") == 0);

			/* Clear the method array */
			memset(method, 0, sizeof(method));

			if (isGet || isPost) {
				/* read the empty char or 'T' char */
				client.read();

				/* if it's get method, it'll read again for the empty char */
				if (isPost) {
					client.read();
				}
			}

			/* Define new buffer for post method */
			char postData[postSize] = { 0 };
			if (!isPost) {
				memset(postData, 0, sizeof(postData));
			}

			unsigned int bufPos = 0;
			do
			{
				char c = client.read();
				if (bufPos > bufferSize) {
					/* It'll be overloaded, clear the cache */
					EthernetProtocol::Clear(&client);
					break;
				}

				cache[bufPos] = c;
				if (c == ' ') {
					/* Read post message after double enter message */
					if (isPost) {
						bool isEnter = false;
						while (client.available()) {
							c = client.read();
							if (c == '\r' && (client.read() == '\n')) {

								/* Check that it has enter before */
								if (isEnter == true)
								{
									/* Fill the post data */
									for (byte i = 0; i <= postSize; i++)
									{
										if (client.available()) {
											postData[i] = client.read();
										}
									}
									postData[postSize] = '\0';

									/* exit the while loop */
									break;
								}
								isEnter = true;
							}
							else {
								isEnter = false;
							}
						}
					}

					/* Flush the cache */
					EthernetProtocol::Clear(&client);
				}

				bufPos++;
			} while (client.available());

			/* For the last action */
			bufPos--;

			/* Terminate the cache buffer */
			cache[bufPos] = '\0';

			if (isGet) {
				DebugPort.println(F("GET OK!"));

				/* Check the root page */
				if (cache[0] == '/' && bufPos <= 2) {
					Helper::LcdWrite(F(">> Main Page"));

					/* Destory the cache */
					memset(cache, 0, sizeof(cache));

					/* Redirect to public index page. */
					EthernetProtocol::Redirect(&client, PAGE_INFO);
				}
				else {
					/* Is a public url ? */
					int urlPosition = Helper::ArrayIndexOf(cache, "/p/", 3, bufPos);
					if (urlPosition != -1) {
						DebugPort.println(F("PUBLIC URL!"));

						/* Wait for debugPort to deliver the message */
						delay(50);

						/* Catch the file name */
						char fileName[20] = { 0 };
						Helper::ArraySubstring(cache, fileName, 0, bufPos);
						fileName[bufPos] = '\0';

						if (strcmp(fileName, L_API_Virtual_Reset) == 0) { /* <IP>/p/api/reset.cex */
							/* Restart the device */
							client.stop();
							EthernetProtocol::ResetTheDevice();
						}
						else if (strcmp(fileName, L_API_Virtual_ClearOutput) == 0) { /* <IP>/p/api/cOut.cex */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							/* Set outputs to passive */
							for (uint8_t pin = PIN_OUTPUT_Start; pin <= PIN_OUTPUT_End; pin++)
							{
								digitalWrite(pin, LOW);
							}
						}
						else if (strncmp(fileName, L_API_Virtual_SetOutput, 15) == 0) { /* <IP>/p/api/sOut.cex */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							char pinBuff[2] = { fileName[17] , fileName[18] };
							int pin = atoi(pinBuff);

							/* Check the pin scale */
							if (pin >= PIN_OUTPUT_Start && pin <= PIN_OUTPUT_End) {
								/* Check the operator */
								char op = fileName[16];
								if (op == '$') {
									digitalWrite(pin, HIGH);
								}
								else {
									digitalWrite(pin, LOW);
								}

								/* Clear the pin buffer */
								memset(pinBuff, 0, sizeof(pinBuff));

								/* Send OK */
								client.print(F("OK"));
							}
							else {
								client.print(F("You can't change this pin!"));
							}
						}
						else if (strcmp(fileName, L_API_Virtual_GetInput) == 0) { /* <IP>/p/api/gIn.cex */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							/* Set outputs to passive */
							for (uint8_t pin = PIN_INPUT_Start; pin < PIN_INPUT_End; pin++) /* Except last one! */
							{
								client.print(inputStatus[pin]);
								client.print(F(","));
							}
							client.print(inputStatus[PIN_INPUT_End]);
						}
						else if (strcmp(fileName, L_API_Virtual_Device) == 0) { /* <IP>/p/api/device.cex */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							client.println(PRODUCT_Name);
							client.println(PRODUCT_Code);
							client.println(PRODUCT_Version);
						}
						else if (strcmp(fileName, L_API_Virtual_GetLogSize) == 0) {  /* <IP>/p/api/getLogSize.cex */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							File logFile = SD.open(L_LOG, O_READ);
							if (logFile) {
								/* Get the size and close the file */
								int size = logFile.size();
								logFile.close();

								client.println(size);
							}
							else {
								client.println(F("UPS!"));
							}
						}
						else if (strcmp(fileName, L_API_Virtual_Log) == 0) { /* <IP>/p/api/log.cex */
							digitalWrite(PIN_Receive, HIGH);

							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							/* Send log content */
							File logFile = SD.open(L_LOG, O_READ);
							if (logFile) {
								while (logFile && logFile.available() > 0)
								{
									client.write(logFile.read());
								}
								logFile.close();
							}
							else {
								client.println();
							}

							/* Terminate the client request */
							client.println();
							client.stop(); /* Other actions will take the times so that client will close there. */

							/* TODO : Version 2, Fix the big file issue */
							/* Backup the log file */
							EthernetProtocol::CopyOrAppendFile(L_LOG, L_LOG_BACKUP);

							/* Delete the old message befoer the backup */
							SD.remove(L_LOG);

							digitalWrite(PIN_Receive, LOW);
						}
						else if (strcmp(fileName, L_API_Virtual_GetAll) == 0) { /* <IP>/p/api/gAll.cs */
							/* Send the http header */
							EthernetProtocol::SendOKStatus(&client);

							/* MAC */
							for (byte i = 0; i < 7; i++)
							{
								client.print(addr_mac[i], HEX);
							}
							client.println();

							/* IP */
							for (byte i = 0; i < 3; i++)
							{
								client.print(addr_device[i]);
								client.print('.');
							}
							client.print(addr_device[3]);
							client.println();

							/* Server */
							for (byte i = 0; i < 3; i++)
							{
								client.print(addr_server[i]);
								client.print('.');
							}
							client.print(addr_server[3]);
							client.println();

							/* Id */
							client.println(addr_id);

							File descriptFile = SD.open(L_Descript, O_READ);
							if (descriptFile.available()) {
								while (descriptFile.available())
								{
									client.write(descriptFile.read());
								}
							}
							else {
								client.print(" ");
							}
							client.println();

							/* BaudRate */
							File baudRateFile = SD.open(L_BaudRate, O_READ);
							if (baudRateFile.available()) {
								while (baudRateFile.available())
								{
									client.write(baudRateFile.read());
								}
							}
							else {
								client.print(" ");
							}
							client.println();
						}
						else {
							char ext[3] = { 0 };
							ext[0] = fileName[bufPos - 3];
							ext[1] = fileName[bufPos - 2];
							ext[2] = fileName[bufPos - 1];
							ext[3] = '\0';

							DebugPort.print(F("FILE :"));
							DebugPort.println(fileName);

							/* Release the cache */
							memset(cache, 0, sizeof(cache));

							File requestFile = SD.open(fileName, FILE_READ);
							if (requestFile) {
								Helper::LcdWrite(fileName);

								DebugPort.print(F("OK!"));

								client.println(H_200);
								client.println(H_Server);

								client.print(H_Content_Type);
								if (strcmp(ext, "htm") == 0) {
									client.println(MIME_html);
								}
								else if (strcmp(ext, "jpg") == 0) {
									client.println(MIME_jpg);
								}
								else if (strcmp(ext, "png") == 0) {
									client.println(MIME_png);
								}
								else if (strcmp(ext, ".js") == 0) {
									client.println(MIME_js);
								}
								else if (strcmp(ext, "css") == 0) {
									client.println(MIME_css);
								}
								else if (strcmp(ext, "ttf") == 0) {
									client.println(MIME_ttf);
								}
								else if (strcmp(ext, "ico") == 0) {
									client.println(MIME_ico);
								}
								else if (strcmp(ext, "txt") == 0 || strcmp(ext, ".cs") == 0) {
									client.println(MIME_txt);
								}

								/* Release the ext */
								memset(ext, 0, sizeof(ext));

								client.print(F("Content-Length:"));
								client.println(requestFile.size());
								client.println(H_Connection_Close);
								client.println();

								/* Send the file content */
								while (requestFile.available()) {
									client.write(requestFile.read());
								}

								/* Close the file */
								requestFile.close();
							}
							else if ((strcmp(ext, "htm") == 0)) {
								DebugPort.println(F("REDIRECT"));
								EthernetProtocol::Redirect(&client, PAGE_WRONG);
							}
							else {
								DebugPort.println(F("Not Found"));

								client.println(H_404);
								client.println(H_Server);
								client.println(H_Connection_Close);
								client.println();
								client.println();
							}
						}

						/* When the api method call, we don't clear the cache. So it'll do it again for mismatch */
						memset(cache, 0, sizeof(cache));
					}
					else {
						/* Notice : Nobody not be able to access the files but p folder */

						/* Send the http header */
						EthernetProtocol::SendOKStatus(&client);
						client.println();
						client.println();

						//EthernetProtocol::Redirect(&client, PAGE_WRONG);
					}
				}

				/* Wait the process */
				delay(200);

				/* stop the client connection */
				client.stop();
			}
			else if (isPost) {
				// postData contains posted data, cache is the page which posted.
				unsigned int pIndex = 0;
				char nextChar = '\0';

				do
				{
					/* Take the command flag */
					char command = postData[pIndex];
					pIndex += 2; // For '=' and command flag

					/* Parse the command parametre */
					char pBuffer[eachPostSize] = { 0 };
					for (unsigned int maxIndex = pIndex + eachPostSize; pIndex < maxIndex; pIndex++) /* Maxiumum read is 50 characters for each command */
					{
						nextChar = postData[pIndex];

						if (nextChar == '&' || nextChar == '\0')
							break;

						/* nextChar will replace when it owns '+' character */
						if (nextChar == '+')
							nextChar = ' ';

						pBuffer[pIndex - (maxIndex - eachPostSize)] = nextChar;
					}
					pIndex++; /* For '&' character */

					/* Terminate the post buffer array */
					pBuffer[pIndex] = '\0';

					/* Handle the post data */
					if (strcmp(cache, "/p/settings.htm") == 0) {
						const __FlashStringHelper *sLocation;

						switch (command)
						{
						case 'i': /* IP Address */
							sLocation = L_IP;
							break;
						case 's':
							sLocation = L_Server;
							break;
						case 'd':
							sLocation = L_Descript;
							break;
						case 'a':
							sLocation = L_BaudRate;
							break;
						}

						/* Save when data exists */
						if (strlen(pBuffer) != 0) {
							/* Delete the file for clean writing process */
							SD.remove(sLocation);

							/* Open the file with writing and create permission */
							File fLoc = SD.open(sLocation, O_CREAT | O_WRITE);
							fLoc.print(pBuffer);
							fLoc.close();

							DebugPort.print(F("Path:"));
							DebugPort.println(sLocation);

							DebugPort.println(pBuffer);
							DebugPort.println(F("Saved!"));
						}
					}
				} while (nextChar != '\0');

				/* Send OK Status for post request */
				EthernetProtocol::SendOKStatus(&client);

				client.println(F("İşleminiz Basarılıdır"));
				client.stop();
				delay(100);

				/* Restart the device for the apply the new settings */
				EthernetProtocol::ResetTheDevice();
			}
		}
	}
	/*EthernetProtocol::LcdDefault();*/
}

void EthernetProtocol::CheckCOM() {
	/* When rs232 has message, it will save them into log file */
	//int configPinStatus = digitalRead(PIN_Config);

	/* TODO : configPinStatus == LOW &&  add FIX */

	if (ComPort.available() > 0) {
		DebugPort.print(F("COM:"));
		DebugPort.println(ComPort.available());

		/* Call the server method */
		this->SendReceiveStarted();

		File logFile = SD.open(L_LOG, O_CREAT | O_WRITE | O_APPEND);
		if (logFile) {
			digitalWrite(PIN_Receive, HIGH);
			digitalWrite(PIN_Process, HIGH);

			Helper::LcdStatus(CR_SERIAL);
			int time = 0;
			do
			{
				if (ComPort.available() > 0) {
					digitalWrite(PIN_Process, HIGH);

					time = 0;
					logFile.write(ComPort.read());
				}
				else {
					digitalWrite(PIN_Process, LOW);
					time++;
				}
			} while (time <= COM_Timeout);
			logFile.close();

			/* Call the server method */
			this->SendReceiveEnded();

			digitalWrite(PIN_Receive, LOW);
		}
		else {
			Helper::LcdStatus(CR_SD_UNAVALIABLE);
		}
	}
}

void EthernetProtocol::SendKnock() {
	int connectionResult = -10;
	connectionResult = client.connect(addr_server, 8080);
	if (connectionResult == 1) {
		DebugPort.println(F("Server found !"));

		/* Knock the host */
		client.print(F("GET /CR/Knock?id="));
		client.print(addr_id);
		client.println(F(" HTTP/1.0"));
		client.println();

		/* Wait until client is avaliable */
		while (client.connected() && !client.available()) {
			delay(1);
		}

		/* Get the answer which come from host */
		//unsigned int buffIndex = 0;
		//while (client.connected() || client.available()) { //connected or data available
		//	DebugPort.print(client.read());
		//}
		delay(10);

		/* Stop the client connection */
		client.stop();
	}
	else {
		DebugPort.println(F("Server [NOT] found !"));
		DebugPort.println(connectionResult);

		/* Change lcd status */
		Helper::LcdStatus(CR_NO_HOST);
	}
}

void EthernetProtocol::CheckLogData() {
	File logFile = SD.open(L_LOG, O_READ);
	if (logFile) {
		uint32_t size = logFile.size();
		if (size > 0) {
			int connectionResult = client.connect(addr_server, 8080);
			if (connectionResult == 1) {
				DebugPort.println(F("Server found !"));

				/* Call the methods */
				client.print(F("GET /CR/OldExist?id="));
				client.print(addr_id);
				client.print("&size=");
				client.print(size);
				client.println(F(" HTTP/1.0"));
				client.println();

				/* Wait until client is avaliable */
				while (client.connected() && !client.available()) {
					delay(1);
				}
			}
			delay(10);

			/* Stop the client */
			client.stop();
		}
	}
}

void EthernetProtocol::SendReceiveStarted() {
	int connectionResult = client.connect(addr_server, 8080);
	if (connectionResult == 1) {
		DebugPort.println(F("Server found !"));

		/* Call the method */
		client.print(F("GET /CR/ReceiveStarted?id="));
		client.print(addr_id);
		client.println(F(" HTTP/1.0"));
		client.println();

		/* Wait until client is avaliable */
		while (client.connected() && !client.available()) {
			delay(1);
		}

		/* Get the answer which come from host */
		//unsigned int buffIndex = 0;
		//while (client.connected() || client.available()) { //connected or data available
		//	DebugPort.print(client.read());
		//}
		delay(10);

		/* Stop the client connection */
		client.stop();
	}
	else {
		DebugPort.println(F("Server [NOT] found !"));
		DebugPort.println(connectionResult);

		/* Change lcd status */
		Helper::LcdStatus(CR_NO_HOST);
	}
}

void EthernetProtocol::SendReceiveEnded() {
	int connectionResult = client.connect(addr_server, 8080);
	if (connectionResult == 1) {
		DebugPort.println(F("Server found !"));

		File file = SD.open(L_LOG, O_READ);
		uint32_t size = file.size();
		file.close();

		/* Call the method */
		client.print(F("GET /CR/ReceiveEnded?id="));
		client.print(addr_id);
		client.print(F("&logSize="));
		client.print(size);
		client.println(F(" HTTP/1.0"));
		client.println();

		/* Wait until client is avaliable */
		while (client.connected() && !client.available()) {
			delay(1);
		}

		/* Get the answer which come from host */
		//unsigned int buffIndex = 0;
		//while (client.connected() || client.available()) { //connected or data available
		//	DebugPort.print(client.read());
		//}
		delay(10);

		/* Stop the client connection */
		client.stop();
	}
	else {
		DebugPort.println(F("Server [NOT] found !"));
		DebugPort.println(connectionResult);

		/* Change lcd status */
		Helper::LcdStatus(CR_NO_HOST);
	}
}

void EthernetProtocol::CheckInputs(bool report) {
	for (uint8_t pin = PIN_INPUT_Start; pin <= PIN_INPUT_End; pin++)
	{
		bool pinValue = (digitalRead(pin) == HIGH);
		if (inputStatus[pin] != pinValue) {
			/* Report the status to host */
			if (report) {
				ReportPin(pin, pinValue);
			}

			/* Set pin value */
			inputStatus[pin] = pinValue;
		}
	}
}

void EthernetProtocol::ReportPin(int pin, bool value) {
	int connectionResult = client.connect(addr_server, 8080);
	if (connectionResult == 1) {
		DebugPort.println(F("Server found !"));

		/* Call the method */
		client.print(F("GET /CR/InputChanged?id="));
		client.print(addr_id);
		client.print(F("&pin="));
		client.print(pin);
		client.print(F("&value="));
		client.print(value);
		client.println(F(" HTTP/1.0"));
		client.println();

		/* Wait until client is avaliable */
		while (client.connected() && !client.available()) {
			delay(1);
		}
		delay(10);

		/* Stop the client connection */
		client.stop();
	}
	else {
		if (connectionResult != 0) {
			DebugPort.println(F("POWER ISSUE !!"));
			
			/* Change lcd status */
			Helper::LcdStatus(CR_POWER_PROBLEM);
		}
		else {
			DebugPort.println(F("Server [NOT] found !"));
			DebugPort.println(connectionResult);

			/* Change lcd status */
			Helper::LcdStatus(CR_NO_HOST);
		}
	}
}

void EthernetProtocol::Clear(EthernetClient* client) {
	while (client->available())
	{
		client->read();
	}
}

void EthernetProtocol::Redirect(EthernetClient* client, byte location) {
	client->println(H_301);
	client->println(H_Server);
	client->println(H_Connection_Close);
	client->print(F("Location: "));

	switch (location)
	{
	case PAGE_INFO:
		client->println(F("p/info.htm"));
		break;
	case PAGE_SETTING:
		client->println(F("p/settings.htm"));
		break;
	case PAGE_WRONG:
		client->println(F("p/error.htm"));
		break;
	}

	client->println();
	client->println();
}

void EthernetProtocol::LcdDefault() {
	IPAddress ipAddr = Ethernet.localIP();
	char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	Helper::LcdWrite(buf);

	/* Release the buffer */
	memset(buf, 0, sizeof(buf));
	memset(&ipAddr, 0, sizeof(ipAddr));
}

void EthernetProtocol::ResetTheDevice() {
	/* Method 1 */
	//asm volatile ("jmp 0");

	/* Clean way to reset the device */
	wdt_enable(WDTO_60MS);
	while (true) {}

	//pinMode(A4, OUTPUT);
	//digitalWrite(A4, LOW);
}

void EthernetProtocol::CopyFile(const __FlashStringHelper *source, const __FlashStringHelper *destination) {
	File sFile = SD.open(source);
	if (sFile) {
		SD.remove(destination);

		File dFile = SD.open(destination, O_CREAT | O_WRITE);
		while (sFile.available())
		{
			dFile.write(sFile.read());
		}
		dFile.close();
	}
	sFile.close();
}

void EthernetProtocol::CopyOrAppendFile(const __FlashStringHelper *source, const __FlashStringHelper *destination) {
	File sFile = SD.open(source);
	if (sFile) {
		File dFile = SD.open(destination, O_CREAT | O_WRITE | O_APPEND);
		while (sFile.available())
		{
			dFile.write(sFile.read());
		}
		dFile.close();
	}
	sFile.close();
}

void EthernetProtocol::LoadDeviceConfiguration() {
	byte index = 0;
	byte sectorIndex = 0;
	char nextChar = '\0';

	DebugPort.println(F("STAGE 1 - LOAD"));

#pragma region BaudRate
	char rate[5] = { 0 };

	File fRate = SD.open(L_BaudRate, O_READ);
	while (fRate.available())
	{
		rate[index] = fRate.read();
	}
	fRate.close();

	//baudRate = static_cast<unsigned int>(atoi(rate));

	//TODO : FIX
	baudRate = 9600;
	//ComPort.begin(baudRate);

	DebugPort.print(F("COM "));
	DebugPort.print(baudRate);
	DebugPort.println(F(" [OK]"));
#pragma endregion

	index = 0;

#pragma region MAC
	char macSector[2] = { 0 };

	File macFile = SD.open(L_Mac, O_READ);
	while (macFile.available())
	{
		macSector[0] = macFile.read();
		macSector[1] = macFile.read();

		addr_mac[sectorIndex] = static_cast<byte>(strtol(macSector, NULL, 16));
		sectorIndex++;
	}
	macFile.close();

	/* Clear the sector array */
	memset(macSector, sizeof(macSector), 0);

	DebugPort.print(F("MAC "));
	Helper::PrintArray(addr_mac, 6);
	DebugPort.println(F(" [OK]"));
#pragma endregion

	char ipSector[3] = { 0 };
	sectorIndex = 0;

#pragma region IP
	/* Load ip configuration from sd card */
	File ipFile = SD.open(L_IP, O_READ);
	while (ipFile.available())
	{
		nextChar = ipFile.read();

		ipSector[index] = nextChar;
		if (nextChar == '.' || !ipFile.available())
		{
			if (nextChar == '.')
				ipSector[index] = '\0';

			ipSector[index + 1] = '\0';
			addr_device[sectorIndex] = static_cast<byte>(atoi(ipSector));

			index = -1;
			sectorIndex++;
		}
		index++;
	}
	ipFile.close();

	DebugPort.print(F("IP "));
	Helper::PrintArray(addr_device, 4);
	DebugPort.println(F(" [OK]"));
#pragma endregion

	sectorIndex = 0;
	index = 0;

#pragma region Server
	/* Load server ip configuration from sd card */
	File serverFile = SD.open(L_Server, O_READ);
	while (serverFile.available())
	{
		nextChar = serverFile.read();

		ipSector[index] = nextChar;
		if (nextChar == '.' || !serverFile.available())
		{
			if (nextChar == '.')
				ipSector[index] = '\0';

			ipSector[index + 1] = '\0';
			addr_server[sectorIndex] = static_cast<byte>(atoi(ipSector));

			index = -1;
			sectorIndex++;
		}
		index++;
	}
	serverFile.close();

	DebugPort.print(F("Server "));
	Helper::PrintArray(addr_server, 4);
	DebugPort.println(F(" [OK]"));
#pragma endregion

	index = 0;
	char strId[7] = { 0 };

#pragma region ID
	File idFile = SD.open(L_ID, O_READ);
	while (idFile.available()) {
		strId[index] = idFile.read();
		index++;
	}
	idFile.close();

	DebugPort.print(F("ID "));
	DebugPort.print(strId);
	DebugPort.println(F(" [OK]"));
#pragma endregion

	addr_id = atoi(strId);
}

void EthernetProtocol::SendOKStatus(EthernetClient *client) {
	client->println(H_200);
	client->println(H_Server);
	client->print(H_Content_Type);
	client->println(MIME_txt);
	client->println();
}
