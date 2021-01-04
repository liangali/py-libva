import os

logfile = '/tmp/tmp_pybind11.txt'
outfile = 'env.sh'

cmd = 'python3 -m pybind11 --includes >%s' % logfile
result = os.system(cmd)

out = []
with open(logfile, 'rt') as f:
    line = f.readline()
    s1, s2 = line.split()
    out.append('export PYTHON_INCLUDE=%s\n' % s1.split('-I')[1])
    out.append('export PYBIND11_INCLUDE=%s\n' % s2.split('-I')[1])

with open(outfile, 'wt') as f:
    f.writelines(out)

with open(outfile, 'rt') as f:
    content = f.readlines()
    for line in content:
        print(line)

os.system('chmod +x %s' % outfile)
os.system('rm %s' % logfile)

print('done')


