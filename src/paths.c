#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

#endif

// Add the relative part to the path.
void catRelPart(const char *rel, char *path) {
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

// Add the absolute part to the path.
int catAbsPart(char *path, size_t max_len) {
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

// Get the absolute path of the app and add the path value to it.
void *getPath(const char *path) {
    // Allocate memory for the path.
    char *abs_path = malloc(PATH_MAX);
    if(abs_path == NULL) {
        return NULL;
    }

    // If the path starts with a slash, assume its absolute already and just copy into the abs_path.
    if(path != NULL && path[0] == '/') {
        strcpy(abs_path, path);
        return abs_path;
    }

    // Get the absolute part of the path.
    size_t rel_len = path != NULL ? strlen(path) : 0;
    catAbsPart(abs_path, PATH_MAX - rel_len);

    // Get the relative part of the path.
    if(path != NULL) {
        catRelPart(path, abs_path);
    }

    return abs_path;
}

