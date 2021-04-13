# for module compiling
import os
Import('RTT_ROOT')
Import('BSP_ROOT')

cwd = str(Dir('#'))
objs = []
list = os.listdir(cwd)

for d in list:
    path = os.path.join(cwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        objs = objs + SConscript(os.path.join(d, 'SConscript'))

objs = objs + SConscript(os.path.join(BSP_ROOT, 'drivers/SConscript'))

Return('objs')
