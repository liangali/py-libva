import sys
sys.path.append('../build')
sys.path.append('py-libva/build')

import json
import pylibva
from json2html import *

pylibva.init()

va_caps = {}
pl = pylibva.profiles()
for p in pl:
    va_caps[p] = {}
    el = pylibva.entrypoints(p)
    for e in el:
        al = pylibva.configs(p, e)
        va_caps[p][e] = json.loads(json.dumps(al))

va_caps_json = json.dumps(va_caps)
with open('../../out.json', 'wt') as f:
    f.writelines(va_caps_json)

html = json2html.convert(json=va_caps_json)
with open('../../out.html', 'wt') as f:
    f.writelines(html)

pylibva.close()
print('done')