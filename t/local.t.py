import sys
import cgitb
cgitb.enable(format="plain")

#  ['./test.py', '/path/to/logfile']
assert len(sys.argv) == 2

log = open(sys.argv[1]).read()
# "F t/local%2Et%2Ehtml fdc_str fairwaydrycleaners",
# "P t/local%2Et%2Ehtml fdc_str",
# "C t 1 22 1"
# ""

lines = list(sorted(a for a in log.split('\n') if a.strip()))

# "C t 1 22 1"
# "F t/local%2Et%2Ehtml fdc_str fairwaydrycleaners",
# "P t/local%2Et%2Ehtml fdc_str",

assert len(lines) == 3

lc, lf, lp = tuple(lines)

vc = lc.split(' ')
vf = lf.split(' ')
vp = lp.split(' ')

assert len(vc) == 5
assert len(vf) == 4
assert len(vp) == 3

assert vc[0] == 'C'
assert vc[1] == 't'
assert vc[4] == '1'

assert vf[0] == 'F'
assert vf[1] == 't/local%2Et%2Ehtml'
assert vf[2] == 'fdc_str'
assert vf[3] == 'fairwaydrycleaners'

assert vp[0] == 'P'
assert vp[1] == 't/local%2Et%2Ehtml'
assert vp[2] == 'fdc_str'

exit(0)
