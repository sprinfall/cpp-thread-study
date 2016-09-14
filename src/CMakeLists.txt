if(UNIX)
    add_definitions(-D_GLIBCXX_USE_WCHAR_T -std=c++11)
endif()

add_executable(hello1 hello1.cpp)
add_executable(hello2 hello2.cpp)
add_executable(hello3 hello3.cpp)
add_executable(hello4 hello4.cpp)

add_executable(counter counter.cpp)

add_executable(mutex1 mutex1.cpp)
add_executable(mutex2 mutex2.cpp)
add_executable(mutex3 mutex3.cpp)
add_executable(mutex4 mutex4.cpp)

add_executable(cv1 cv1.cpp)
add_executable(cv2 cv2.cpp)

add_executable(bounded_buffer bounded_buffer.cpp)

add_executable(win32_semaphore win32_semaphore.cpp)
add_executable(semaphore semaphore.cpp)

add_executable(pool pool.cpp)

add_executable(shared_mutex shared_mutex.cpp)
add_executable(rw_lock rw_lock.cpp)
