#include "helper.h"

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

char **tokenizeString(char *str, const char *delim)
{
    char **tokens = (char **)malloc(30 * sizeof(char *));
    char *token;
    int i = 0;

    token = strtok(str, delim);
    while (token != NULL)
    {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL; // Null-terminate the tokens array

    return tokens;
}
