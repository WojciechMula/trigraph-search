#!/usr/bin/env python

import random
import sys

def main():
    try:
        path  = sys.argv[1]
        count = int(sys.argv[2])
    except IndexError, ValueError:
        sys.stderr.write("Usage: predictable_shuf.py path number")
        return

    with open(path, 'rt') as f:
        lines = f.readlines()

    random.seed(0) # make the sequence the same each time
    for line in random.sample(lines, count):
        sys.stdout.write(line)


if __name__ == '__main__':
    main()
