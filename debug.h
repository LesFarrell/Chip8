#ifndef DEBUG_HEADER_GUARD
#define DEBUG_HEADER_GUARD

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


clock_t Debug_StartTime;
clock_t Debug_StopTime;

void Debug_Print(int LOG_TYPE, char *str, ...);
void Debug_LogMessage(int LOG_TYPE, char* message);

#define LOG_NONE    1
#define LOG_ERROR   2
#define LOG_INFO    3
#define LOG_WARN    4


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: CHECK
 * Checks to see if the passed expression is true, if not calls Debug_Print().
 *
 * Parameters:
 * x - Expression to be checked.
 * m - string passed to Debug_Print() on failure.
 *
 * Returns:
 * void.
 */
#define CHECK(x, m)                                                                                               \
  if (!(x))                                                                                                       \
  {                                                                                                               \
    Debug_Print(LOG_ERROR ,"%s:%d: %s() - Debug Check '%s' failed %s\n", __FILE__, __LINE__, __func__, #x, m);    \
  }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: ASSERT
 * Checks to see if the passed expression is true, if not calls Debug_Print() and aborts the program.
 *
 * Parameters:
 * x - Expression to be checked.
 * m - string passed to Debug_Print() on failure.
 *
 * Returns:
 * calls abort() to terminate the program.
 */
#define ASSERT(x)                                                                                   \
  if (!(x))                                                                                         \
  {                                                                                                 \
    Debug_Print(LOG_INFO,"%s:%d: %s(): assertion '%s' failed\n", __FILE__, __LINE__, __func__, #x); \
    abort();                                                                                        \
  }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: STARTTIMER
 * Starts our simple debug timer.
 *
 * Parameters:
 *
 *
 * Returns:
 * void.
 */
#define STARTTIMER                          \
  if ((Debug_StartTime = clock()) == -1)    \
  {                                         \
    printf("clock returned error.");        \
    exit(1);                                \
  }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: STOPTIMER
 * Stops our debug timer.
 *
 * Parameters:
 *
 *
 * Returns:
 * void.
 */
#define STOPTIMER                           \
  if ((Debug_StopTime = clock()) == -1)     \
  {                                         \
    printf("clock returned error.");        \
    exit(1);                                \
  }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: SHOWTIME
 * Displays to number of seconds that have passed, to stderr.
 *
 * Parameters:
 *
 *
 * Returns:
 * void.
 */
#define SHOWTIME fprintf(stderr, "%6.3f seconds elapsed.\n", ((double) Debug_StopTime - Debug_StartTime) / CLOCKS_PER_SEC);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#endif