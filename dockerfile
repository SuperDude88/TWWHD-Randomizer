FROM wiiuenv/devkitppc:20220728

ENV PATH=$DEVKITPPC/bin:$PATH

WORKDIR /

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 028899ecd3febe6cadbd3934c240d5f5e7a1d40d && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

COPY --from=wiiuenv/libiosuhax:2022052320420215a984 /artifacts $DEVKITPRO

# TODO: update to these libraries later (libiosuhax is old)
# # Install libmocha
# RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha -b devoptab --single-branch  && \
#     cd libmocha && \
#     git checkout cd0f3d12810fe2ca54a4136e6f29ecaf6b88ea1f && \
#     make -j$(nproc) && \
#     make install && \
#     cd .. && \
#     rm -rf libmocha
# 
# # Install libromfs
# RUN git clone --recursive https://gitlab.com/4TU/resinfs.git libromfs --single-branch && \
#     cd libromfs && \
#     git checkout 79cefa6610a682abac5b5996fbc38b5c67ae707e && \
#     make -j$(nproc) && \
#     sudo -E make install && \
#     cd .. && \
#     rm -rf libromfs

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
