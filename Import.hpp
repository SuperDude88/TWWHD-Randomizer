#define Import_H
#pragma once

#include <fstream>
#include <cstdint>

ACTR ReadChest(std::fstream& fptr, int offset);

ACTR ReadActor(std::fstream& fptr, int offset);

SCOB ReadScob(std::fstream& fptr, int offset);

