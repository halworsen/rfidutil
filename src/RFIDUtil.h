#ifndef RFIDUtil_h
#define RFIDUtil_h

#include <Arduino.h>
#include <MFRC522.h>

class RFIDUtil{
public:
	MFRC522::StatusCode status;

	RFIDUtil(MFRC522 *_reader) : reader(_reader) {};

	bool ReadCard();
	bool ReadBlock(byte block, byte *result, MFRC522::MIFARE_Key *read_key, bool authenticated=false);
	bool WriteBlock(byte block, byte *bytes, byte size, MFRC522::MIFARE_Key *write_key, bool authenticated=false);

private:
	const MFRC522 *reader;

	bool Authenticate(byte block, bool a_first, MFRC522::MIFARE_Key *key);	
};

#endif