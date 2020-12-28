/*
 * WebStorage.h
 *
 *  Created on: 7 Oct 2020
 *      Author: maro
 */

#ifndef WEBSTORAGE_H_
#define WEBSTORAGE_H_
#include <FS.h>
#include "SPIFFS.h"

class WebStorage {
public:
	WebStorage();
	virtual ~WebStorage();
	void begin();
	void handle();
	String open(String name);
	void loadEmbeddedFiles(String fname, const unsigned char * content, u_long numbytes);
	fs::FS * fs() {return _fs;};
	bool exists(String name);
	bool remove(String name);
	fs::File openDir(String name);
	bool formatFS(void);
private:
	void _fileSaveContent_P(String fname, PGM_P content, u_long numbytes, bool overWrite = false);

	fs::FS * _fs;
	fs::SPIFFSFS * _spiffsfs;
};

#endif /* WEBSTORAGE_H_ */
