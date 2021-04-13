from os import getenv, getcwd, chdir, environ
from os.path import join, abspath
from sys import path as importPath
from pathlib import Path

# 基础文件路径设定
PROJECT_ROOT = abspath(getcwd())

print(" * PROJECT_ROOT = %s" % PROJECT_ROOT)

if getenv('RTT_ROOT'):
    RTT_ROOT = getenv('RTT_ROOT')
else:
    RTT_ROOT = join(getcwd(), 'rt-thread')

BSP_ROOT = join(RTT_ROOT + 'bsp/w60x')

importPath.append(join(RTT_ROOT, 'tools'))
importPath.append(BSP_ROOT)

environ["BSP_ROOT"] = BSP_ROOT
environ["RTT_ROOT"] = RTT_ROOT
environ["PKGS_DIR"] = join(PROJECT_ROOT, 'packages')
environ["ENV_ROOT"] = join(Path.home(), '.env')

print(" * RTT_ROOT = %s" % RTT_ROOT)

# 加载最主要的构建设置文件
try:
    from building import *
except:
    print('Cannot found RT-Thread root directory, please set environment variable "RTT_ROOT", or download it into rt-thread folder in current project.')
    print('找不到 RT-Thread 根目录，请设置环境变量“RTT_ROOT”，或下载到当前项目的rt-thread文件夹中')
    print('Download from: https://github.com/RT-Thread/rt-thread')
    exit(-1)

# 加载w60x特定的构建设置
try:
    chdir(BSP_ROOT)
    import rtconfig
    chdir(PROJECT_ROOT)
except:
    print('missing %s/rtconfig.py.' % BSP_ROOT)
    exit(-1)

# 导出
Export('BSP_ROOT')
Export('RTT_ROOT')
Export('rtconfig')

TARGET = 'Bin/w60x.app.' + rtconfig.TARGET_EXT

rtconfig.POST_ACTION = f"""
{rtconfig.SIZE} $TARGET
{rtconfig.OBJCPY} -O binary $TARGET Bin/w60x.app.bin
python serialport.py makeimg
"""


# 输出调试信息
print(" * COMPILER_PATH = %s" % rtconfig.EXEC_PATH)
rtconfig.LFLAGS = rtconfig.LFLAGS.replace(
    'drivers/linker_scripts', join(BSP_ROOT, 'drivers/linker_scripts'))

print(" * ASFLAGS = %s" % rtconfig.AFLAGS)
print(" * CCFLAGS = %s" % rtconfig.CFLAGS)
print(" * LINKFLAGS = %s" % rtconfig.LFLAGS)

# Kconfig补丁
with open(join(BSP_ROOT, 'Kconfig'), 'r+t') as kconf_file:
    content = kconf_file.read()
    import re
    if re.search('^mainmenu', content) != None:
        print("[WARN] patch file: %s" % kconf_file.name)
        content = re.sub('^mainmenu', '# mainmenu', content)
        kconf_file.seek(0)
        kconf_file.write(content)
        kconf_file.truncate()

# 导出构建过程
DefaultEnvironment(tools=[])
env = Environment(tools=['mingw'],
                  AS=rtconfig.AS, ASFLAGS=rtconfig.AFLAGS,
                  CC=rtconfig.CC, CCFLAGS=rtconfig.CFLAGS,
                  AR=rtconfig.AR, ARFLAGS='-rc',
                  LINK=rtconfig.LINK, LINKFLAGS=rtconfig.LFLAGS,
                  COMPILATIONDB_USE_ABSPATH=True)
env.PrependENVPath('PATH', rtconfig.EXEC_PATH)

if rtconfig.PLATFORM == 'iar':
    env.Replace(
        CCCOM=['$CC $CCFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -o $TARGET $SOURCES'])
    env.Replace(ARFLAGS=[''])
    env.Replace(LINKCOM=env["LINKCOM"] + ' --map project.map')

objs = PrepareBuilding(env, RTT_ROOT, has_libcpu=False)

# 生成compile_commands.json
env.Tool('compilation_db')
cdb = env.CompilationDatabase('.vscode/compile_commands.json')
Alias('cdb', cdb)

# 运行
DoBuilding(TARGET, objs)
