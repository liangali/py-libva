import sys
import cv2
import numpy as np

sys.path.append('../build')
import pylibva as pyva
class VA_FILTER:
    INTERPOLATION_DEFAULT           = 0x00000000
    INTERPOLATION_NEAREST_NEIGHBOR  = 0x00001000
    INTERPOLATION_BILINEAR          = 0x00002000
    INTERPOLATION_ADVANCED          = 0x00003000
    INTERPOLATION_MASK              = 0x0000f000
    SCALING_DEFAULT                 = 0x00000000
    SCALING_FAST                    = 0x00000100
    SCALING_HQ                      = 0x00000200
    SCALING_NL_ANAMORPHIC           = 0x00000300
    SCALING_MASK                    = 0x00000f00

def dump_image(buf, fmt, filename='../../tmp.bmp', dumptxt=False):
    if dumptxt:
        np.savetxt('%s.txt'%filename, buf, fmt='%03d', delimiter=', ')
    if fmt == 'NV12':
        dst = cv2.cvtColor(buf, cv2.COLOR_YUV2BGR_NV12)
        cv2.imwrite(filename, dst)
    elif fmt == 'RGBP':
        dst = buf.transpose((1, 2, 0))
        cv2.imwrite(filename, dst)

def get_nv12(w, h, fromfile=False):
    if fromfile:
        input_file = '../../test.%dx%d.nv12'%(w, h)
        data_nv12 = np.fromfile(input_file, dtype=np.uint8, count=w*h*3//2).reshape((h*3//2, w))
    else:
        data_nv12 = np.random.randint(256, size=(h*3//2, w), dtype=np.uint8)
        cv2.putText(data_nv12, 'this is NV12', (50, h//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255, 255, 255, 255), 8)
    return data_nv12

def get_rgbp(w, h):
    data_rgbp = np.random.randint(256, size=(h, w, 3), dtype=np.uint8)
    cv2.putText(data_rgbp, 'this is RGBP', (50, h//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255, 255, 255, 255), 8)
    data_rgbp = data_rgbp.transpose((2, 0, 1))
    return data_rgbp

def nv12_to_nv12(srcw, srch, dstw, dsth, flags):
    src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_YUV420", 1)
    dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_YUV420", 1)

    input_nv12 = get_nv12(srcw, srch, True)
    pyva.write_surface(src_surf, input_nv12)

    vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
    pyva.vpp_execute(vpp_ctx, src_surf, dst_surf, flags)
    output_nv12 = pyva.read_surface(dst_surf)

    dump_image(input_nv12,  'NV12', '../../tmp.input1.nv12.bmp')
    dump_image(output_nv12, 'NV12', '../../tmp.output.nv12.bmp')

    pyva.destroy_surface(src_surf, 1)
    pyva.destroy_surface(dst_surf, 1)
    pyva.destroy_context(vpp_ctx)

    return output_nv12

def nv12_to_rgbp(srcw, srch, dstw, dsth, flags):
    src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_YUV420", 1)
    dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_RGBP", 1)

    input_nv12 = get_nv12(srcw, srch)
    pyva.write_surface(src_surf, input_nv12)

    vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
    pyva.vpp_execute(vpp_ctx, src_surf, dst_surf, flags)
    output_rgbp = pyva.read_surface(dst_surf)

    dump_image(input_nv12,  'NV12', '../../tmp.input2.nv12.bmp')
    dump_image(output_rgbp, 'RGBP', '../../tmp.output.rgbp.bmp')

    pyva.destroy_surface(src_surf, 1)
    pyva.destroy_surface(dst_surf, 1)
    pyva.destroy_context(vpp_ctx)

    return output_rgbp

pyva.init()

out1_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_DEFAULT)
out2_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_NEAREST_NEIGHBOR)
out3_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_BILINEAR)
out4_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_ADVANCED)
dump_image(np.absolute(out2_nv12 - out1_nv12),  'NV12', '../../tmp.output.2-1.bmp', True)
dump_image(np.absolute(out3_nv12 - out1_nv12),  'NV12', '../../tmp.output.3-1.bmp', True)
dump_image(np.absolute(out4_nv12 - out1_nv12),  'NV12', '../../tmp.output.4-1.bmp', True)

# Bilinear - Fast vs. HQ ==> same
out5_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_BILINEAR | VA_FILTER.SCALING_FAST)
out6_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_BILINEAR | VA_FILTER.SCALING_HQ)
dump_image(np.absolute(out6_nv12 - out5_nv12),  'NV12', '../../tmp.output.6-5.bmp', True)

# Advanced - Fast vs. HQ ==> same
out7_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_ADVANCED | VA_FILTER.SCALING_FAST)
out8_nv12 = nv12_to_nv12(1920, 1080, 300, 300, VA_FILTER.INTERPOLATION_ADVANCED | VA_FILTER.SCALING_HQ)
dump_image(np.absolute(out8_nv12 - out7_nv12),  'NV12', '../../tmp.output.8-7.bmp', True)

# Fast - Bilinear vs. Advanced ==> diff
dump_image(np.absolute(out7_nv12 - out5_nv12),  'NV12', '../../tmp.output.7-5.bmp', True)

# HQ - Bilinear vs. Advanced ==> diff
dump_image(np.absolute(out8_nv12 - out6_nv12),  'NV12', '../../tmp.output.8-6.bmp', True)

nv12_to_rgbp(1920, 1080, 224, 224, VA_FILTER.INTERPOLATION_BILINEAR)
pyva.close()

print('done')