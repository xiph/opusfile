# Continuous integration build script for opusfile.
# This script is run by automated frameworks to verify commits
# see https://mf4.xiph.org/jenkins/job/opusfile-autotools/

# This is intended to be run from the top-level source directory.

# WARNING: clobbers outside the current tree!
rm  -f ../opus
ln -s /srv/jenkins/jobs/opus/workspace ../opus

# compile
./autogen.sh
./configure PKG_CONFIG_PATH=$PWD/../opus
make clean
make

# verify distribution target
make distcheck

# build the documentation
make -C doc/latex
