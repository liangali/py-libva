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

def test_set_attr(loop):
    for i in range(loop):
        pyva.init()
        print('\n')
        attr = pyva.get_attr(19)
        print('pyva: get_attr', attr, 'old_region = ', parse_attr(attr['value'])['current_memory_region'])
        new_region = (1 - parse_attr(attr['value'])['current_memory_region'])
        print('pyva: set_attr', 'will set memory region to %d'%new_region)
        pyva.set_attr(19, new_region)
        attr = pyva.get_attr(19)
        print('pyva: get_attr', attr, 'new_region = ', parse_attr(attr['value'])['current_memory_region'], '\n')
        time.sleep(0.01)
        pyva.close()

# test_query(10)
get_set_attr(4)

print('done')