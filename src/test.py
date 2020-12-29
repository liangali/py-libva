import sys
sys.path.append('../build')
sys.path.append('py-libva/build')
import pylibva

pylibva.init()

pl = pylibva.profiles()
for p in pl:
    print(p)
    el = pylibva.entrypoints(p)
    for e in el:
        print('    %s' % e)

pylibva.close()
print('done')