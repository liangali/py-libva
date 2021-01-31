import sys
sys.path.append('../build')
import pylibva as pyva

import json
from json2html import *

pyva.init()

outjson = []
for rtf in pyva.get_rtformat():
    surf = pyva.create_surface(640, 480, rtf, 1)
    print('SURF: %s, %d' % (rtf, surf))
    if surf == 0xffffffff:
        continue
    info = pyva.query_info(surf)
    outjson.append(json.loads(json.dumps(info)))
    pyva.destroy_surface(surf, 1)

html = json2html.convert(json=outjson)
with open('../../surface.html', 'wt') as f:
    f.writelines(html)

pyva.close()
print('done')