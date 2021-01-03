project (optee_darknetz C)

set(THREADS_PREFER_PTHREAD_FLAG ON)

set( THREADS_PTHREAD_ARG 
     "0"
     CACHE STRING "Result from TRY_RUN" FORCE)

find_package(Threads REQUIRED)

FILE(GLOB SRC ca/src/*.c)
FILE(GLOB EXAM ca/examples/*.c)
set(MAIN ca/main.c)

add_executable (${PROJECT_NAME} ${SRC} ${EXAM} ${MAIN})

target_include_directories(${PROJECT_NAME}
				 PRIVATE ca/include
				 PRIVATE ca/src
				 PRIVATE include)

target_link_libraries (${PROJECT_NAME} PRIVATE teec -lm Threads::Threads)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
