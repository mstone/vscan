import sys, tarfile
import cgitb
cgitb.enable(format="plain")

# Check that sys.argv is like
#  ['./test.py', 'C', '/usr', '1234', '5678', '91011']

assert len(sys.argv) == 6

assert sys.argv[1] == 'C'

assert sys.argv[2] == '/usr'

for i in sys.argv[3:]:
    assert int(i) > 0

# Check that examples.tar.gz exists and is a non-empty gzipped tarball

tf = tarfile.open("examples.tar.gz", "r:gz")
assert len(tf.getmembers()) > 0

exit(0)
