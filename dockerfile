FROM devkitpro/devkitppc:20251231

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer
ENV CMAKE_ARGS=

WORKDIR /

# Install python for ASM patches
COPY ./asm/requirements.txt /scripts/requirements.txt
RUN apt-get update && apt-get install python3 python3-pip python3-venv -y

# Set up a python virtualenv
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH=$VIRTUAL_ENV/bin:$PATH
RUN pip3 install -r /scripts/requirements.txt

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 26ed8f3b391f7df5ce1a6337a6e0524a0bafc764 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch && \
    cd libmocha && \
    git checkout 0317b390d2077895ed86d1d67f012a691e23652a && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf libmocha

# Install librpxloader
RUN git clone --recursive https://github.com/wiiu-env/librpxloader librpxloader --single-branch && \
    cd librpxloader && \
    git checkout e342001ff03f3e7194d1471a64a44396c01dc517 && \
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
