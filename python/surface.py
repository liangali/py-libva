import sys
sys.path.append('../build')
import pylibva as pyva

import json
from json2html import *

pyva.init()

surf = pyva.create_surface(1280, 720, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(surf))

def query_surface_info():
    outjson = []
    for rtf in pyva.get_rtformat():
        surf = pyva.create_surface(1920, 1080, rtf, 1)
        #print('SURF: %s, %x' % (rtf, surf))
        if surf == 0xffffffff:
            continue
        info = pyva.query_info(surf)
        
        # convert for better readability
        a = '%x'%info['fourcc']             # valude to hex string
        b = bytearray.fromhex(a).decode()   # hex string to ascii
        info['fourcc'] = b[::-1]            # reverse string
        info['red_mask'] = '0x%x'%info['red_mask']
        info['green_mask'] = '0x%x'%info['green_mask']
        info['blue_mask'] = '0x%x'%info['blue_mask']
        info['alpha_mask'] = '0x%x'%info['alpha_mask']
        info['image_id'] = rtf # override image_id
        
        outjson.append(json.loads(json.dumps(info)))
        pyva.destroy_surface(surf, 1)
    
    html = json2html.convert(json=outjson)
    with open('../../surface.html', 'wt') as f:
        f.writelines(html)

query_surface_info()

pyva.close()
print('done')