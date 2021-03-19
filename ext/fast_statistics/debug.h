#ifndef DEBUG_H
#define DEBUG_H

#include <x86intrin.h>
#include <stdio.h>
#include <string.h>

#define CONCAT_(a, b) a##b
#define CONCAT(a,b) CONCAT_(a, b)
#define PROFILE DebugTimer CONCAT(Timer, __COUNTER__)(__func__, __LINE__);

struct DebugTimer {
  char* name;
  unsigned long long counter;
  
  DebugTimer(const char* function_name, int line_number) {
    name = (char*)malloc(200 * sizeof(char));

    strcpy(name, function_name);
    strcat(name, "_");
    snprintf(name + strlen(name), 4, "%d", line_number);

    counter = __rdtsc();
  }

  ~DebugTimer() {
    /* printf("\n%30s:\t %-10llu", name, __rdtsc() - counter); */
    free(name);
    fflush(stdout);
  }
};
#endif
