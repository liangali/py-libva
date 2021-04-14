import sys
import cv2
import numpy as np

sys.path.append('../build')
import pylibva as pyva

def dump_image(buf, fmt, filename='../../tmp.bmp'):
    if fmt == 'NV12':
        dst = cv2.cvtColor(buf, cv2.COLOR_YUV2BGR_NV12)
        cv2.imwrite(filename, dst)
    elif fmt == 'RGBP':
        dst = buf.transpose((1, 2, 0))
        cv2.imwrite(filename, dst)

def get_nv12(w, h):
    # input_file = '../../test.%dx%d.nv12'%(w, h)
    # in_data = np.fromfile(input_file, dtype=np.uint8, count=w*h*3//2).reshape((h*3//2, w))
    data_nv12 = np.random.randint(256, size=(h*3//2, w), dtype=np.uint8)
    cv2.putText(data_nv12, 'this is NV12', (50, h//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255, 255, 255, 255), 8)
    return data_nv12

def get_rgbp(w, h):
    data_rgbp = np.random.randint(256, size=(h, w, 3), dtype=np.uint8)
    cv2.putText(data_rgbp, 'this is RGBP', (50, h//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255, 255, 255, 255), 8)
    data_rgbp = data_rgbp.transpose((2, 0, 1))
    return data_rgbp

def nv12_to_nv12(srcw, srch, dstw, dsth):
    src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_YUV420", 1)
    dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_YUV420", 1)

    input_nv12 = get_nv12(srcw, srch)
    pyva.write_surface(src_surf, input_nv12)

    vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
    pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)
    output_nv12 = pyva.read_surface(dst_surf)

    dump_image(input_nv12,  'NV12', '../../tmp.input1.nv12.bmp')
    dump_image(output_nv12, 'NV12', '../../tmp.output.nv12.bmp')

    pyva.destroy_surface(src_surf, 1)
    pyva.destroy_surface(dst_surf, 1)
    pyva.destroy_context(vpp_ctx)

def nv12_to_rgbp(srcw, srch, dstw, dsth):
    src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_YUV420", 1)
    dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_RGBP", 1)

    input_nv12 = get_nv12(srcw, srch)
    pyva.write_surface(src_surf, input_nv12)

    vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
    pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)
    output_rgbp = pyva.read_surface(dst_surf)

    dump_image(input_nv12,  'NV12', '../../tmp.input2.nv12.bmp')
    dump_image(output_rgbp, 'RGBP', '../../tmp.output.rgbp.bmp')

    pyva.destroy_surface(src_surf, 1)
    pyva.destroy_surface(dst_surf, 1)
    pyva.destroy_context(vpp_ctx)

pyva.init()
nv12_to_nv12(1920, 1080, 300, 300)
nv12_to_rgbp(1920, 1080, 224, 224)
pyva.close()

print('done')