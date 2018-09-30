#ifndef _ETHERNETPROTOCOL_h
#define _ETHERNETPROTOCOL_h
#endif

class EthernetProtocol {
public:
	void Start();
	void Check();
	void CheckCOM();

	/* Protocol methods */
	void SendKnock();
	void CheckLogData();
	void CheckInputs(bool report);
private:
	void Clear(EthernetClient* client);
	void Redirect(EthernetClient* client, byte location);
	void LcdDefault();
	void ResetTheDevice();
	void CopyFile(const __FlashStringHelper *source, const __FlashStringHelper *destination);
	void CopyOrAppendFile(const __FlashStringHelper *source, const __FlashStringHelper *destination);

	void LoadDeviceConfiguration();
	void SendOKStatus(EthernetClient *client);

	/* Protocol methods */
	void SendReceiveStarted();
	void SendReceiveEnded();
	void ReportPin(int pin, bool value);
};