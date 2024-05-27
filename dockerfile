FROM devkitpro/devkitppc:20240504

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer
ENV CMAKE_ARGS=

WORKDIR /

# Install python for ASM patches
COPY ./asm/requirements.txt /scripts/requirements.txt
RUN apt-get update && apt-get install python3 python3-pip -y && pip3 install -r /scripts/requirements.txt

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 09e36bf299b4d90fda162c11b43648965e587ee2 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch  && \
    cd libmocha && \
    git checkout 4d0585cdb49964d54313e1fa2f10d37aedf77dac && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf libmocha

# Install librpxloader
RUN git clone --recursive https://github.com/wiiu-env/librpxloader librpxloader --single-branch  && \
    cd librpxloader && \
    git checkout 64787ea63d5d8a8e693e1a37bd837a93b3edd005 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf librpxloader

# Things for building
VOLUME /src
WORKDIR /src

CMD if [ "$BUILD_TYPE" = "randomizer" ]; then \
        mkdir -p build && \
        cd build && \
        rm -rf * && \
        $DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake ../ $CMAKE_ARGS && \
        make -j$(nproc); \
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
                $DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake ../ $CMAKE_ARGS && \
                make -j$(nproc); \
            else \
                echo "Invalid build type"; \
            fi; \
        fi; \
    fi
