FROM wiiuenv/devkitppc:20220728

ENV PATH=$DEVKITPPC/bin:$PATH

WORKDIR /

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout a902da1ce7f45c35121f2a9a294c94e23b9d3fe7 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha -b devoptab --single-branch  && \
    cd libmocha && \
    git checkout 509527f110f53518d4280b5d70e359796f7c1b1f && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf libmocha

# # Install libromfs
RUN git clone --recursive https://github.com/SuperDude88/libromfs-wiiu libromfs --single-branch && \
    cd libromfs && \
    git checkout c9d4c12a45dc92a4ddca4e1507b4d07ac9e7ff03 && \
    make -j$(nproc) && \
    sudo -E make install && \
    cd .. && \
    rm -rf libromfs

# Hack to put the include/lib file into DKP/wut's native search directories
RUN mv /opt/devkitpro/wut/usr/include/* /opt/devkitpro/wut/include/

# Things for building
VOLUME /src
WORKDIR /src

CMD mkdir -p build && \
    cd build && \
    rm -rf * && \
    $DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake ../ && \
    make
