FROM wiiuenv/devkitppc:20220806

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer

WORKDIR /

# Install python for ASM patches
RUN apt-get update && apt-get install python3 -y

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 0edd2e9c5c24d128714a6ef892c4b77bb5e20fce && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch  && \
    cd libmocha && \
    git checkout 75f31e0a3e2415a3619f732eab95e65f73d8d99b && \
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

CMD if [ "$BUILD_TYPE" = "randomizer" ]; then \
        mkdir -p build && \
        cd build && \
        rm -rf * && \
        $DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake ../ && \
        make; \
    else \
        if [ "$BUILD_TYPE" = "asm" ]; then \
            cd asm && \
            python3 ./assemble.py; \
        else \
            if [ "$BUILD_TYPE" = "full" ]; then \
                cd asm && \
                python3 ./assemble.py && \
                cd ../ && \
                \
                mkdir -p build && \
                cd build && \
                rm -rf * && \
                $DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake ../ && \
                make; \
            else \
                echo "Invalid build type"; \
            fi; \
        fi; \
    fi
