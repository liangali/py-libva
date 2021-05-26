import sys
sys.path.append('../build')
import pylibva as pyva

pyva.init()

num = pyva.attribute_num()
print(num)

pyva.close()

print('done')