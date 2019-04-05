#include <stdlib.h>

// Declared in init.cpp

#ifdef _WIN32

int setenv(const char *envname, const char *envval, int overwrite) {
  return (int) _putenv_s(envname, envval);
}

int unsetenv(const char *name) {
  return (int) _putenv_s(name, "");
}

#endif
