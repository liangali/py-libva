import pylibva

a = pylibva.add(1, 2)
print(a)

pylibva.init()

pl = pylibva.profiles()
for p in pl:
    print(p)

print('done')