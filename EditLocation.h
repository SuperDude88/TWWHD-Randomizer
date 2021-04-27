#ifndef EditLocation_H
#define EditLocation_H

#include <Structs.h>
#include <Import.h>
#include <fstream>

int EditChest(std::fstream& fptr, ACTR chest, uint8_t item_id, int offset);

int EditActor(std::fstream& fptr, ACTR actor, uint8_t item_id, int offset);

int EditScob(std::fstream& fptr, SCOB scob, uint8_t item_id, int offset);

#endif
