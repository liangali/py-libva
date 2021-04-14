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

srcw, srch = 1920, 1080
dstw, dsth = 300, 300

# input_file = '../../test.%dx%d.nv12'%(srcw, srch)
# in_data = np.fromfile(input_file, dtype=np.uint8, count=srcw*srch*3//2).reshape((srch*3//2, srcw))

in_data_nv12 = np.random.randint(256, size=(srch*3//2, srcw), dtype=np.uint8)
cv2.putText(in_data_nv12, 'this is NV12', (50, srch//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255,255,255), 4)

in_data_rgbp = np.random.randint(256, size=(srch, srcw, 3), dtype=np.uint8)
cv2.putText(in_data_rgbp, 'this is RGBP', (50, srch//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255, 255, 255, 255), 10)
in_data_rgbp = in_data_rgbp.transpose((2, 0, 1))

pyva.init()

src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_RGBP", 1)
dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_RGBP", 1)
pyva.write_surface(src_surf, in_data_rgbp)

vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)
out_data = pyva.read_surface(dst_surf)

dump_image(in_data_rgbp, 'RGBP', '../../tmp.in.bmp')
dump_image(out_data, 'RGBP', '../../tmp.out.bmp')

pyva.destroy_surface(src_surf, 1)
pyva.destroy_surface(dst_surf, 1)
pyva.destroy_context(vpp_ctx)
pyva.close()

print('done')