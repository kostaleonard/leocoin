#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif
#include "tests/file_paths.h"

return_code_t _get_executable_directory(char *dirname) {
    return_code_t return_code = SUCCESS;
#ifdef _WIN32
    DWORD length = GetModuleFileName(NULL, dirname, TESTS_MAX_PATH);
    if (0 == length || length == TESTS_MAX_PATH) {
        return_code = FAILURE_FILE_IO;
        goto end;
    } else {
        // Remove the executable name to get the directory.
        char *last_backslash = strrchr(dirname, '\\');
        if (NULL != last_backslash) {
            *last_backslash = '\0';
        }
    }
#else
    // On Unix-like systems, read the symbolic link /proc/self/exe
    ssize_t length = readlink("/proc/self/exe", dirname, TESTS_MAX_PATH);
    if (length <= 0 || length >= TESTS_MAX_PATH) {
        return_code = FAILURE_FILE_IO;
        goto end;
    } else {
        dirname[length] = '\0';
    }
    // Remove the executable name to get the directory.
    char *last_slash = strrchr(dirname, '/');
    if (NULL != last_slash) {
        *last_slash = '\0';
    }
#endif
end:
    return return_code;
}

return_code_t _get_tests_subdirectory(char *dirname, char *subdirname) {
    char executable_directory[TESTS_MAX_PATH] = {0};
    return_code_t return_code = _get_executable_directory(executable_directory);
    if (0 != return_code) {
        goto end;
    }
    int return_value = snprintf(
        dirname,
        TESTS_MAX_PATH,
        "%s/../tests/%s/",
        executable_directory,
        subdirname);
    if (return_value >= TESTS_MAX_PATH) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
    }
end:
    return return_code;
}

return_code_t get_fixture_directory(char *dirname) {
    return _get_tests_subdirectory(dirname, "fixtures");
}

return_code_t get_output_directory(char *dirname) {
    return _get_tests_subdirectory(dirname, "output");
}
