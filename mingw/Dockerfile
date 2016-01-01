FROM fedora:20
MAINTAINER opus@xiph.org

# Linux build.
RUN yum update -y
RUN yum install -y git gcc make wget
RUN yum install -y autoconf automake libtool

# mingw cross build.
RUN yum install -y mingw32-gcc zip

RUN yum clean all

RUN git clone https://git.xiph.org/opusfile.git

WORKDIR opusfile
RUN git pull
RUN make -C mingw
RUN ./configure --host=i686-w64-mingw32 --prefix=${PWD}/mingw PKG_CONFIG_PATH=${PWD}/mingw/lib/pkgconfig && make && make check && make install
