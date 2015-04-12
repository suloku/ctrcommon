#include "ctrcommon/fs.hpp"

#include "service.hpp"

#include <sys/dirent.h>
#include <sys/unistd.h>
#include <stdio.h>

#include <algorithm>
#include <sstream>

#include <3ds.h>

static FS_archive sdmcArchive = {ARCH_SDMC, {PATH_EMPTY, 1, (u8*) ""}};

const std::string fsFixPath(const std::string path) {
    std::string fixed = path;
    std::string::size_type colon = fixed.find(':');
    if(colon != std::string::npos) {
        fixed = fixed.substr(colon + 1, std::string::npos);
    }

    if(fixed.find(':') != std::string::npos) {
        return NULL;
    }

    if(fixed[0] == '/') {
        return path;
    }

    static char cwdBuf[PATH_MAX + 1];
    getcwd(cwdBuf, PATH_MAX);

    std::stringstream stream;
    stream << cwdBuf << fixed;
    return stream.str();
}

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

bool fsHasExtension(const std::string path, const std::string extension) {
    std::string::size_type dotPos = path.rfind('.');
    return dotPos != std::string::npos && path.substr(dotPos + 1).compare(extension) == 0;
}

bool fsHasExtensions(const std::string path, const std::vector<std::string> extensions) {
    std::string::size_type dotPos = path.rfind('.');
    return extensions.empty() || (dotPos != std::string::npos && std::find(extensions.begin(), extensions.end(), path.substr(dotPos + 1)) != extensions.end());
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

bool fsDelete(const std::string path) {
    Result res = FSUSER_OpenArchive(NULL, &sdmcArchive);
    if(res == 0) {
        if(fsIsDirectory(path)) {
            res = FSUSER_DeleteDirectory(NULL, sdmcArchive, FS_makePath(PATH_CHAR, fsFixPath(path).c_str()));
        } else {
            res = FSUSER_DeleteFile(NULL, sdmcArchive, FS_makePath(PATH_CHAR, fsFixPath(path).c_str()));
        }

        FSUSER_CloseArchive(NULL, &sdmcArchive);
    }


    if(res != 0) {
        platformSetError(serviceParseError((u32) res));
    }

    return res == 0;
}
