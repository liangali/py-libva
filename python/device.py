import sys
import time
sys.path.append('../build')
import pylibva as pyva

def parse_attr(value):
    out = {}
    out['current_memory_region'] = value & 0xf
    out['local_memory_regions'] = (value>>4) & 0xf
    out['memory_regions_mask'] = (value>>16) & 0xffff
    return out

def test_query(loop):
    for i in range(loop):
        pyva.init()
        num = pyva.attr_num()
        attrlist = pyva.query_attr()
        #print(attrlist)
        for attr in attrlist:
            if attr['type'] == 19: # VADisplayAttribMemoryRegion
                print(parse_attr(attr['value']))
        time.sleep(0.01)
        pyva.close()

test_query(30)

print('done')