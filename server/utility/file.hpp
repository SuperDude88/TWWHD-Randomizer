#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "platform.hpp"

#ifdef DEVKITPRO
#include <sys/stat.h>
#endif

namespace Utility
{
	//std::filesystem is partially broken on Wii U, these are cross-platform replacements

	//BUG: seek to std::ios::end doesn't seem to work on MLC, find workaround? (relevant uses are currently replaced)
    inline std::ostream& seek(std::ostream& stream, const std::streamoff& pos, const std::ios::seekdir& dir = std::ios::beg) {
        #ifdef DEVKITPRO
        switch(dir) {
            case std::ios::cur:
            {
                const std::streamoff cur = stream.tellp();
                if(pos > 0) {
                    stream.seekp(0, std::ios::end);
                    if(stream.tellp() < (cur + pos)) {
                        const std::string buffer((cur + pos) - stream.tellp(), '\0');
                        stream.write(&buffer[0], buffer.size());
                    }
                }
                return stream.seekp(cur + pos, std::ios::beg);
            }
            case std::ios::end:
            {
                if(pos > 0) {
                    const std::string buffer(pos, '\0');
                    stream.write(&buffer[0], buffer.size());
                }
                return stream.seekp(pos, std::ios::end);
            }
			case std::ios::beg:
                [[fallthrough]];
			default:
            {
                stream.seekp(0, std::ios::end);
                if(stream.tellp() < pos) {
                    const std::string buffer(pos - stream.tellp(), '\0');
                    stream.write(&buffer[0], buffer.size());
                }
                return stream.seekp(pos, std::ios::beg);
            }
        }
        #else
            return stream.seekp(pos, dir);
        #endif
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
			const char* path = fsPath.string().c_str();

			//borrowed from https://github.com/emiyl/dumpling/blob/master/source/app/filesystem.cpp#L236
			char tmp[PATH_MAX];
    		char *p = NULL;
    		size_t len;
    		std::snprintf(tmp, sizeof(tmp), "%s", path);
    		len = std::strlen(tmp);
    		if (tmp[len-1] == '/') tmp[len-1] = 0;
    		for(p = tmp+1; *p; p++) {
    		    if (*p == '/') {
    		        *p = 0;
    		        mkdir(tmp, ACCESSPERMS);
    		        *p = '/';
    		    }
    		}
    		mkdir(tmp, ACCESSPERMS);
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
}
