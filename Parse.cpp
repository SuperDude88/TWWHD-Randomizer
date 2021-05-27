#include <fstream>
#include <vector>
#include <string>
#include "json.hpp"
#include "Structs.hpp"
#include <iostream>
#include "Parse.hpp"

std::vector<Location> ParseLocations(std::string LocationsPath) {

	std::ifstream fptr;
	fptr.open(LocationsPath, std::ios::in);

	std::vector<Location> Locations;
	
	//add section to read logical requirements 
	std::cout << "about to parse locations json" << std::endl;

	nlohmann::json jsondata = nlohmann::json::parse(fptr);
	for (auto& array : jsondata["Locations"]) {
		Location location;
		location.Name = array["Name"];
		location.Item = array["Original Item"];

		for (unsigned int i = 0; i < (array["Category"].size()); i++) {
			location.Category.push_back(array["Category"][i]);
		}

		location.Needs = array["Needs"];

		location.Path = array["Path"];
		location.Type = array["Type"];

		for (unsigned int i = 0; i < (array["Offset"].size()); i++) {
			location.Offsets.push_back(array["Offset"][i]);
		}

		if (location.Type == "Event") {
			location.Extra = array["NameOffset"];
		}

		Locations.push_back(location);
	}
	return Locations;
}

std::vector<Macro> ParseMacros(std::string MacrosPath) {

	std::ifstream fptr;
	fptr.open(MacrosPath, std::ios::in);

	std::vector<Macro> Macros;

	nlohmann::json jsondata = nlohmann::json::parse(fptr);
	for (auto& array : jsondata["Macros"]) {
		Macro macro;
		macro.Name = array["Name"];
		macro.Expression = array["Expression"];
		Macros.push_back(macro);
	}

	return Macros;

}