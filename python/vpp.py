import sys
import cv2
import numpy as np

sys.path.append('../build')
import pylibva as pyva

def dump_image(buf, filename='../../tmp.bmp'):
    w, h = buf.shape[1], buf.shape[0]*2//3
    dst = cv2.cvtColor(buf, cv2.COLOR_YUV2BGR_NV12)
    cv2.imwrite(filename, dst)

srcw, srch = 1920, 1080
dstw, dsth = 300, 300

# input_file = '../../test.%dx%d.nv12'%(srcw, srch)
# in_data = np.fromfile(input_file, dtype=np.uint8, count=srcw*srch*3//2).reshape((srch*3//2, srcw))
in_data = np.random.randint(256, size=(srch*3//2, srcw), dtype=np.uint8)
cv2.putText(in_data, 'this is a test', (50, srch//2), cv2.FONT_HERSHEY_DUPLEX, 8, (255,255,0), 4)

pyva.init()

src_surf = pyva.create_surface(srcw, srch, "VA_RT_FORMAT_YUV420", 1)
dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_YUV420", 1)
pyva.write_surface(src_surf, in_data)

vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)
out_data = pyva.read_surface(dst_surf)

dump_image(in_data, '../../tmp.in.bmp')
dump_image(out_data, '../../tmp.out.bmp')

pyva.destroy_surface(src_surf, 1)
pyva.destroy_surface(dst_surf, 1)
pyva.destroy_context(vpp_ctx)
pyva.close()

print('done')