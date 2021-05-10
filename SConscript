# for module compiling
import os
from building import DefineGroup

try:
    Import('PROJECT_ROOT')
except:
    PROJECT_ROOT = '//:/path/never/exists'

__file__ = os.path.abspath((lambda x: x).__code__.co_filename)
__dir__ = os.path.dirname(__file__)
if os.path.abspath(PROJECT_ROOT) == __dir__:
    # building this project
    Import('env')
    print('building w60x-config-tool as server')
    Import('IncludeChilds')

    env.AppendUnique(CPPPATH =[os.path.join(__dir__, 'library/shared')])
    objs = []
    objs += IncludeChilds(__dir__)
else:
    # building as a library (client)
    objs = SConscript(os.path.join(__dir__, 'library/include.py'),
                      variant_dir='w60x-config-tool',
                      duplicate=0)

Return('objs')
