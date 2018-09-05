#!/usr/bin/env python
from __future__ import print_function
import os
import sys
import argparse


def main(arguments):

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('infile1', help="Input file", type=argparse.FileType('r'))
    parser.add_argument('infile2', help="Input file", type=argparse.FileType('r'))
    parser.add_argument('-o', '--outfile', help="Output file",
                        default=sys.stdout, type=argparse.FileType('w'))

    args = parser.parse_args(arguments)

    infile1dict, infile2dict = {}, {}
    keys, items = [], []

    for line in args.infile1:
        if '=' in line: continue
        elif 'link' in line.lower():
            keys = line.split()
            for key in keys:
                infile1dict[key.lower()] = []
        else:
            items = line.split()
            for key, item in zip(keys,items):
                infile1dict[key.lower()].append(item)

    keys, items = [], []
    for line in args.infile2:
        if '=' in line: continue
        elif 'link' in line.lower():
            keys = line.split()
            print(keys)
            for key in keys:
                infile2dict[key.lower()] = []
        else:
            items = line.split()
            for key, item in zip(keys,items):
                infile2dict[key.lower()].append(item)

    if infile1dict==infile2dict: 
        print("success: outputs match")
    else:
        print("outputs do not match")

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
