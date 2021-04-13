# -*- coding: utf-8 -*-

from os import name as PLATFORM_NAME
from os import getenv, getcwd, mkdir, kill, getpid, waitpid, remove, chmod
from signal import SIGINT
from os.path import join, abspath, isfile, isdir
from sys import path as importPath, stderr, executable as argv0
from subprocess import run, DEVNULL, STDOUT, PIPE
from time import sleep

# 1M
# "python" $WM_TOOLS/wm_gzip.py "./Bin/rtthread.bin"
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin" "./Bin/rtthread_1M.img" 0 0 "./Bin/version.txt" 90000 10100
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin.gz" "./Bin/rtthread_GZ_1M.img" 0 1 "./Bin/version.txt" 90000 10100 "./Bin/rtthread.bin"
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin" "./Bin/rtthread_SEC_1M.img" 0 0 "./Bin/version.txt" 90000 10100
# "$WM_TOOLS/makeimg_all" "./Bin/secboot.img" "./Bin/rtthread_1M.img" "./Bin/rtthread_1M.FLS"
# 2M
# "python" $WM_TOOLS/wm_gzip.py "./Bin/rtthread.bin"
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin" "./Bin/rtthread_2M.img" 3 0 "./Bin/version.txt" 100000 10100
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin.gz" "./Bin/rtthread_GZ_2M.img" 3 1 "./Bin/version.txt" 100000 10100 "./Bin/rtthread.bin"
# "$WM_TOOLS/makeimg" "./Bin/rtthread.bin" "./Bin/rtthread_SEC_2M.img" 3 0 "./Bin/version.txt" 100000 10100
# "$WM_TOOLS/makeimg_all" "./Bin/secboot.img" "./Bin/rtthread_2M.img" "./Bin/rtthread_2M.FLS"

PROJECT_ROOT = abspath(getcwd())
BIN_PATH = join(PROJECT_ROOT, 'Bin')
PID_FILE = join(BIN_PATH, '.flash.pid')
BIN_FILE = join(BIN_PATH, 'w60x.app.bin')
IMG_FILE = join(BIN_PATH, 'w60x.app.img')
FLS_FILE = join(BIN_PATH, 'w60x.app.fls')
VERSION_FILE = join(PROJECT_ROOT, 'version.txt')


if not isdir(BIN_PATH):
    mkdir(BIN_PATH)

try:
    with open(PID_FILE, 'rt') as f:
        pid = int(f.read())
    print(f'kill process {pid}')
    kill(pid, SIGINT)
    waitpid(pid, 0)
except:
    pass

with open(PID_FILE, 'wt') as f:
    f.write(str(getpid()))


def do_exit(code):
    remove(PID_FILE)
    exit(code)


def stream_process(process):
    go = process.poll() is None
    for line in process.stdout:
        print(line)
    return go


def exec(exe, args, encoding=None):
    print("\x1B[2m +", exe,  ' '.join(args), "\x1B[0m")
    p = run(executable=exe, args=[exe, *args],  stderr=stderr,
            stdout=stderr, shell=False, encoding=encoding)
    if p.returncode != 0:
        print("command failed.")
        do_exit(p.returncode)


def find_library():
    try:
        from json import load
        with open(join(PROJECT_ROOT, 'packages/pkgs.json'), 'rt') as f:
            data = load(f)
        for ele in data:
            if ele['name'] == 'WM_LIBRARIES':
                return join(PROJECT_ROOT, 'packages', 'wm_libraries-'+ele['ver'])
    except:
        import traceback
        traceback.print_exc()
    return None


def makeimg():
    pass


def main(argv):
    if len(argv) == 1:
        print("missing arguments")
        return

    action = argv[1]
    print(f"== {action} ==")

    wm_lib_path = find_library()
    if wm_lib_path == None:
        print("missing packages, you should run `pkgs --update`")
        do_exit(1)
    print(f"use path {wm_lib_path}...")
    wm_tools_path = join(wm_lib_path, 'Tools')
    importPath[0] = wm_tools_path

    def tools_path(*p):
        return join(wm_tools_path, *p)

    if action == 'makeimg':
        assert(len(argv) == 2)
        if not isfile(BIN_FILE):
            print(f'Program not compiled: {BIN_FILE}')
            do_exit(1)
        from makeimg import main as runMakeImg
        exec(argv0, [tools_path('wm_gzip.py'), BIN_FILE])
        runMakeImg(['', BIN_FILE+'.gz', IMG_FILE, '0', '1',
                    VERSION_FILE, '90000', '10100', BIN_FILE])
        chmod(tools_path('makeimg_all'), 0o777)
        exec(tools_path('makeimg_all'), [
             tools_path('secboot.img'), IMG_FILE, FLS_FILE])
        return

    try:
        if len(argv) == 3:
            portnumber = int(argv[2])
        else:
            portnumber = int(getenv('SERIAL_PORT_NUMBER', 'not-set'))
        if portnumber < 0:
            raise
    except:
        print("Missing serial port number\n  Set it in .vscode/settings.json\n  Or pass in last argument")
        do_exit(1)

    if action == 'flash':
        from download import main as runDownload
        port = f'COM{portnumber}' if PLATFORM_NAME == 'nt' else f'ttyUSB{portnumber}'
        runDownload(['', port, join(PROJECT_ROOT, IMG_FILE)])

    if action == 'term':
        import serial.tools.miniterm as serialTerminal
        port = f'COM{portnumber}' if PLATFORM_NAME == 'nt' else f'/dev/ttyUSB{portnumber}'
        try:
            from sys import argv
            argv.clear()
            argv.append(argv0)
            serialTerminal.main(default_port=port, default_baudrate=115200)
        except Exception as e:
            print(f'{type(e)}: {e}')
            do_exit(1)


if __name__ == '__main__':
    from sys import argv
    try:
        main(argv)
    except KeyboardInterrupt:
        print('\r^C')
        pass
