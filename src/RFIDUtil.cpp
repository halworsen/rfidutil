#include "RFIDUtil.h"
#include <Arduino.h>
#include <MFRC522.h>

#define GetSectorFromBlock(B) (int)(B/4)
#define BLOCKS_IN_SECTOR 4
#define BYTES_IN_BLOCK 16

inline void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

bool RFIDUtil::ReadCard(){
  // look for a new card
  if(!reader->PICC_IsNewCardPresent())
      return false;

  // read the tag
  if(!reader->PICC_ReadCardSerial())
      return false;

  return true;
}

bool RFIDUtil::ReadBlock(byte block, byte *result, MFRC522::MIFARE_Key *read_key, bool authenticated=false){
	// authenticate with the sector if needed
	if(!authenticated){
		Authenticate(block, true, read_key);
	}

	// read the block
	// +2 for CRC bytes
	byte buffer[BYTES_IN_BLOCK+2];
	byte size = sizeof(buffer);

	status = reader->MIFARE_Read(block, buffer, &size);
	if(status != MFRC522::STATUS_OK){
		if(Serial){
			Serial.println("[RFIDUtil] Failed to read block");
			Serial.println(reader->GetStatusCodeName(status));
		}
		return false;
	}

	// copy over the block data except for the CRC
	for(int i = 0; i < BYTES_IN_BLOCK; i++){
		result[i] = buffer[i];
	}
	result[BYTES_IN_BLOCK] = 0x01; // indicate success

	return true;
}

bool RFIDUtil::WriteBlock(byte block, byte *bytes, byte size, MFRC522::MIFARE_Key *write_key, bool authenticated=false){
	// authenticate with the sector if needed
	if(!authenticated){
		Authenticate(block, false, write_key);
	}

    status = (MFRC522::StatusCode) reader->MIFARE_Write(block, bytes, size);
    if(status != MFRC522::STATUS_OK){
      Serial.println("[RFIDUtil] Failed to write to block");
      Serial.println(reader->GetStatusCodeName(status));
      return false;
    }

    return true;
}

bool RFIDUtil::Authenticate(byte block, bool a_first, MFRC522::MIFARE_Key *key){
	byte sector = GetSectorFromBlock(block);
	byte trailer_block = ((sector+1) * BLOCKS_IN_SECTOR) - 1;
	byte command;

	command = MFRC522::PICC_CMD_MF_AUTH_KEY_B;
	if(a_first)
		command = MFRC522::PICC_CMD_MF_AUTH_KEY_A;

	status = (MFRC522::StatusCode) reader->PCD_Authenticate(command, trailer_block, key, &(reader->uid));
	if(status != MFRC522::STATUS_OK){
		// try again by authing against the other key
		command = MFRC522::PICC_CMD_MF_AUTH_KEY_A;
		if(a_first)
			command = MFRC522::PICC_CMD_MF_AUTH_KEY_B;

		status = (MFRC522::StatusCode) reader->PCD_Authenticate(command, trailer_block, key, &(reader->uid));
		if(status != MFRC522::STATUS_OK){
			if(Serial){
				Serial.println("[RFIDUtil] Failed to authenticate against sector");
				Serial.println(reader->GetStatusCodeName(status));
			}
			return false;
		}
	}

	return true;
}