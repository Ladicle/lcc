#!/bin/bash 

try() {
    expected="$1"
    input="$2"

    ./lcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" != "$expected" ]; then
	echo "Error: got=$actual, expected=$expected"
	exit 1
    fi
}

try 0 0
try 42 42
try 41 ' 12 + 34 - 5 '

echo OK
