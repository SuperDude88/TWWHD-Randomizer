#include "file.hpp"
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

	#ifdef DEVKITPRO
	void createPath(const char* path) {
		//based on https://github.com/emiyl/dumpling/blob/42048cb966c04c5fa1c1a54b1f7013b5143f081a/source/app/filesystem.cpp#L236

	    char tmp[PATH_MAX];
	    char *p = NULL;
	    size_t len;
	    snprintf(tmp, sizeof(tmp), "%s", path);
	    len = strlen(tmp);
	    if (tmp[len-1] == '/') tmp[len-1] = 0;
	    for(p = tmp+1; *p; p++) {
	        if (*p == '/') {
	            *p = 0;
	            mkdir(tmp, ACCESSPERMS);
	            *p = '/';
	        }
	    }
	    mkdir(tmp, ACCESSPERMS);
	}
	#endif

	bool copy(const std::filesystem::path& from, const std::filesystem::path& to) {
		#ifdef DEVKITPRO
			//based on https://github.com/emiyl/dumpling/blob/12935ede46e9720fdec915cdb430d10eb7df54a7/source/app/dumping.cpp#L208

			std::string srcPath = from.string();
			std::string destPath = to.string();

			DIR* dirHandle;
    		if ((dirHandle = opendir(srcPath.c_str())) == nullptr) {
    		    Utility::platformLog("Couldn't open directory to copy files from:\n");
    		    Utility::platformLog("%s\n", destPath.c_str());
    		    return false;
    		}

    		// Append slash when last character isn't a slash
    		createPath(destPath.c_str());

    		// Loop over directory contents
    		struct dirent* dirEntry;
    		while ((dirEntry = readdir(dirHandle)) != nullptr) {
    		    std::string entrySrcPath = srcPath + "/" + dirEntry->d_name;
    		    std::string entryDstPath = destPath + "/" + dirEntry->d_name;
    		    // Use lstat since readdir returns DT_REG for symlinks
    		    struct stat fileStat;
    		    if (lstat(entrySrcPath.c_str(), &fileStat) != 0) {
    		    	Utility::platformLog("Couldn't check what type this file/folder was:\n");
    		    	Utility::platformLog("%s\n", entrySrcPath.c_str());
    		    }

    		    if (S_ISLNK(fileStat.st_mode)) {
    		        continue;
    		    }
    		    else if (S_ISREG(fileStat.st_mode)) {
    		        // Copy file
    		        if (!copy_file(entrySrcPath, entryDstPath)) {
    		            closedir(dirHandle);
    		            return false;
    		        }
    		    }
    		    else if (S_ISDIR(fileStat.st_mode)) {
    		        // Ignore root and parent folder entries
    		        if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0) continue;

    		        // Copy all the files in this subdirectory
    		        if (!copy(entrySrcPath, entryDstPath)) {
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
}
