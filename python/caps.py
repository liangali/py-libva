import sys
sys.path.append('../build')

import json
import pylibva as pyva
from json2html import *

pyva.init()

va_caps = {}
pl = pyva.profiles()
for p in pl:
    va_caps[p] = {}
    el = pyva.entrypoints(p)
    for e in el:
        al = pyva.configs(p, e)
        va_caps[p][e] = json.loads(json.dumps(al))

va_caps_json = json.dumps(va_caps)
with open('../../caps.json', 'wt') as f:
    f.writelines(va_caps_json)

html = json2html.convert(json=va_caps_json)
with open('../../caps.html', 'wt') as f:
    f.writelines(html)

pyva.close()
print('done')