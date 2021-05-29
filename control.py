#!/usr/bin/env python3
from importlib.util import spec_from_file_location, module_from_spec
from os.path import dirname, abspath
from sys import path as importPath

CWD = dirname(abspath((lambda x: x).__code__.co_filename))
SELF_DIR_PASS = f"{CWD}/rt-thread-w60x"
importPath[0] = SELF_DIR_PASS

spec = spec_from_file_location("__main__", f"{SELF_DIR_PASS}/control.py")
mdl = module_from_spec(spec)
spec.loader.exec_module(mdl)
