#include <stdint.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>

extern char **environ;

static const int32_t BUFFER_SIZE = 255;

size_t size_of_dynamic_array(char **array)
{
    int32_t size = 0;
    while (*array != NULL)
    {
        ++size;
        ++array;
    }

    return size;
}

char **parse_command(char *command)
{
    char **parsed_command = malloc(8 * sizeof(char *));
    if (!parsed_command)
    {
        printf("malloc call failed");
        return NULL;
    }

    const char *delimiter = " ";
    char *token = strtok(command, delimiter);
    int32_t index = 0;
    while (token)
    {
        parsed_command[index] = token;
        ++index;

        token = strtok(NULL, delimiter);
    }
    parsed_command[index] = NULL;

    return parsed_command;
}

int32_t execute_command(char **parsed_command)
{
    char *const *arguments = parsed_command;
    char *file_name = parsed_command[0];
    int32_t process_pid = 0;
    int32_t result = posix_spawnp(&process_pid, file_name, NULL, NULL, arguments, environ);

    if (result < 0)
    {
        printf("%s: command not found\n", file_name);
        return result;
    }

    int32_t status;
    waitpid(process_pid, &status, WUNTRACED);

    return result;
}

int32_t cd(const char *path)
{
    return chdir(path);
}

int32_t ll()
{
    char command[] = "ls -la";
    return execute_command(parse_command(command));
}

int main(int32_t argc, char *argv[])
{
    char input_buffer[BUFFER_SIZE + 1];
    while (true)
    {
        printf("$ ");

        char *res = fgets(input_buffer, BUFFER_SIZE, stdin);
        if (!res)
        {
            if (feof(stdin))
            {
                return EXIT_SUCCESS;
            }
            printf("Reading input failed\n");
            return EXIT_FAILURE;
        }
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        if (input_buffer[0] == '\0')
        {
            continue;
        }

        char **parsed_command = parse_command(input_buffer);
        if (!parsed_command)
        {
            printf("Failed to parse command\n");
            return EXIT_FAILURE;
        }

        if (strncmp("cd", parsed_command[0], BUFFER_SIZE) == 0)
        {
            if (size_of_dynamic_array(parsed_command) > 2)
            {
                printf("cd: too many arguments\n");
            }
            else
            {
                int32_t result = cd(parsed_command[1]);
                if (result < 0)
                {
                    printf("Failed to change directory\n");
                }
            }
            continue;
        }
        if (strncmp("ll", parsed_command[0], BUFFER_SIZE) == 0)
        {
            int32_t result = ll();
            continue;
        }

        int32_t result = execute_command(parsed_command);
        if (result < 0)
        {
            continue;
        }

        free(parsed_command);
    }
}