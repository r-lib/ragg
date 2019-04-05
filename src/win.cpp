#include <stdlib.h>
#include <cstdio>

// Declared in init.cpp

#ifdef _WIN32

int setenv(const char *envname, const char *envval, int overwrite) {
  return (int) _putenv_s(envname, envval);
}

int unsetenv(const char *name) {
  char buf[1 + snprintf(NULL, 0, "%s=", name)];
  snprintf(buf, sizeof(buf), "%s=", name);
  return (int) _putenv(buf);
}

#endif
