#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include "../command/Log.hpp"

#ifdef DEVKITPRO
	#include <sys/stat.h>
	#include <dirent.h>
	#include <coreinit/filesystem_fsa.h>
#endif

namespace Utility
{
	//std::filesystem is partially broken on Wii U, these are cross-platform replacements

    inline std::ostream& seek(std::ostream& stream, const std::streamoff& off, const std::ios::seekdir& way = std::ios::beg) {
        //#ifdef DEVKITPRO
		//Wii U crashes if you seek past eof, most other platforms extend the file
		//Handle writing the extra padding manually

        switch(way) {
            case std::ios::cur:
            {
                const std::streamoff& cur = stream.tellp();
                if(off > 0) {
                    stream.seekp(0, std::ios::end);
                    if(stream.tellp() < (cur + off)) {
                        const std::string buffer((cur + off) - stream.tellp(), '\0');
                        stream.write(&buffer[0], buffer.size());
                    }
                }
				else if ((cur + off) < 0) {
					//can't seek before start of file, seek to beginning as failsafe
                    return stream.seekp(0, std::ios::beg);
				}
                return stream.seekp(cur + off, std::ios::beg);
            }
            case std::ios::end:
			//BUG: seek to std::ios::end doesn't seem to work on MLC, find workaround? (relevant uses are currently replaced)
            {
				stream.seekp(0, std::ios::end);
                if(off > 0) {
                    const std::string buffer(off, '\0');
                    stream.write(&buffer[0], buffer.size());
                }
				else if((-off) > stream.tellp()) {
					//Can't seek before start of file, seek to beginning as failsafe
					return stream.seekp(0, std::ios::beg);
				}
                return stream.seekp(off, std::ios::end);
            }
			case std::ios::beg:
                [[fallthrough]];
			default:
            {
				if(off < 0) {
					//can't seek before start of file, seek to beginning as failsafe
					stream.seekp(0, std::ios::beg);
				}
                stream.seekp(0, std::ios::end);
                if(stream.tellp() < off) {
                    const std::string buffer(off - stream.tellp(), '\0');
                    stream.write(&buffer[0], buffer.size());
                }
                return stream.seekp(off, std::ios::beg);
            }
        }
        //#else
        //    return stream.seekp(off, way);
        //#endif
    }

	inline bool exists(const std::filesystem::path& fsPath) {
		#ifdef DEVKITPRO
			struct stat sbuff;
    		return stat(fsPath.string().c_str(), &sbuff) == 0;
		#else
			return std::filesystem::exists(fsPath);
		#endif
	}

	inline bool create_directories(const std::filesystem::path& fsPath) {
		#ifdef DEVKITPRO
			std::string temp = fsPath.string();
			if(temp.back() == '/') temp.pop_back();
			for(size_t i = 0; i < temp.size(); i++) {
				if(temp[i] == '/') {
					const std::string& sub = temp.substr(0, i);
                	if (!std::filesystem::is_directory(sub)) mkdir(sub.c_str(), ACCESSPERMS);
				}
			}
			mkdir(temp.c_str(), ACCESSPERMS);
		#else
			std::filesystem::create_directories(fsPath);
		#endif

		return true;
	}

	inline bool remove_all(const std::filesystem::path& fsPath) {
		#ifdef DEVKITPRO
			for(auto& p : std::filesystem::directory_iterator(fsPath)) {
				if(std::filesystem::is_directory(p.path())) {
					if(!std::filesystem::is_empty(p.path())) {
						if(!Utility::remove_all(p.path())) return false;
					}
				}
				else {
					std::filesystem::remove(p.path());
				}
			}

			if(!std::filesystem::is_empty(fsPath)) return false;
			return std::filesystem::remove(fsPath);
		#else
			return std::filesystem::remove_all(fsPath);
		#endif
	}

	bool copy_file(const std::filesystem::path& from, const std::filesystem::path& to);

	bool copy(const std::filesystem::path& from, const std::filesystem::path& to);

	int getFileContents(const std::string& filename, std::string& fileContents);

	#ifdef DEVKITPRO
    inline bool flush_mlc() {
        const int fsaHandle = FSAAddClient(NULL);
        if(fsaHandle < 0) {
            return false;
        }

        FSError ret = FSAFlushVolume(fsaHandle, "/vol/storage_mlc01");
        if(ret != FS_ERROR_OK) {
            return false;
        }

        ret = FSADelClient(fsaHandle);
        if(ret != FS_ERROR_OK) {
            return false;
        }

        return true;
    }
	#endif
}
