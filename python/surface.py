import sys
sys.path.append('../build')
import pylibva as pyva

import json
from json2html import *

def query_surface_format():
    outjson = []
    for rtf in pyva.get_rtformat():
        surf = pyva.create_surface(1920, 1080, rtf, 1)
        if surf == 0xffffffff:
            print('ERROR: failed to create %s, return = %x' % (rtf, surf))
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

def test_surface_creation():
    width = [i for i in range(8, 4096, 64)]
    height = [i for i in range(8, 4096, 64)]
    total_sum = 0
    for fmt in pyva.get_rtformat():
        total, passed = 0, 0;
        for w in width:
            for h in height:
                surf = pyva.create_surface(w, h, fmt, 1)
                if surf != 0xffffffff:
                    passed += 1
                pyva.destroy_surface(surf, 1)
                total += 1
        total_sum += total
        print('INFO: test create/destroy for %s, [%d/%d] passed' % (fmt, passed, total))
    print("%d tests finished" % total_sum)

pyva.init()
query_surface_format()
test_surface_creation()
pyva.close()

print('done')