from building import *
import rtconfig

print('building w60x-config-tool as client')
cwd = GetCurrentDir()
CPPPATH = [cwd + '/public']

src = []
src += Glob('src/*.c')

depend = []

group = DefineGroup('w60x-config-client', src, depend, CPPPATH=CPPPATH)

Return('group')
