#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

#endif

void catRelPath(const char *rel, char *path) {
    #ifdef _WIN32
    path += strlen(path);
    while(*rel != '\0') {
        *path = *rel == '/' ? '\\' : *rel;
        path++; rel++;
    }
    *path = '\0';

    #else 
    strcat(path, rel);

    #endif
}

int getAbsPart(char *path, size_t max_len) {
    #ifdef __APPLE__
    CFBundleRef main_bundle;
    if(!(main_bundle = CFBundleGetMainBundle())) {
        return -1;
    }

    CFURLRef resourcesURL;
    if(!(resourcesURL = CFBundleCopyResourcesDirectoryURL(main_bundle))) {
        return -1;
    }

    Boolean res = CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8 *) path, max_len - 1);
    CFRelease(resourcesURL);
    if(!res) {
        return -1;
    }

    strcat(path, "/");

    #else 
    
    #error "Unsupported Operating system (paths.c)"

    #endif

    return 0;
}

void *getPath(const char *rel) {
    size_t rel_len = rel != NULL ? strlen(rel) : 0;
    char *path = malloc(PATH_MAX);
    
    if(path == NULL) {
        return NULL;
    }

    if(rel_len && rel[0] == '/') {
        strcpy(path, rel);
        return path;
    }

    getAbsPart(path, PATH_MAX - rel_len);
    if(rel != NULL) {
        catRelPath(rel, path);
    }

    return path;
}

