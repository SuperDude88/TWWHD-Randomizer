#include "file.hpp"

#include <mutex>
#include <cstring>
#include <utility/platform.hpp>



namespace Utility {
	//IMPROVEMENT: better way to make these thread-safe?
	#ifdef DEVKITPRO
		static constexpr int FILE_BUF_SIZE = 4*1024*1024;
		static char buf[FILE_BUF_SIZE];
	#endif
	static std::mutex bufMutex;
	bool copy_file(const std::filesystem::path& from, const std::filesystem::path& to) {
		Utility::platformLog("Copying %s\n", to.string().c_str());
		#ifdef DEVKITPRO
			//use a buffer to speed up file copying

			std::ifstream src(from, std::ios::binary);
        	std::ofstream dst(to, std::ios::binary);
			if(!src.is_open()) {
				ErrorLog::getInstance().log("Failed to open " + from.string());
				return false;
			}
			if(!dst.is_open()) {
				ErrorLog::getInstance().log("Failed to open " + to.string());
				return false;
			}

			std::unique_lock<std::mutex> lock(bufMutex);
			while(src) {
				src.read(buf, FILE_BUF_SIZE);
				dst.write(buf, src.gcount());
			}
			return true;
		#else
			// GNU on windows currently has a bug where you can't copy over a file that already exists
			// even if you pass std::filesystem::copy_options::overwrite_existing. So delete the copy location
			// file in this case
			#if defined(WIN32) && defined(__GNUG__)
				std::filesystem::remove(to);
			#endif
			return std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
		#endif
	}

	bool copy(const std::filesystem::path& from, const std::filesystem::path& to) {
		#ifdef DEVKITPRO
			//based on https://github.com/emiyl/dumpling/blob/12935ede46e9720fdec915cdb430d10eb7df54a7/source/app/dumping.cpp#L208

			DIR* dirHandle;
    		if ((dirHandle = opendir(from.string().c_str())) == nullptr) {
    		    ErrorLog::getInstance().log("Couldn't open directory to copy files from: " + to.string());
    		    return false;
    		}

    		Utility::create_directories(to);

    		// Loop over directory contents
    		struct dirent* dirEntry;
    		while ((dirEntry = readdir(dirHandle)) != nullptr) {
    		    const std::string entrySrcPath = from / dirEntry->d_name;
    		    const std::string entryDstPath = to / dirEntry->d_name;

    		    // Use lstat since readdir returns DT_REG for symlinks
    		    struct stat fileStat;
    		    if (lstat(entrySrcPath.c_str(), &fileStat) != 0) {
    		    	ErrorLog::getInstance().log("Couldn't check what type this file/folder was: " + entrySrcPath);
					return false;
    		    }

    		    if (S_ISLNK(fileStat.st_mode)) {
    		        continue;
    		    }
    		    else if (S_ISREG(fileStat.st_mode)) {
    		        // Copy file
    		        if (!copy_file(entrySrcPath, entryDstPath)) {
    		    		ErrorLog::getInstance().log("Failed to copy file: " + entrySrcPath);
    		            closedir(dirHandle);
    		            return false;
    		        }
    		    }
    		    else if (S_ISDIR(fileStat.st_mode)) {
    		        // Ignore root and parent folder entries
    		        if (std::strncmp(dirEntry->d_name, ".", 1) == 0 || std::strncmp(dirEntry->d_name, "..", 2) == 0) continue;

    		        // Copy all the files in this subdirectory
    		        if (!copy(entrySrcPath, entryDstPath)) {
    		    		ErrorLog::getInstance().log("Failed to copy dir: " + entrySrcPath);
    		            closedir(dirHandle);
    		            return false;
    		        }
    		    }
    		}

    		closedir(dirHandle);
		#else
			std::filesystem::copy(from, to, std::filesystem::copy_options::recursive);
		#endif

		return true;
	}

	// Short function for getting the string data from a file
	int getFileContents(const std::string& filename, std::string& fileContents)
	{
	    std::ifstream file(filename);
	    if (!file.is_open())
	    {
	        ErrorLog::getInstance().log("unable to open file \"" + filename + "\"");
	        return 1;
	    }

	    // Read and load file contents
	    auto ss = std::ostringstream{};
	    ss << file.rdbuf();
	    fileContents = ss.str();

	    return 0;
	}
}
