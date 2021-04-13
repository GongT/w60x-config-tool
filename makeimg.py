# -*- coding: utf-8 -*-

import sys
from serialport import main

print("========================", file=sys.stderr)
print("build complete.", file=sys.stderr)


argv = sys.argv[:]
argv.insert(0, 'makeimg')
main(argv)
