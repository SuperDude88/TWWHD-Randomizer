FROM devkitpro/devkitppc:20230501

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer

WORKDIR /

# Install python for ASM patches
RUN apt-get update && apt-get install python3 -y

# Install wut
RUN git clone https://github.com/devkitPro/wut wut --single-branch && \
    cd wut && \
    git checkout 1a85382b17e27e989f448d3e9d492f4d274a9179 && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf wut

# Install libmocha
RUN git clone --recursive https://github.com/wiiu-env/libmocha libmocha --single-branch  && \
    cd libmocha && \
    git checkout 455ecf6997899c7eb6732830bb24607de83736cf && \
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
