#ifndef MINILIBC_H_
#define MINILIBC_H_

#include "sdk.h"
#include "defines.h"

typedef unsigned int size_t;

#define NULL ((void *)0)
#define BUG_DELAY 1500 /* ms */

#define DEBUG(str) __debug__(str "%d", __LINE__, 0, 0)
#define DEBUG2(str, x) __debug__(str "%d:%X", __LINE__, (x), 0)
#define DEBUG3(str, x, y) __debug__(str "%d:%X_%X", __LINE__, (x), (y))
#define BUG(str) do { __debug__(str "%d", __LINE__, 0, 0); MicoSleepMilliSecs(BUG_DELAY); } while (0)
#define BUG2(str, x) do { __debug__(str "%d:%X", __LINE__, x, 0); MicoSleepMilliSecs(BUG_DELAY); } while (0)
#define BUG3(str, x, y) do { __debug__(str "%d:%X_%X", __LINE__, x, y); MicoSleepMilliSecs(BUG_DELAY); } while (0)
#define FLOAT(x, digits) ((int)x), ((int)((x) * digits - ((int)x) * digits))

#ifdef NO_PRINTF
  #define PRINTF(lv,args...) do { } while(0)
  static inline void m_printf(const char *format, ...) { format = format; }
  void m_FORCE_printf(const char *format, ...);
#else
  #define PRINTF(lv,args...) do { if (DEBUG >= lv) m_printf(args); } while (0)
  void m_printf(const char *format, ...);
#endif

void __debug__(const char *fmt, int line, int x0, int x1);
char *m_sprintf(char *dest, const char *format, ...); // return dest not length of the string
void *memcpy(void *restrict dst, const void *restrict src, int n);
void *memset(void *dst, int c, size_t n);

#endif
