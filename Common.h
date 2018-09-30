#define bufferSize 100
#define postSize 70
#define eachPostSize 50
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

#pragma region Serial Port
#define SERIAL_BUFFER_SIZE 256
#define DebugPort Serial
#define ComPort Serial1
#define DebugBaudRate 9600
#define COM_Timeout 9000
#pragma endregion

#pragma region Product
#define PRODUCT_Name F("CR")
#define PRODUCT_Version F("V1002 11.12.2015")
#define PRODUCT_Code F("HOTDOG")
#pragma endregion

#pragma region Pins
#define PIN_SD 4
#define PIN_Ethernet 10
#define PIN_Config A0

#define PIN_Receive A1
#define PIN_Process A2

#define PIN_INPUT_Start 22
#define PIN_OUTPUT_Start 38

#define PIN_INPUT_End 22 // 37
#define PIN_OUTPUT_End 38 // 53
#pragma endregion

#pragma region Device Status
#define CR_UP 0
#define CR_DOWN 1
#define CR_READY 2
#define CR_SERVER 3
#define CR_BEAT 4
#define CR_SERIAL 5
#define CR_BEAT_LOOP_START 6
#define CR_BEAT_LOOP_END 7
#define CR_SD_UNAVALIABLE 11
#define CR_SD_WRONG 12 
#define CR_SETTING_WRONG 13
#define CR_SETTING_NOIP 14
#define CR_SETTING_DHCP 15
#define CR_SETTING_PREPARE 16
#define CR_SETTING_CLEAR 17
#define CR_NO_HOST 30
#define CR_POWER_PROBLEM 40
#pragma endregion

#pragma region Locations
#define L_BaudRate F("/s/baudrate.cr")
#define L_Setting F("/s/settings.cr")
#define L_Server F("/s/server.cr")
#define L_IP F("/s/ip.cr")
#define L_Descript F("/s/descript.cr")
#define L_ID F("/s/id.cr")
#define L_Mac F("/s/mac.cr")
#define L_LOG F("/log/geek.xlg")
#define L_LOG_BACKUP F("/log/backup.clg")

#define L_API_Folder F("/p/api/")
#define L_API_MAC F("/p/api/gMAC.cex")
#define L_API_IP F("/p/api/gIP.cex")
#define L_API_Server F("/p/api/gServ.cex")
#define L_API_ID F("/p/api/gID.cex")
#define L_API_Descript F("/p/api/gDes.cex")
#define L_API_BaudRate F("/p/api/gRate.cex")

#define L_API_Virtual_Reset "/p/api/reset.cex"
#define L_API_Virtual_Device "/p/api/device.cex"
#define L_API_Virtual_SetOutput "/p/api/sOut.cex"
#define L_API_Virtual_ClearOutput "/p/api/cOut.cex"
#define L_API_Virtual_GetInput "/p/api/gIn.cex"
#define L_API_Virtual_Log "/p/api/gLog.cex"
#define L_API_Virtual_GetAll "/p/api/gAll.cex"
#define L_API_Virtual_GetLogSize "/p/api/gSize.cex"
#pragma endregion

#pragma region HTTP Header
#define H_200 F("HTTP/1.1 200 OK")
#define H_404 F("HTTP/1.1 404 Not Found")
#define H_301 F("HTTP/1.1 301 Moved Permanently")
#define H_Server F("Server: CR-100")
#define H_Connection_Close F("Connection: close")
#define H_Content_Type F("Content-Type: ")
#pragma endregion

#pragma region MIME Types
#define MIME_html F("text/html")
#define MIME_jpg F("image/jpeg")
#define MIME_png F("image/png")
#define MIME_js F("application/javascript")
#define MIME_css F("text/css")
#define MIME_ttf F("font/opentype")
#define MIME_ico F("image/x-icon")
#define MIME_txt F("text/plain;charset=utf-8")
#pragma endregion

#pragma region Pages
#define PAGE_INFO 0
#define PAGE_SETTING 1
#define PAGE_WRONG 2
#pragma endregion 
