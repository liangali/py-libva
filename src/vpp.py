import sys
sys.path.append('../build')
sys.path.append('py-libva/build')
import pylibva as pyva

pyva.init()

rtformtats = pyva.get_rtformat()

surf = pyva.create_surface(640, 480, "VA_RT_FORMAT_YUV420", 1)
print(pyva.query_info(surf))
pyva.destroy_surface(surf, 1)

pyva.close()
print('done')