import sys
sys.path.append('../build')
sys.path.append('py-libva/build')
import json
import pylibva

pylibva.init()

va_caps = {}
pl = pylibva.profiles()
for p in pl:
    va_caps[p] = {}
    el = pylibva.entrypoints(p)
    for e in el:
        al = pylibva.configs(p, e)
        va_caps[p][e] = json.loads(json.dumps(al))

with open('../../out.json', 'wt') as f:
    f.writelines(json.dumps(va_caps))

pylibva.close()
print('done')