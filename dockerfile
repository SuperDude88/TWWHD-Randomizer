FROM wiiuenv/devkitppc:20220917

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer

WORKDIR /

# Install python for ASM patches
RUN apt-get update && apt-get install python3 -y

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 7d9fa9e416bffbcd747f1a8e5701fd6342f9bc3d && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch  && \
    cd libmocha && \
    git checkout f3c45c52ad512b31d84f8254b7ad228aa4e0bab9 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf libmocha

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
