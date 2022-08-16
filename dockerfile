FROM wiiuenv/devkitppc:20220806

ENV PATH=$DEVKITPPC/bin:$PATH
ENV BUILD_TYPE=randomizer

WORKDIR /

# Install python for ASM patches
RUN apt-get update && apt-get install python3 -y

# Install wut
RUN git clone https://github.com/Maschell/wut wut -b fix_memoryleak --single-branch && \
    cd wut && \
    git checkout 23b286feeb9c5e42b9619466c43566fe7fea383b && \
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
