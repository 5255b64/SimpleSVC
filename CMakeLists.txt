cmake_minimum_required(VERSION 3.17)
project(Code C)

set(CMAKE_C_STANDARD 11)
link_directories("/usr/lib")
link_libraries(asan)

#add_definitions("-O0 -std=gnu11 -lm -Wextra -Wall -Werror -g -fsanitize=address")
add_definitions("-O0 -std=gnu11 -lm -Wextra -Wall -Werror -g -fsanitize=address")


# add_definitions("-O0 -std=gnu11 -lm -Wextra -Wall -Werror -g")

add_executable(Code
        main.c
        svc.c
        test1.c
        test2.c
        testcases.h
        test_svc_rm1.c)


target_link_libraries(Code asan)