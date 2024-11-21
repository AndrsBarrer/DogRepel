#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "string.h"

/**
 * @brief Tokenizes a string based on a given set of delimiters.
 *
 * This function splits the input string `str` into an array of tokens
 * using the specified delimiters in `delim`. The resulting tokens are
 * stored in a dynamically allocated array of strings.
 *
 * @note The input string `str` is modified in-place because `strtok` replaces
 * delimiter characters with null characters (`\0`) to terminate tokens.
 * Ensure that `str` is modifiable.
 *
 * @param str The string to be tokenized. Must be modifiable and not `const`.
 * @param delim A null-terminated string specifying the delimiter characters.
 *
 * @return A dynamically allocated array of pointers to tokens. The array is
 * terminated with a `NULL` pointer. The caller is responsible for freeing this array.
 */
char **tokenizeString(char *str, const char *delim);

/**
 * @brief Delays the execution for a specified number of milliseconds.
 *
 * This function pauses the execution of the task for a given number of milliseconds.
 * It uses FreeRTOS's `vTaskDelay` function to achieve the delay, which is based on the
 * system's tick period.
 *
 * @param ms The number of milliseconds to delay the task.
 */
void delayMs(uint16_t ms);