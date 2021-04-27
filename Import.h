#ifndef Import_H
#define Import_H

#include <fstream>
#include <cstdint>

ACTR ReadChest(std::fstream& fptr, int offset);

ACTR ReadActor(std::fstream& fptr, int offset);

SCOB ReadScob(std::fstream& fptr, int offset);

#endif

