/*
 * ESPWebUI.cpp
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#include "ESPWebUI.h"
#include "ESPWebUIHtmlGz.h"
#include "Debug.h"
#include <SPIFFSEditor.h>
#include <ArduinoOTA.h>

ESPWebUI::ESPWebUI() {
	_webStorage = new WebStorage();
	_network = new Network(_webStorage);
	_server = new AsyncWebServer(DEFAULT_HTTP_PORT);
	_shouldReboot = false;

}

ESPWebUI::~ESPWebUI() {
	// TODO Auto-generated destructor stub
}


void ESPWebUI::begin() {
	_webStorage->begin();
	_network->begin();
	//_webStorage->loadEmbeddedFiles("/setup.html.gz", PAGE_SETUP, sizeof(PAGE_SETUP));
	//_webStorage->loadEmbeddedFiles("/filebrowse.html.gz", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
    _webStorage->loadEmbeddedFiles("/jquery-1.9.1.min.js.gz", PAGE_JQUERY_1_9_1, sizeof(PAGE_JQUERY_1_9_1));
    _defineESPWebUIHtmlAPI();
	#ifdef ESP32
    	_server->addHandler(new SPIFFSEditor(*_webStorage->fs(),"",""));
	#elif defined(ESP8266)
    	_server->addHandler(new SPIFFSEditor("","",*_webStorage->fs()));
	#endif
    _server->begin();

}

void ESPWebUI::handle() {

	if (_shouldReboot) {
		Serial.println("Rebooting...");
		delay(100);
		ESP.restart();
	}
	if (_jSONCallBack != "") {
		_jsonSaveHandle(_jSONCallBack);  //async last json file save;
		_jSONCallBack = "";
	}

}

void ESPWebUI::_defineESPWebUIHtmlAPI()
{
	//-------------------------------------------------------------------
	//_server->on("/", HTTP_ANY, std::bind(&ESPWebUI::_handleRoot, this, std::placeholders::_1));
	_server->on("/", [this](AsyncWebServerRequest *request) {
		_handleRoot( request) ;
	});
	//-------------------------------------------------------------------
	_server->onNotFound([this](AsyncWebServerRequest *request) {
		_onRequest( request) ;
	});
	_server->onFileUpload([this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
		_onUpload( request, filename, index, data, len, final) ;
	});
	_server->onRequestBody([this](AsyncWebServerRequest *request, uint8_t *data, size_t index, size_t len, size_t total) {
		//onBody( request, data, len, index,total) ;
		DPRINTLN("onbody");
	});
	//-------------------------------------------------------------------
	//_server->on("/browse", HTTP_ANY, std::bind(&ESPWebUI::_handleFileBrowser, this, std::placeholders::_1));
	_server->on("/browse", [this](AsyncWebServerRequest *request) {
		_handleFileBrowser( request) ;
	});
	//-------------------------------------------------------------------
	//_server->on("/jsonsave", std::bind(&ESPWebUI::handleJsonSave, this));
	_server->on("/jsonsave", [this](AsyncWebServerRequest *request) {
		DPRINT("jsonsave called");
		_handleJsonSave( request) ;
	});
	//-------------------------------------------------------------------
	//_server->on("/jsonload", std::bind(&ESPWebUI::handleJsonLoad, this));
		_server->on("/jsonload", [this](AsyncWebServerRequest *request) {
		_handleJsonLoad( request) ;
	});
	//-------------------------------------------------------------------
	//_server->on("/formatspiff", std::bind(&ESPWebUI::formatspiffs, this));
	_server->on("/formatfs", [this](AsyncWebServerRequest *request) {
		_formatFs( request) ;
	});
	//-------------------------------------------------------------------
	//_server->on("/restartesp", std::bind(&ESPWebUI::restartESP, this));
	_server->on("/restartesp", [this](AsyncWebServerRequest *request) {
		_restartDevice( request) ;
	});
	//-------------------------------------------------------------------
	_server->on("/flashupdate", HTTP_POST, [this](AsyncWebServerRequest *request) {
		_shouldReboot = !Update.hasError(); //TODO
		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _shouldReboot ? "OK" : "FAIL"); //TODO
		//AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
		response->addHeader("Connection", "close");
		request->send(response);
	}, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
		if (!index) {
			DPRINT("Update Start: ");DPRINTLN(filename.c_str());
			//Serial.printf("Update Start: %s\n", filename.c_str());
			//Update.runAsync(true);<-TODO: is that needed?
			if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {//TODO: uint32_t maxSketchSpace = 0x140000;//https://github.com/bbx10/WebServer_tng/issues/4
				Update.printError(Serial);
			}
		}
		if (!Update.hasError()) {
			if (Update.write(data, len) != len) {
				Update.printError(Serial);//TODO: serial?
			}
		}
		if (final) {
			if (Update.end(true)) {
				Serial.printf("Update Success: %uB\n", index + len);
			}
			else {
				Update.printError(Serial);//TODO: serial?
			}
		}
	});
	//-------------------------------------------------------------------
	_server->on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	//-------------------------------------------------------------------
	//_server->on("/availnets", std::bind(&ESPWebUI::sendAvailNetworks, this));
	_server->on("/availnets", [this](AsyncWebServerRequest *request) {
		DPRINTLN("availnets ");
		_sendAvailNetworks( request) ;
	});
	//_server->on("/info.html", std::bind(&ESPWebUI::sendNetworkStatus, this));
	_server->on("/info.html", [this](AsyncWebServerRequest *request) {
		_sendNetworkStatus( request) ;
	});
	_server->on("/scanwifi", HTTP_GET, [](AsyncWebServerRequest *request) {   //async scan we need to start scan before getting availnetworks after a delay....
		WiFi.scanDelete();
		WiFi.scanNetworks(true);
		request->send(200, "text/html", "");

	});
	//------

}

void ESPWebUI::_handleRoot(AsyncWebServerRequest *request) {  //handles root of website (used in case of virgin systems.)
	DPRINTLN("handel root");
	DPRINTLN(request->url());
	String pathWithGz = DEFAULT_ROOT_PATH + ".gz";
	if (_webStorage->exists(DEFAULT_ROOT_PATH)){ //= "/index.html"
		request->send(*_webStorage->fs(), DEFAULT_ROOT_PATH);
	}else if (_webStorage->exists(pathWithGz)){
		AsyncWebServerResponse *response = request->beginResponse(*_webStorage->fs(), pathWithGz);
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
	}else if (_network->connectionFailed()){
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", PAGE_SETUP, sizeof(PAGE_SETUP));
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
	}else{
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
	}
}

void ESPWebUI::_onRequest(AsyncWebServerRequest *request) {
	DPRINT("Handle Unknown Request ");DPRINTLN(request->url());
	if (!_handleFileRead(request, request->url())){
		DPRINTLN("unknown request send errror 404 ");
		request->send(404, "text/plain", " FileNotFound " + request->url());
	}
}


bool ESPWebUI::_handleFileRead(AsyncWebServerRequest *request, String path)
{
	DPRINTLN("handleFileRead: " + path);
	bool success = true;
	path = request->urlDecode(path);
	String pathWithGz = path + ".gz";
	bool download = (request->arg("download") == "true");
	if (!download)
	{
		if (_webStorage->exists(path))  //set cache
		{
			DPRINTLN("file exists ");
			request->send(*_webStorage->fs(), path);

		}else if (_webStorage->exists(pathWithGz)) {
			DPRINTLN("file exists as gz");
			AsyncWebServerResponse *response = request->beginResponse(*_webStorage->fs(), pathWithGz, _getContentType(path));
			response->addHeader("Content-Encoding", "gzip");
			//response->addHeader("Access-Control-Allow-Origin", "*");//always add '*'?
			//response->addHeader("Cache-Control", "max-age=86400");//set cache header here....
			request->send(response);
		}else{
			//request->send(404, "text/plain", "FileNotFound");
			success = false;
		}
	} else {    //download
			if (_webStorage->exists(path))  //set cache
			{
				DPRINTLN("file exists - downloding");
				AsyncWebServerResponse *response = request->beginResponse(*_webStorage->fs(), path, String(), true);
				response->addHeader("Access-Control-Allow-Origin", "*");//always add '*'?
				//response->addHeader("Cache-Control", "max-age=86400");//set cache header here....
				request->send(response);

			}else if (_webStorage->exists(pathWithGz)) {
				DPRINTLN("filebrowse exists as gz - downloding");
				AsyncWebServerResponse *response = request->beginResponse(*_webStorage->fs(), pathWithGz, _getContentType(path), true);
				response->addHeader("Content-Encoding", "gzip");
				response->addHeader("Access-Control-Allow-Origin", "*");//always add '*'?
				//response->addHeader("Cache-Control", "max-age=86400");//set cache header here....
				request->send(response);
			}else{
				//request->send(404, "text/plain", "FileNotFound");
				success = false;
			};

	};
	return success;

}

void ESPWebUI::_handleFileBrowser(AsyncWebServerRequest *request)
{
	DPRINTLN("handleFileBrowser");
	if (request->arg("do") == "list") {
		DPRINTLN("list");
		_handleFileList(request);
	}else if (request->arg("do") == "delete") {
			DPRINTLN("delete");
			_handleFileDelete(request,request->arg("file"));
	}else{
		//_handleFileRead(request,"/filebrowse.html");
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
	}
}

void ESPWebUI::_handleFileList(AsyncWebServerRequest *request)
{
	File root = _webStorage->openDir("/");
	String output = "{\"success\":true, \"is_writable\" : true, \"results\" :[";
	bool firstrec = true;
	if(!root){
        DPRINTLN("- failed to open directory");
		//serverLog("- failed to open directory");
        //return;
    }
    if(!root.isDirectory()){
        DPRINTLN(" - not a directory");
        //return;
    }
	File dir = root.openNextFile();
	DPRINTLN(dir);
	//while (dir.next()) {
	while (dir) {
		DPRINTLN("nextFileOpen");
		if (!firstrec) { output += ','; }  //add comma between recs (not first rec)
		else {
			firstrec = false;
		}
		String fn = dir.name();
		DPRINTLN(fn);
		fn.remove(0, 1); //remove slash
		output += "{\"is_dir\":false";
		output += ",\"name\":\"" + fn;
		output += "\",\"size\":" + String(dir.size());
		output += ",\"path\":\"";
		output += "\",\"is_deleteable\":true";
		output += ",\"is_readable\":true";
		output += ",\"is_writable\":true";
		output += ",\"is_executable\":true";
		output += ",\"mtime\":1452813740";   //dummy time
		output += "}";
		dir = root.openNextFile();
	}
	output += "]}";
	DPRINTLN("got list >"+output);
	request->send(200, "text/json", output);
}

void ESPWebUI::_handleFileDelete(AsyncWebServerRequest *request,String fname)
{
	DPRINTLN("handleFileDelete: " + fname);
	fname = '/' + fname;
	fname = request->urlDecode(fname);
	if (!_webStorage->exists(fname)) //TODO
		return request->send(404, "text/plain", "FileNotFound");
	if (_webStorage->exists(fname))
	{
		_webStorage->remove(fname);
		request->send(200, "text/plain", "");
	}
}

String ESPWebUI::_getContentType(String filename)
{
	if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}


void ESPWebUI::_formatFs(AsyncWebServerRequest *request)
{
	DPRINTLN("formatting spiff...");
	request->send(200, "text/html", "Format Started....rebooting");
	if (!_webStorage->formatFS()) {
		DPRINTLN("Format failed");
		//TODO:write to log
	}
	else { DPRINTLN("format done...."); }

}

void ESPWebUI::_onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	//Handle upload
	DPRINTLN("handel upload");
	if (!index) {
		if (!filename.startsWith("/")) filename = "/" + filename;
		DPRINT("handleFileUpload Name: "); 	DPRINTLN(filename);
		_fsUploadFile = _webStorage->fs()->open(filename, "w");	//TODO: wrap this in storage
	}
	_fsUploadFile.write(data, len);
	if (final) {
		_fsUploadFile.close();
		DPRINT("UploadEnd file: ");DPRINT(filename.c_str());
		//Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
	}

}

void ESPWebUI::_restartDevice(AsyncWebServerRequest *request) {
	request->send(200, "text/plain", "Restarting Device...");
	_shouldReboot = true;
}

void ESPWebUI::_handleJsonSave(AsyncWebServerRequest *request)
{
	//new must have JS in it!
	DPRINT(request->url());
	int args = request->args();
	for(int i=0;i<args;i++){
	  DPRINT( request->argName(i).c_str());DPRINTLN(request->arg(i).c_str());
	}
	if (!request->hasArg("js") || !request->hasArg("f"))
		return request->send(500, "text/plain", "BAD JsonSave ARGS");  //must have f and js

	String fname = "/" + request->arg("f");
	fname = request->urlDecode(fname);
	DPRINT("ard f recognized....file: ");DPRINTLN(fname);
	File file = _webStorage->fs()->open(fname, "w"); //TODO: wrap this
	if (file) {
		file.println(request->arg("js"));  //save json data
		file.close();
	}
	else  //cant create file
		return request->send(500, "text/plain", "JSONSave FAILED");
	request->send(200, "text/plain", "");
	DPRINTLN("handleJsonSave: " + fname);
	if (_jsonSaveHandle != NULL) {
		_jSONCallBack=(fname);
	} else {
		_jSONCallBack = "";
	}
}

void ESPWebUI::_handleJsonLoad(AsyncWebServerRequest *request)
{

	if (!request->hasArg("f")){
		return request->send(500, "text/plain", "BAD JsonSave ARGS");  //must have f
	};

	String fname =  request->arg("f");
	fname = "/" + request->urlDecode(fname);
	DPRINTLN("handleJsonRead: " + fname);


	DPRINTLN("handleJsonRead: " + fname);
	_handleFileRead(request,fname);
}

void ESPWebUI::_sendAvailNetworks(AsyncWebServerRequest *request) {
	int n = WiFi.scanComplete();
	DPRINTLN(n);
	if (n == -2) {
		WiFi.scanNetworks(true);
		DPRINTLN("n -2 ");
	}
	else {
		String postStr = "{ \"Networks\":[";    //build json string of networks available
		if (n == 0)
		{
			postStr += "{\"ssidname\":\"No networks found yet!\",\"qual\":\"0\",\"sec\":\" \" }";
		}
		else
		for (int i = 0; i < n; ++i) {
			int quality = 0;
			if (WiFi.RSSI(i) <= -100)
			{
				quality = 0;
			}
			else if (WiFi.RSSI(i) >= -50)
			{
				quality = 100;
			}
			else
			{
				quality = 2 * (WiFi.RSSI(i) + 100);
			}
			postStr += "{\"ssidname\":\"" + String(WiFi.SSID(i)) + "\",\"qual\":\"" + String(quality) + "\",\"sec\":\"" + String((WiFi.encryptionType(i) ==  WIFI_AUTH_OPEN/*ENC_TYPE_NONE*/) ? " " : "*") + "\"},";
		}
		if (postStr.charAt(postStr.length() - 1) == ',') postStr.remove(postStr.length() - 1, 1);
		postStr += "] }";  //finish json array
		DPRINTLN(postStr);
		request->send(200, "application/json", postStr);
		WiFi.scanDelete();
		if (WiFi.scanComplete() == -2) {
			WiFi.scanNetworks(true);
		}
	}
}

