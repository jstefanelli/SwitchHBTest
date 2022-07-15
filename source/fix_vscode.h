
#ifdef __VSCODE_FIX__
#undef __unix__

extern "C" {
    int setenv(const char* name, const char* value, int override) {
        return 0;
    }

    int unsetenv(const char* name) {
        return 0;
    }
}

#endif