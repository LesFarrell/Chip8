/*---------------------------------------------------------------------------------------
 * About:    Debug.c
 * Author:   Les Farrell
 *
 * Purpose:
 *           Routines and functions used to log messages and errors.
 *
 * Notes:
 *
 *---------------------------------------------------------------------------------------*/
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "debug.h"

#ifndef timegm
#define timegm _mkgmtime
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: RemoveCRLF
 * Remove carriage returns and line feeds from the passed string.
 *
 * Parameters:
 * s - Pointer to the string.
 *
 * Returns:
 * s - is updated with the updated string.
 *
 */
void RemoveCRLF(char *s)
{
    while (*s && *s != '\n' && *s != '\r')
    {
        s++;
    }
    *s = 0;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: Debug_GetISOTimeDate
 * Returns the UTC time as an ISO date time string.
 *
 * Parameters:
 *  None.
 *
 * Returns:
 *	char * - Returns an ISO Datetime.  yyyy-mm-dd hh:mm:ss
 *
 */
void Debug_GetISOTimeDate(char *DateTime)
{
    time_t rawtime;
    struct tm *info;
    time(&rawtime);

    info = gmtime(&rawtime);
    strftime(DateTime, 64, "%Y-%m-%d %X", info);
    return;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 * Function: Debug_LogMessage.
 * Log a message to a file.
 *
 * Parameters:
 * 	message - The string to write to file.
 * 	LOG_TYPE - The type of log file to create.
 *
 *           - LOG_NONE (Displays only, doesn't write to log file.)
 *           - LOG_ERROR (Display and log as a Error message.)
 *           - LOG_INFO (Display and log as a Information message.)
 *           - LOG_WARN (Display and log as a Warning message.)
 *
 * Returns:
 *  void.
 */
void Debug_LogMessage(int LOG_TYPE, char *message)
{
    char SpecialFolder[FILENAME_MAX] = {'\0'};
    char FullPath[FILENAME_MAX] = {'\0'};
    char FullMessage[2048] = {'\0'};
    char DateTime[64] = {'\0'};
    char buffer[1024] = {'\0'};
    unsigned long FileSize = 0;
    LPVOID lpMsgBuf;
    FILE *fp;

    // Get the Users Personal folder.
    SHGetFolderPathA(0, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, SpecialFolder);
    strcpy(FullPath, SpecialFolder);
    strcat(FullPath, "\\");
    strcat(FullPath, "LOG_FILES");

    // Try and create the logging folder.
    CreateDirectoryA(FullPath, NULL);
    SetLastError(0);
    strcat(FullPath, "\\");

    // Start to build up the message string.
    Debug_GetISOTimeDate(&DateTime[0]);
    strcat(FullMessage, DateTime);
    strcat(FullMessage, " : ");

    switch (LOG_TYPE)
    {
    case LOG_NONE:
        return;
        break;

    case LOG_ERROR:
        strcat(FullPath, "ERROR.LOG");
        strcat(FullMessage, "[ERROR] ");

        /*
            if (GetLastError() != 0)
			{
				// Add the text of the LastError API.
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
				strcat(FullMessage, "API Last Error : ");
				strcpy(buffer, (LPCTSTR)lpMsgBuf);
				strcat(FullMessage, buffer);
				strcat(FullMessage, "\n");
			}
            */
        break;

    case LOG_INFO:
        strcat(FullPath, "INFOMATION.LOG");
        strcat(FullMessage, "[INFO] ");
        break;

    case LOG_WARN:
        strcat(FullPath, "WARNINGS.LOG");
        strcat(FullMessage, "[WARN] ");
        break;

    default:
        break;
    }

    // Add the message text.
    strcat(FullMessage, message);
    strcat(FullMessage, "\n");

    // Check the File Size.
    fp = fopen(FullPath, "r");
    fseek(fp, 0, SEEK_END); // seek to end of file.
    FileSize = ftell(fp);   // get current file pointer.
    fclose(fp);

    // Delete the log file, if it gets too large.
    if (FileSize > ((1024 * 1024) * 100))
    {
        remove(FullPath);
    }

    // Open the file for appending.
    fp = fopen(FullPath, "a");
    fputs(FullMessage, fp);
    fclose(fp);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
* Function: Debug_Print
* A General-Purpose, Debug Output Function.
* Similar to printf (Same syntax).
*
* Parameters:
*  LOG_TYPE  : The type of logging we are doing if any?
*
*           - LOG_NONE (Displays only, doesn't write to log file.)
*           - LOG_ERROR (Display and log as a Error message.)
*           - LOG_INFO (Display and log as a Information message.)
*           - LOG_WARN (Display and log as a Warning message.)
*
*
*  Formating : Almost the same parameters as printf. (Only Almost, so double check!)
*
* Returns:
*  void.
*/
void Debug_Print(int LOG_TYPE, char *str, ...)
{
    char Result[1024] = {'\0'}; // Holds the formatted text.
    char DateTime[128] = {'\0'};
    char Temp[2048] = {'\0'};

    Debug_GetISOTimeDate(DateTime);

    // Get arg pointer.
    va_list ptr;

    // Point ptr to the first arg after str.
    va_start(ptr, str);

    // Pass args to wvsprintf().
    wvsprintfA(Result, str, ptr);

    // Remove any extra carriage returns or Line Feeds from the debug string.
    RemoveCRLF(Result);

    OutputDebugStringA(Result);
    Debug_LogMessage(LOG_TYPE, Result);

    sprintf(Temp, "%s - %s\n", DateTime, Result);
    fprintf(stderr, "%s", Temp);

    return;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
