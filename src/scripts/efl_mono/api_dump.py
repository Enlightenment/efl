#!/usr/bin/env python3

'''Dump names from generated xmldoc.'''

import sys
import argparse
import xml.etree.ElementTree as etree


def main(argv=None):
    if argv is None:
        argv = sys.argv

    parser = argparse.ArgumentParser(description='Dump C# api')
    parser.add_argument('docfile', metavar='DOCFILE', type=str,
                        nargs=1, help='Input xmldoc file.')
    parser.add_argument('outfile', metavar='OUTFILE', action='store',
                        default=None, nargs='?', help='Output file')

    args = parser.parse_args()

    tree = etree.parse(args.docfile[0])
    if args.outfile:
        outhandle = open(args.outfile, 'w')
    else:
        outhandle = sys.stdout

    for member in tree.findall('members/member'):
        outhandle.write(member.attrib['name'] + '\n')

if __name__ == "__main__":
    main()
