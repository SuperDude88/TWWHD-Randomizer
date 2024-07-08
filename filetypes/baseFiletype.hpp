#pragma once

#include <typeinfo>
#include <sstream>

#include <utility/path.hpp>

class FileType {    
    //static_assert(std::is_enum_v<error_enum>, "error_enum must be an enum type");
public:
    FileType() = default;
    virtual ~FileType() = default;

	//virtual error_enum loadFromBinary(std::istream&) = 0;
	//virtual error_enum loadFromFile(const fspath&) = 0;
	//virtual error_enum writeToStream(std::ostream&) = 0;
	//virtual error_enum writeToFile(const fspath&) = 0;
protected:
	virtual void initNew() = 0;
};

class RawFile final : public FileType {
public:
    std::stringstream data;

    RawFile() = default;
    explicit RawFile(const std::string& data_) :
        data(data_)
    {}
private:
    void initNew() override {}
};
