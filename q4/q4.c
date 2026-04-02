#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BELT 5
#define TOTAL_LUGGAGE 20

// Shared data
int belt[MAX_BELT];
int belt_size = 0;
int total_loaded = 0;
int total_dispatched = 0;

pthread_mutex_t belt_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t belt_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t belt_not_empty = PTHREAD_COND_INITIALIZER;

void *loader_thread(void *arg) {
  int luggage_id = 1;
  while (luggage_id <= TOTAL_LUGGAGE) {
    sleep(2); // takes 2s to load one luggage

    pthread_mutex_lock(&belt_mutex);

    while (belt_size == MAX_BELT) {
      // Belt full, wait
      pthread_cond_wait(&belt_not_full, &belt_mutex);
    }

    // Add luggage to belt
    belt[belt_size++] = luggage_id;
    total_loaded++;
    printf("Loader: Loaded luggage %d (Belt size: %d)\n", luggage_id,
           belt_size);

    luggage_id++;

    // Signal aircraft loader that belt is not empty
    pthread_cond_signal(&belt_not_empty);
    pthread_mutex_unlock(&belt_mutex);
  }

  return NULL;
}

void *aircraft_thread(void *arg) {
  while (1) {
    sleep(4);

    pthread_mutex_lock(&belt_mutex);

    while (belt_size == 0) {
      if (total_dispatched >= TOTAL_LUGGAGE) {
        pthread_mutex_unlock(&belt_mutex);
        return NULL; // done
      }
      pthread_cond_wait(&belt_not_empty, &belt_mutex);
    }

    int luggage_id = belt[--belt_size];
    total_dispatched++;
    printf("Aircraft: Dispatched luggage %d (Belt size: %d)\n", luggage_id,
           belt_size);

    // Signal loader that belt has space
    pthread_cond_signal(&belt_not_full);
    pthread_mutex_unlock(&belt_mutex);

    if (total_dispatched >= TOTAL_LUGGAGE)
      break;
  }
  return NULL;
}

// Monitor Thread
void *monitor_thread(void *arg) {
  while (1) {
    sleep(5);

    pthread_mutex_lock(&belt_mutex);
    printf("\n--- MONITOR ---\n");
    printf("Total loaded: %d\n", total_loaded);
    printf("Total dispatched: %d\n", total_dispatched);
    printf("Current belt size: %d\n", belt_size);
    printf("----------------\n\n");

    if (total_dispatched >= TOTAL_LUGGAGE) {
      pthread_mutex_unlock(&belt_mutex);
      break;
    }

    pthread_mutex_unlock(&belt_mutex);
  }
  return NULL;
}

int main() {
  pthread_t loader, aircraft, monitor;

  pthread_create(&loader, NULL, loader_thread, NULL);
  pthread_create(&aircraft, NULL, aircraft_thread, NULL);
  pthread_create(&monitor, NULL, monitor_thread, NULL);

  pthread_join(loader, NULL);
  pthread_join(aircraft, NULL);
  pthread_join(monitor, NULL);

  printf("\nAll luggage processed. Exiting program.\n");
  return 0;
}
