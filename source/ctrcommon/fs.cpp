#include "ctrcommon/fs.hpp"

#include "service.hpp"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <sstream>

#include <3ds.h>

u64 fsGetFreeSpace(MediaType mediaType) {
    u32 clusterSize;
    u32 freeClusters;
    Result res = 0;
    if(mediaType == NAND) {
        res = FSUSER_GetNandArchiveResource(NULL, NULL, &clusterSize, NULL, &freeClusters);
    } else {
        res = FSUSER_GetSdmcArchiveResource(NULL, NULL, &clusterSize, NULL, &freeClusters);
    }

    if(res != 0) {
        platformSetError(serviceParseError((u32) res));
        return 0;
    }

    return (u64) clusterSize * (u64) freeClusters;
}

bool fsExists(const std::string path) {
    FILE* fd = fopen(path.c_str(), "r");
    if(fd) {
        fclose(fd);
        return true;
    }

    return fsIsDirectory(path);
}

bool fsIsDirectory(const std::string path) {
    DIR* dir = opendir(path.c_str());
    if(dir) {
        closedir(dir);
        return true;
    }

    return false;
}

std::string fsGetFileName(const std::string path) {
    std::string::size_type slashPos = path.rfind('/');
    if(slashPos == std::string::npos) {
        return path;
    }

    return path.substr(slashPos + 1);
}

std::string fsGetExtension(const std::string path) {
    std::string::size_type dotPos = path.rfind('.');
    if(dotPos == std::string::npos) {
        return "";
    }

    return path.substr(dotPos + 1);
}

bool fsHasExtension(const std::string path, const std::string extension) {
    if(extension.empty()) {
        return true;
    }

    const std::string ext = fsGetExtension(path);
    return strcasecmp(ext.c_str(), extension.c_str()) == 0;
}

bool fsHasExtensions(const std::string path, const std::vector<std::string> extensions) {
    if(extensions.empty()) {
        return true;
    }

    const std::string ext = fsGetExtension(path);
    for(std::vector<std::string>::const_iterator it = extensions.begin(); it != extensions.end(); it++) {
        std::string extension = *it;
        if(strcasecmp(ext.c_str(), extension.c_str()) == 0) {
            return true;
        }
    }

    return false;
}

std::vector<FileInfo> fsGetDirectoryContents(const std::string directory) {
    std::vector<FileInfo> result;
    bool hasSlash = directory.size() != 0 && directory[directory.size() - 1] == '/';
    const std::string dirWithSlash = hasSlash ? directory : directory + "/";

    DIR* dir = opendir(dirWithSlash.c_str());
    if(dir == NULL) {
        return result;
    }

    while(true) {
        struct dirent* ent = readdir(dir);
        if(ent == NULL) {
            break;
        }

        result.push_back({dirWithSlash + std::string(ent->d_name), std::string(ent->d_name)});
    }

    closedir(dir);
    return result;
}
