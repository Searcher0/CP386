#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define MAX_SIZE 4096

// Structure to hold the Collatz sequence for multithreading
typedef struct
{
    int num;
    int sequence[1000];
    int length;
} collatz_data_t;

// Function to generate Collatz sequence
void collatz(int n, int *sequence, int *length)
{
    int i = 0;
    while (n != 1)
    {
        sequence[i++] = n;
        if (n % 2 == 0)
        {
            n /= 2;
        }
        else
        {
            n = 3 * n + 1;
        }
    }
    sequence[i++] = 1; // Sequence ends at 1
    *length = i;
}

// Thread function to generate and store Collatz sequence
void *generate_sequence(void *arg)
{
    collatz_data_t *data = (collatz_data_t *)arg;
    collatz(data->num, data->sequence, &(data->length));
    return NULL;
}

int main()
{
    const char *shm_name = "/collatz_shm"; // Shared memory name
    int shm_fd;                            // Shared memory file descriptor
    int *shm_ptr;                          // Pointer to shared memory
    int max_size = MAX_SIZE;               // Size of the shared memory

    // Open the file containing starting numbers
    FILE *file = fopen("start_numbers.txt", "r");
    if (!file)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Create shared memory object
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Shared memory open error");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory
    ftruncate(shm_fd, max_size);

    // Map shared memory to process address space
    shm_ptr = mmap(0, max_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    int num;
    pthread_t thread;
    collatz_data_t data;

    // Read each number from the file
    while (fscanf(file, "%d", &num) != EOF)
    {
        data.num = num;

        // Use pthreads to generate the Collatz sequence in parallel
        if (pthread_create(&thread, NULL, generate_sequence, &data) != 0)
        {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }

        // Wait for the thread to finish
        pthread_join(thread, NULL);

        pid_t pid = fork();
        if (pid == 0)
        {                                     // Child process
            int *collatz_sequence = shm_ptr;  // Attach to shared memory
            int sequence_length = shm_ptr[0]; // First element is the length

            // Print the Collatz sequence
            printf("Child Process: The Collatz sequence for %d is: ", num);
            for (int i = 1; i <= sequence_length; i++)
            {
                printf("%d ", collatz_sequence[i]);
            }
            printf("\n");

            // Detach and exit child process
            munmap(shm_ptr, max_size);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        { // Parent process
            // Store the sequence in shared memory
            shm_ptr[0] = data.length; // Store length of sequence
            for (int i = 0; i < data.length; i++)
            {
                shm_ptr[i + 1] = data.sequence[i]; // Store sequence
            }

            // Wait for the child process to finish
            wait(NULL);
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Clean up shared memory
    munmap(shm_ptr, max_size);
    shm_unlink(shm_name);
    fclose(file);

    return 0;
}
