#include "file.hpp"
#include <thread>
#include <csignal>
#include <mutex>

namespace Utility {
	//IMPROVEMENT: better way to make these thread-safe?
	static constexpr int FILE_BUF_SIZE = 4*1024*1024;
	static char buf[FILE_BUF_SIZE];
	static std::mutex bufMutex;
	bool copy_file(const std::filesystem::path& from, const std::filesystem::path& to) {
		Utility::platformLog("Copying %s\n", to.string().c_str());
		#ifdef DEVKITPRO
			//use a buffer to speed up file copying

			std::ifstream src(from, std::ios::binary);
        	std::ofstream dst(to, std::ios::binary);
			if(!src.is_open()) {
				Utility::platformLog(from.string() + " not opened\n");
				return false;
			}
			if(!dst.is_open()) {
				Utility::platformLog(to.string() + " not opened\n");
				return false;
			}

			std::unique_lock<std::mutex> lock(bufMutex);
			while(src) {
				src.read(buf, FILE_BUF_SIZE);
				dst.write(buf, src.gcount());
			}
			return true;
		#else
			return std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
		#endif
	}

	bool copy(const std::filesystem::path& from, const std::filesystem::path& to) {
		#ifdef DEVKITPRO
			std::error_code err;
			for(std::filesystem::directory_iterator p(from), end; p != end;) {
				std::string srcPath = p->path().string();
				std::string relPath = srcPath.substr(srcPath.find(from.string()) + from.string().size() + 1); //would use std::filesystem::relative but link fails for dkp
				if(std::filesystem::is_directory(p->path())) {
					if(!Utility::create_directories(to / relPath)) return false;
					if(!Utility::copy(p->path(), to / relPath)) return false;
				}
				else if (std::filesystem::is_regular_file(p->path())) {
					if(!Utility::copy_file(p->path(), to / relPath)) return false;
				}

				p.increment(err);
			}
			
		#else
			std::filesystem::copy(from, to, std::filesystem::copy_options::recursive);
		#endif
		
		return true;
	}
}
