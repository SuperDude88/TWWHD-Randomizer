#ifndef EditLocation_H
#define EditLocation_H

#include <Structs.h>
#include <Import.h>
#include <fstream>

int EditChest(std::fstream& fptr, ACTR chest, uint8_t item_id, int offset);

#endif