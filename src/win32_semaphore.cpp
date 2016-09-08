#include <windows.h>
#include <cstdio>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686946(v=vs.85).aspx

// The following example uses a semaphore object to limit the number of threads
// that can perform a particular task. First, it uses the CreateSemaphore
// function to create the semaphore and to specify initial and maximum counts,
// then it uses the CreateThread function to create the threads.
//
// Before a thread attempts to perform the task, it uses the WaitForSingleObject
// function to determine whether the semaphore's current count permits it to do
// so. The wait function's time - out parameter is set to zero, so the function
// returns immediately if the semaphore is in the nonsignaled state.
// WaitForSingleObject decrements the semaphore's count by one.
//
// When a thread completes the task, it uses the ReleaseSemaphore function to
// increment the semaphore's count, thus enabling another waiting thread to
// perform the task.

#define SEMAPHORE_COUNT 12
#define THREAD_COUNT 12

HANDLE g_semaphore;

DWORD WINAPI Worker(LPVOID /*param*/) {
  DWORD wait_result;
  BOOL cont = TRUE;

  while (cont) {
    // Try to enter the semaphore gate.

    wait_result = WaitForSingleObject(g_semaphore, 0);  // INFINITE

    switch (wait_result) {
    case WAIT_OBJECT_0:
      // The semaphore object was signaled.
      SYSTEMTIME lt;
      GetLocalTime(&lt);

      printf("Thread %d: wait succeeded (%02d:%02d:%02d)\n", GetCurrentThreadId(), lt.wHour, lt.wMinute, lt.wSecond);

      cont = FALSE;

      // Simulate thread spending time on task
      Sleep(1000);

      // Release the semaphore when task is finished
      // increase count by one
      if (!ReleaseSemaphore(g_semaphore, 1, NULL)) {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
      }

      break;

    case WAIT_TIMEOUT:
      // The semaphore was not signaled, so a time-out occurred.
      //printf("Thread %d: wait timed out\n", GetCurrentThreadId());
      break;
    }
  }

  return TRUE;
}

int main() {

  // Create unnamed semaphore.

  g_semaphore = CreateSemaphore(NULL, 1, 1, NULL);

  if (g_semaphore == NULL) {
    printf("CreateSemaphore error: %d\n", GetLastError());
    return 1;
  }

  // Create worker threads.

  HANDLE threads[THREAD_COUNT];

  for (int i = 0; i < THREAD_COUNT; i++) {
    threads[i] = CreateThread(NULL, 0, Worker, NULL, 0, NULL);

    if (threads[i] == NULL) {
      printf("CreateThread error: %d\n", GetLastError());
      return 1;
    }
  }

  // Wait for all threads to terminate.

  WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

  for (int i = 0; i < THREAD_COUNT; i++) {
    CloseHandle(threads[i]);
  }

  CloseHandle(g_semaphore);

  return 0;
}
