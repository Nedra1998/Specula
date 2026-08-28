#include "specula/util/memory.hpp"

#include "specula/util/log.hpp"

#ifdef SPECULA_HAVE_MALLOC_H
#  include <malloc.h>
#endif

#ifdef SPECULA_IS_WINDOWS
#  include <psapi.h>
#  include <windows.h>
#  pragma comment(lib "psapi.lib")
#elif defined(SPECULA_IS_OSX)
#  include <mach/mach.h>
#elif defined(SPECULA_IS_LINUX)
#  include <cstdio>

#  include <unistd.h>
#endif

size_t specula::get_current_rss() {
#ifdef SPECULA_IS_WINDOWS
  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  return (size_t)info.WorkingSetSize;

#elif defined(SPECULA_IS_OSX)
  struct mach_task_basic_info info;
  mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) !=
      KERN_SUCCESS)
    return (size_t)0L; /* Can't access? */
  return (size_t)info.resident_size;

#elif defined(SPECULA_IS_LINUX)
  FILE *fp = fopen("/proc/self/statm", "r");
  if (fp == nullptr) {
    LOG_ERROR("Unable to open /proc/self/statm");
    return 0;
  }

  long rss = 0L;
  if (fscanf(fp, "%*s%ld", &rss) != 1) {
    LOG_ERROR("Unable to read /proc/self/statm");
    fclose(fp);
    return 0;
  }
  fclose(fp);
  return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);

#else
#  error "get_current_rss() is not defined for this target"
  return 0;
#endif
}
