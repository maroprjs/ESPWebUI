/*
 * Network.cpp
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#include "Network.h"
#include <ArduinoJson.h>
#include "Debug.h"

Network::Network(WebStorage * storage) {
	_webStorage = storage;
}

Network::~Network() {
	// TODO Auto-generated destructor stub
}


void Network::begin() {
	WiFi.scanNetworks(true);  //async needs to be called once on startup...
	DPRINTLN("wifi scan...");
	if (_loadNwConfigFromWebStorage()){ //success
		if (_wlanConnection.wifiMode == wifi_mode_t::WIFI_STA){
			WiFi.mode(WIFI_OFF);
			delay(100);
			WiFi.mode(WIFI_STA);
			DPRINTLN("Required  WiFi.mode(WIFI_STA)!");
			if (_initWifiStation() == false){
				DPRINTLN("Connection failed, switching to AP mode");
				WiFi.mode(WIFI_OFF);
				delay(100);
				WiFi.mode(WIFI_AP);
				_initWifiAP();
			}else{
				WiFi.setAutoReconnect(true);
			};
		}
		else if(_wlanConnection.wifiMode == wifi_mode_t::WIFI_AP_STA){
			DPRINT("Required  WiFi.mode(WIFI_AP_STA)!");
			WiFi.mode(WIFI_OFF);
			delay(100);
			WiFi.mode(WIFI_AP_STA);
			if (_initWifiStation() == false){
				DPRINTLN("Connection failed, switching to AP mode");
				WiFi.mode(WIFI_OFF);
				delay(100);
				WiFi.mode(WIFI_AP);
			}else{
				WiFi.setAutoReconnect(true);
			};
			_initWifiAP();

		}
	}else{
		DPRINTLN("Network Config Load from storage failed!");
		DPRINTLN("go straight to AP Mode");
		WiFi.mode(WIFI_AP);  //access point only....if no client connect
		_wlanConnection.wifiMode = WIFI_OFF;
		_wlanConnection.deviceName = PROJECT_NAME;
		_initWifiAP();
	}
}

void Network::handle() {
	// TODO Auto-generated constructor stub

}



bool Network::_loadNwConfigFromWebStorage()
{
	bool retVal = false;
	String values = "";
	values = _webStorage->open(DEFAULT_CONFIG_FILE);
	DPRINTLN(values);
	DPRINTLN(sizeof(values));
	//DynamicJsonDocument doc(sizeof(values));
	StaticJsonDocument<1024> doc;
	DeserializationError error = deserializeJson(doc, values);
	if (error) {
	    DPRINT("deserializeJson() failed: ");
	    DPRINTLN(error.c_str());
	    return retVal;
	};
	DPRINTLN("in loadConfigFromStorage(): network.json found ");
	if (doc.containsKey("devicename") &&
		doc.containsKey("ssid") && doc.containsKey("password") &&
		doc.containsKey("dhcp") &&
		doc.containsKey("ip_0") && doc.containsKey("ip_1") &&doc.containsKey("ip_2") && doc.containsKey("ip_3") &&
		doc.containsKey("nm_0") && doc.containsKey("nm_1") && doc.containsKey("nm_2") && doc.containsKey("nm_3") &&
		doc.containsKey("gw_0") && doc.containsKey("gw_1") && doc.containsKey("gw_2") && doc.containsKey("gw_3"))
		//doc.containsKey("mDNSoff") )
		// && root.containsKey("cDNSoff")
	{
		DPRINTLN("in loadConfigFromStorage(): network.json is valid ");
		_wlanConnection.deviceName = doc["devicename"];
		_wlanConnection.routerSSID = doc["ssid"];
		_wlanConnection.routerPassword = doc["password"];
		if (doc["dhcp"] == "1") {
			_wlanConnection.dhcpInUse = true;
		}else {
			_wlanConnection.dhcpInUse = false;
		};
		if (_wlanConnection.dhcpInUse == false){
			_wlanConnection.ipAddress[0] = doc["ip_0"]; _wlanConnection.ipAddress[1] = doc["ip_1"]; _wlanConnection.ipAddress[2] = doc["ip_2"]; _wlanConnection.ipAddress[3] = doc["ip_3"];
			_wlanConnection.netmask[0] = doc["nm_0"]; _wlanConnection.netmask[1] = doc["nm_1"]; _wlanConnection.netmask[2] = doc["nm_2"]; _wlanConnection.netmask[3] = doc["nm_3"];
			_wlanConnection.gateway[0] = doc["gw_0"]; _wlanConnection.gateway[1] = doc["gw_1"]; _wlanConnection.gateway[2] = doc["gw_2"]; _wlanConnection.gateway[3] = doc["gw_3"];
		};
		//if (String(root["mDNSoff"].asString()) == "true") _nwConnection.mDNSinUse = false; else  _nwConnection.mDNSinUse = true;
		//if (String(root["CDNSoff"].asString()) == "true") _nwConnection.cDNSinUse = false; else  _nwConnection.cDNSinUse = true;
		if (doc.containsKey("SoftAP")){
			_wlanConnection.wifiMode = (doc["SoftAP"] == "true")  ? WIFI_AP_STA : WIFI_STA;
		};
		//if (String(root["SoftAP"].asString()) == "true") _nwConnection.permanentAP = true; else  _nwConnection.permanentAP = false;
		//:WIFI_STA;(1)//:WIFI_OFF;(0)//:WIFI_AP_STA(3);//:WIFI_AP(2);
		//_nwConnection.wifiMode = (WiFiMode_t)String(root["SoftAP"].asString()).toInt();
		DPRINTLN("in loadConfigFromStorage(): all good");
			retVal = true;
		//}else{
		//	DPRINTLN("in loadConfigFromStorage(): json-format incompatible ");
		//}
	}else{
		DPRINT("in loadConfigFromStorage(): "); DPRINT(DEFAULT_CONFIG_FILE);DPRINTLN(" NOT found ");
	};
	return retVal;
}

bool Network::_initWifiStation(){
	bool retVal = true;
	delay(100);
//	PRINT("wifi.hostname: ");PRINTLN(WiFi.hostname());
	WiFi.begin(_wlanConnection.routerSSID, _wlanConnection.routerPassword);
	if(_isConnected() == true){
		//if (_nwConnection.mDNSinUse == true) {
			//TODO: check if necessary:
			//PRINTLN(WiFi.dnsIP());
  			//WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8,8,8,8));
  			//delay(10);
  			//PRINTLN(WiFi.dnsIP());
			//String name = _nwConnection.deviceName + DEFAULT_DOMAIN;
			//MDNS.begin(name.c_str());  //multicast webname when not in SoftAP mode
			DPRINTLN("normally Starting mDSN "); //PRINTLN("not Starting mDSN " + name);
			//MDNS.addService("http", "tcp", 80);
		//}
	}else{
		DPRINTLN("STA Connection failed");
		retVal = false;
	}
	return retVal;
}

void Network::_initWifiAP(){
	delay(100);
	//->no IP set, will get 192.168.4.1 anyway
	//WiFi.softAPConfig(IPAddress(DEF_AP_IP_0,DEF_AP_IP_1,DEF_AP_IP_2,DEF_AP_IP_3), IPAddress(DEF_AP_IP_0,DEF_AP_IP_1,DEF_AP_IP_2,DEF_AP_IP_3), IPAddress(255, 255, 255, 0));
	WiFi.softAP(_wlanConnection.deviceName);
	//if (_nwConnection.cDNSinUse == true) {   //if captive dns allowed
	//	startDNS();
	//}
	//TODO: add this if needed:
	//WiFi.softAP("AQS12345678", "password");
	//WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	this->_wlanConnection.ipAddress = WiFi.softAPIP();
	//this->_wlanConnection.ip[1] = WiFi.softAPIP()[1];
	//this->_wlanConnection.ip[2] = WiFi.softAPIP()[2];
	//this->_wlanConnection.ip[3] = WiFi.softAPIP()[3];
	DPRINT("Soft AP started IP address: ");
	DPRINTLN(WiFi.softAPIP());


}

bool Network::_isConnected(){
	bool retVal = false;
	unsigned long startTime = millis();
	while ((WiFi.status() != WL_CONNECTED) && ((millis() - startTime) < MAX_WIFI_CONNECT_TIME ))// ... Give ESP x seconds to connect to station.
	{
		DPRINT('.');
		// SerialLog.print(WiFi.status());
		delay(500);
		yield();
	}
	//if (WiFi.waitForConnectResult() != WL_CONNECTED)<-doesn't work well!!!
	//{
	//	PRINTLN("first check, cannot connect to: " + _nwConnection.ssid);
	//	retVal = false;
	//	//return false
	//}else{
	//	retVal = true;
	//}



	//check again if first check was unseccessful:
	if ((retVal == false) && (WiFi.status() == WL_CONNECTED))
	{
		// ... print IP Address
		DPRINT("wlan connected, IP address: ");
		DPRINTLN(WiFi.localIP());
		this->_wlanConnection.ipAddress = WiFi.localIP();
		//this->_nwConnection.ip[1] = WiFi.localIP()[1];
		//this->_nwConnection.ip[2] = WiFi.localIP()[2];
		//this->_nwConnection.ip[3] = WiFi.localIP()[3];

		retVal = true;
	}

	if ((retVal == true) && (_wlanConnection.dhcpInUse == false))	{
		WiFi.config(_wlanConnection.ipAddress,
				 	_wlanConnection.gateway,
					_wlanConnection.netmask);
					//TODO: dns can be given here!!
	}
	return retVal;
}

bool Network::connectionFailed(){
	DPRINT("required wifimode: ");DPRINTLN(_wlanConnection.wifiMode);
	DPRINT("actual wifimode: ");DPRINTLN(WiFi.getMode());
	return (((_wlanConnection.wifiMode == wifi_mode_t::WIFI_STA)|| //required some STA, but only AP available
			 (_wlanConnection.wifiMode == wifi_mode_t::WIFI_AP_STA) ||
			 (_wlanConnection.wifiMode == wifi_mode_t::WIFI_OFF)
			) && (WiFi.getMode() == wifi_mode_t::WIFI_AP));
}

