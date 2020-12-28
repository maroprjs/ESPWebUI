/*
 * Network.h
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#ifdef ESP32
	#include <WiFi.h>
#elif defined(ESP8266)
	#include <ESP8266WiFi.h>
#endif

#include "WebStorage.h"

class Network {
	String const DEFAULT_CONFIG_FILE = "/network.json";
	uint const MAX_WIFI_CONNECT_TIME = 30000;//in [ms] // ... Give ESP 30 seconds to connect to station.

	struct WlanConnection{
		const char* deviceName; //use as hostname and own SSID in AP mode
		const char* routerSSID;
		const char* routerPassword;
		const char* ownPassword;
		bool dhcpInUse;
		IPAddress ipAddress;
		IPAddress netmask;
		IPAddress gateway;
		WiFiMode_t wifiMode;
	};

public:
	Network(WebStorage * storage);
	virtual ~Network();
	void begin();
	void handle();
	bool connectionFailed();
	IPAddress IP(){return _wlanConnection.ipAddress;};

private:
	bool _loadNwConfigFromWebStorage();

	WebStorage * _webStorage;
	WlanConnection _wlanConnection;
	bool _initWifiStation();
	void _initWifiAP();
	bool _isConnected();
};

#endif /* NETWORK_H_ */
