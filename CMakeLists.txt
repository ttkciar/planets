cmake_minimum_required(VERSION 3.12)
project(planets C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(planets
        build.c
        cmds.c
        command.c
        conflict.c
        direct.c
        init.c
        lock.c
        log.c
        main.c
        make.c
        misc.c
        planets.h
        plot.c
        print.c
        range.c
        score.c
        telegram.c
        turn.c
        universe.c
        update.c)

find_package(Curses REQUIRED)

target_link_libraries(planets ${CURSES_LIBRARIES})