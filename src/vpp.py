import sys
sys.path.append('../build')
sys.path.append('py-libva/build')
import pylibva

pylibva.init()


surf = pylibva.create_surface(640, 480, "NV12", 1)

print(pylibva.query_info(surf))

pylibva.destroy_surface(surf, 1)

pylibva.close()
print('done')