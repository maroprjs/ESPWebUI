/*
 * WebStorage.cpp
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#include "WebStorage.h"
#include "Debug.h"


WebStorage::WebStorage() {
	// = &SPIFFS;
	_spiffsfs = &SPIFFS;
	_fs = _spiffsfs;

}

WebStorage::~WebStorage() {
	// TODO Auto-generated destructor stub
}

void WebStorage::begin() {
	_spiffsfs->begin();

}

void WebStorage::handle() {
	// TODO

}

String WebStorage::open(String name){
	String retVal = "";

	fs::File f = SPIFFS.open(name, "r");
	if (!f) {
		DPRINT(name);DPRINTLN(" not set/found!");
		retVal = "";
	}
	else {  //file exists;
		//retVal = f.readStringUntil('\n');  //read json
		retVal = f.readString();  //read json
		f.close();
	}
	return retVal;
}

void WebStorage::loadEmbeddedFiles(String fname, const unsigned char * content, u_long numbytes) {   //if new system we save the embedded htmls into the root of Spiffs as .gz!
	_fileSaveContent_P(fname, (PGM_P)content, numbytes, true);
}

bool WebStorage::exists(String name){

	bool f = _fs->exists(name);
	if (!f) {
		DPRINT(name);DPRINTLN(" not set/found!");
	}

	return f;
}

bool WebStorage::remove(String name){

	bool f = SPIFFS.remove(name);
	if (!f) {
		DPRINT(name);DPRINTLN(" removal not succesfull !");
	}

	return f;
}

fs::File WebStorage::openDir(String name){

	fs::File f = SPIFFS.open(name);
	if (!f) {
		DPRINT(name);DPRINTLN(" not set/found!");
	}

	return f;
}

void WebStorage::_fileSaveContent_P(String fname, PGM_P content, u_long numbytes, bool overWrite ) {   //save PROGMEM array to spiffs file....//f must be already open for write!

	if (SPIFFS.exists(fname) && overWrite == false) return;


	const int writepagesize = 1024;
	char contentUnit[writepagesize + 1];
	contentUnit[writepagesize] = '\0';
	u_long remaining_size = numbytes;


	File f = SPIFFS.open(fname, "w");



	if (f) { // we could open the file

		while (content != NULL && remaining_size > 0) {
			size_t contentUnitLen = writepagesize;

			if (remaining_size < writepagesize) contentUnitLen = remaining_size;
			// due to the memcpy signature, lots of casts are needed
			memcpy_P((void*)contentUnit, (PGM_VOID_P)content, contentUnitLen);

			content += contentUnitLen;
			remaining_size -= contentUnitLen;

			// write is so overloaded, had to use the cast to get it pick the right one
			f.write((uint8_t *)contentUnit, contentUnitLen);
		}
		f.close();
		DPRINTLN("created:" + fname);
	}
}

bool WebStorage::formatFS(void){

	bool f = SPIFFS.format();
	if (!f) {
		//PRINT(name);PRINTLN(" format FS not succesfull !");
	}

	return f;
}
