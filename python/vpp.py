import sys
sys.path.append('../build')
import pylibva as pyva

srcw, srch = 640, 480
dstw, dsth = 224, 224

pyva.init()
rtformtats = pyva.get_rtformat()

src_surf = pyva.create_surface(srcw, srcw, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(src_surf))
dst_surf = pyva.create_surface(dstw, dsth, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(dst_surf))

vpp_ctx = pyva.create_context(dstw, dsth, dst_surf)
pyva.vpp_execute(vpp_ctx, src_surf, dst_surf)

pyva.destroy_surface(src_surf, 1)
pyva.destroy_surface(dst_surf, 1)
pyva.destroy_context(vpp_ctx)
pyva.close()

print('done')