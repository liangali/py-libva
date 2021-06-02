import sys
import time
import subprocess
import numpy as np
sys.path.append('../build')
import pylibva as pyva

def get_mem_usage(card_id = 0):
    meminfo = []
    cmd = "sudo cat /sys/kernel/debug/dri/%d/i915_gem_objects"%card_id
    stdout = subprocess.check_output(cmd, shell=True, text=True)
    for line in stdout.split('\n'):
        tag, t, a = '', 0, 0
        if 'total:' in line and 'available:' in line:
            tag = line.split(':')[0]
            for s in line.split():
                if 'total:' in s:
                    t = int(s.split('total:')[1].strip(','), 16)/(1024*1024)
                elif 'available:' in s:
                    a = int(s.split('available:')[1], 16)/(1024*1024)
            info = '%s: %5.4f / %5.4f MB'%(tag, a, t)
            meminfo.append(info)
    return meminfo

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

def test_bo_on_tile(tile_id):
    if tile_id < 0 or tile_id > 1:
        print('ERROR: invalid tile_id = %d'%tile_id)
        return
    print('='*32, 'bo will be created on tile_id = %d'%tile_id)

    w, h, loop = 1920, 1080, 10
    data_nv12 = np.random.randint(256, size=(h*3//2, w), dtype=np.uint8)
    
    pyva.init()
    attr = pyva.get_attr(19)
    if tile_id != parse_attr(attr['value'])['current_memory_region']:
        pyva.set_attr(19, tile_id)
    attr = pyva.get_attr(19)
    # print('pyva: get_attr', attr, 'new_region = ', parse_attr(attr['value'])['current_memory_region'], '\n')
    if tile_id != parse_attr(attr['value'])['current_memory_region']:
        print('ERROR: failed to set target tile_id', '!'*16)
    else:
        surflist = []
        print('%30s'%'Before surface creation ', get_mem_usage(1))
        for i in range(loop):
            surf = pyva.create_surface(w, h, "VA_RT_FORMAT_YUV420", 1)
            pyva.write_surface(surf, data_nv12)
            surflist.append(surf)
        print('%30s'%'After surface creation', get_mem_usage(1))
        for surf in surflist:
            tmp = pyva.read_surface(surf)
            pyva.destroy_surface(surf, 1)
        print('%30s'%'After surface destroy', get_mem_usage(1))
    pyva.close()


# test_query(10)
# test_set_attr(2)

tile_id = 0
if len(sys.argv) >= 2 and (int(sys.argv[1], 10) in [0, 1]):
    tile_id = int(sys.argv[1], 10)
test_bo_on_tile(tile_id)


print('%30s'%'application exit', get_mem_usage(1))
print('done')