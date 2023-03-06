#!/usr/bin/bash

FILE=doctest.h
if [ ! -f $FILE ]; then
    echo "$FILE does not exist. Downloading ..."
    curl https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -o doctest.h
fi

test_compilation="g++ --std=c++17 -g -Wall -Wextra -Wpedantic -Werror hash.h hash.cpp test.cpp -o test"
app_compilation="g++ --std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -DNDEBUG hash.h hash.cpp sha256.cpp -o sha256"
if [ ! -z $1 ]; then
    if [ $1 = test ]; then
        echo $test_compilation
        $test_compilation
    fi

    if [ $1 = app ]; then
        echo $app_compilation
        $app_compilation
    fi

    if [ $1 = all ]; then
        echo $test_compilation
        $test_compilation
        if [ ! $? -eq 0 ]; then
            exit
        fi
        echo $app_compilation
        $app_compilation
    fi
else
    echo $test_compilation
    $test_compilation
    if [ ! $? -eq 0 ]; then
        exit
    fi
    echo $app_compilation
    $app_compilation
fi
