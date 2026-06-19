FROM devkitpro/devkitppc:20260503

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
    git checkout 6f9a38068d8feec5a315c99845053165eb5907e5 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch && \
    cd libmocha && \
    git checkout 50fefdf8307a875c63bdbdcf6c973779d4ddac92 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf libmocha

# Install librpxloader
RUN git clone --recursive https://github.com/wiiu-env/librpxloader librpxloader --single-branch && \
    cd librpxloader && \
    git checkout 7b055f15f2c0e31af5b0a4ceda44ecb1ec39cc34 && \
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
