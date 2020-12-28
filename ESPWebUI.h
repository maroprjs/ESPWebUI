/*
 * ESPWebUI.h
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#ifndef ESPWEBUI_H_
#define ESPWEBUI_H_


#include "Network.h"
#include "WebStorage.h"
#include <ESPAsyncWebServer.h>

class ESPWebUI {
	uint16_t const DEFAULT_HTTP_PORT = 80;
	String const DEFAULT_ROOT_PATH = "/index.html";
public:
	ESPWebUI();
	virtual ~ESPWebUI();
	void begin();
	void handle();
	AsyncWebServer* http() {return _server;};
	Network* network() {return _network;};

private:
	void _defineESPWebUIHtmlAPI();
	void _handleRoot(AsyncWebServerRequest *request);
	bool _handleFileRead(AsyncWebServerRequest *request, String path);
	void _onRequest(AsyncWebServerRequest *request);
	void _handleFileBrowser(AsyncWebServerRequest *request);
	void _handleFileList(AsyncWebServerRequest *request);
	void _handleFileDelete(AsyncWebServerRequest *request,String fname);
	String _getContentType(String filename);
	void _formatFs(AsyncWebServerRequest *request);
	void _onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
	void _restartDevice(AsyncWebServerRequest *request);
	void(*_jsonSaveHandle)(String fname) = NULL;  //callback function when json form is saved...fname is the filename saved.
	void _handleJsonSave(AsyncWebServerRequest *request);
	void _handleJsonLoad(AsyncWebServerRequest *request);
	void _sendAvailNetworks(AsyncWebServerRequest *request);
	void _sendNetworkStatus(AsyncWebServerRequest *request);

	WebStorage * _webStorage;
	Network * _network;
	AsyncWebServer * _server;
	File _fsUploadFile;
	bool _shouldReboot;
	String _jSONCallBack;  //async json callback in handle;.



};

#endif /* ESPWEBUI_H_ */
