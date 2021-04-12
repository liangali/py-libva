import sys
import cv2
import numpy as np

sys.path.append('../build')
import pylibva as pyva

srcw, srch = 640, 480
dstw, dsth = 224, 224
input_file = '../../test.640x480.nv12'

pyva.init()
rtformtats = pyva.get_rtformat()

src_surf = pyva.create_surface(srcw, srcw, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(src_surf))
dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(dst_surf))

# in_data = np.fromfile(input_file, dtype=np.uint8, count=srcw*srch*3//2).reshape((srch*3//2, srcw))
in_data = np.arange(srcw*srch*3//2).reshape((srch*3//2, srcw))
pyva.write_surface(src_surf, in_data2)

vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)
out_data = pyva.read_surface(dst_surf)

pyva.destroy_surface(src_surf, 1)
pyva.destroy_surface(dst_surf, 1)
pyva.destroy_context(vpp_ctx)
pyva.close()

print('done')