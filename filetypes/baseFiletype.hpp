#pragma once
#include <typeinfo>
#include <sstream>

class FileType {    
    //static_assert(std::is_enum_v<error_enum>, "error_enum must be an enum type");
public:
    FileType() = default;
    virtual ~FileType() = default;

	//virtual error_enum loadFromBinary(std::istream&) = 0;
	//virtual error_enum loadFromFile(const std::string&) = 0;
	//virtual error_enum writeToStream(std::ostream&) = 0;
	//virtual error_enum writeToFile(const std::string&) = 0;
protected:
	virtual void initNew() = 0;
};

class GenericFile : public FileType {
public:
    std::stringstream data;

    GenericFile() = default;
    explicit GenericFile(const std::string& data_) :
        data(data_)
    {}
private:
    void initNew() override {}
};
