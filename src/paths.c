#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

void *get_path(char *rel) {
    size_t rel_len = strlen(rel);
    char *path = malloc(PATH_MAX);
    CFBundleRef main_bundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(main_bundle);
    CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8 *) path, PATH_MAX - rel_len - 1);
    CFRelease(resourcesURL);
    strcat(path, "/");
    strcat(path, rel);
    printf("%s\n", path);
    return path;
}

#else 

void *get_path(char *rel) {
    printf("NOT SUPPORTED\n");
    return NULL;
}

#endif 