void ESPWebUI::_sendNetworkStatus(AsyncWebServerRequest *request)
{

	uint8_t mac[6];
	char macStr[18] = { 0 };
	WiFi.macAddress(mac);
	sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	String state = "N/A";
	String Networks = "";
	if (WiFi.status() == 0) state = "Idle";
	else if (WiFi.status() == 1) state = "NO SSID AVAILBLE";
	else if (WiFi.status() == 2) state = "SCAN COMPLETED";
	else if (WiFi.status() == 3) state = "CONNECTED";
	else if (WiFi.status() == 4) state = "CONNECT FAILED";
	else if (WiFi.status() == 5) state = "CONNECTION LOST";
	else if (WiFi.status() == 6) state = "DISCONNECTED";

	Networks = "";  //future to scan and show networks async

	String wifilist = "";
	wifilist += "WiFi State: " + state + "<br>";
	wifilist += "Scanned Networks <br>" + Networks + "<br>";

	String values = "";
	values += "<body> SSID          :" + (String)WiFi.SSID() + "<br>";
	values += "IP Address     :   " + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "<br>";
	values += "Wifi Gateway   :   " + (String)WiFi.gatewayIP()[0] + "." + (String)WiFi.gatewayIP()[1] + "." + (String)WiFi.gatewayIP()[2] + "." + (String)WiFi.gatewayIP()[3] + "<br>";
	values += "NetMask        :   " + (String)WiFi.subnetMask()[0] + "." + (String)WiFi.subnetMask()[1] + "." + (String)WiFi.subnetMask()[2] + "." + (String)WiFi.subnetMask()[3] + "<br>";
	values += "Mac Address    >   " + String(macStr) + "<br>";
	//values += "NTP Time       :   " + String(_timeClient->getHours()) + ":" + String(_timeClient->getMinutes()) + ":" + String(_timeClient->getSeconds()) + " " + String(_timeClient->getEpochTime()) + "-" + String(_timeClient->getDay()) + "<br>";
	values += "Server Uptime  :   " + String(millis() / 60000) + " minutes" + "<br>";
	values += "Server Heap  :   " + String(ESP.getFreeHeap()) + "<br>";
	values += wifilist;
	values += " <input action=\"action\" type=\"button\" value=\"Back\" onclick=\"history.go(-1);\" style=\"width: 100px; height: 50px;\" /> </body> ";
	request->send(200, "text/html", values);
}

