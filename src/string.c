#include "lib-header/string.h"
#include "lib-header/stdmem.h"

size_t strlen(char *str)
{
    int cnt = 0;
    while (str[cnt] != 0x0)
    {
        cnt++;
    }
    return cnt;
}

char *strtok(char *str, const char *delim)
{
    static char *next_token = NULL;

    if (str != NULL)
    {
        next_token = str;
    }
    else if (next_token == NULL)
    {
        return NULL;
    }

    char *current_token = next_token;
    char *p = next_token;
    while (*p != '\0')
    {
        const char *d = delim;
        while (*d != '\0')
        {
            if (*p == *d)
            {
                *p = '\0';
                next_token = p + 1;
                if (current_token == next_token)
                {
                    current_token++;
                    continue;
                }
                return current_token;
            }
            d++;
        }
        p++;
    }

    next_token = NULL;
    if (current_token == next_token)
    {
        return NULL;
    }
    return current_token;
}

int strcmp(char *str1, char *str2)
{
    unsigned int i = 0;

    if (strlen(str1) != strlen(str2))
    {
        return FALSE;
    }
    for (i = 0; i < strlen(str1); i++)
    {
        if (str1[i] != str2[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

void strcpy(char *dst, char *src) {
    // Empty Old String
    for (size_t i = 0; i < strlen(dst); i++) {
        dst[i] = '\0';
    }

    // Copy String in Src to Dst
    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    size_t i;
    for (i = 0; i < n && *src != '\0'; i++)
    {
        *dest++ = *src++;
    }
    for (; i < n; i++)
    {
        *dest++ = '\0';
    }
    return ret;
}

int strparse(char *str, char command[12][128], char *delim)
{
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            command[i][j] = '\0';
        }
    }
    char *token;
    int i = 0;

    // Parse the string
    token = strtok(str, delim);
    while (token != NULL && i < 128)
    {
        // Copy the string to the array
        strncpy(command[i], token, strlen(token));
        // Increment the count and move to the next token
        i++;
        token = strtok(NULL, " ");
    }

    return i;
};