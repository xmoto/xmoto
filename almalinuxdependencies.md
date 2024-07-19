# Alma Linux Dependencies to Install the game using cmake
# just paste the below given dependencies in the terminal in the build folder of the xmoto like most probably /Downloads/xmoto-0.6.2/build 
# in the build folder open the terminal 
# paste the given below dependencies 
# then cmake .. to check the dependencies resolved or not 
# then cmake to compile 
# then make install 

sudo dnf install -y \
    cmake \
    curl \
    bzip2 \
    libjpeg-turbo \
    libpng \
    libxml2 \
    xz \
    lua \
    mesa-libGLU \
    SDL2 \
    SDL2_mixer \
    SDL2_net \
    SDL2_ttf \
    sqlite \
    libxdg-basedir \
    zlib \
    cmake \
    bzip2-devel \
    libcurl-devel \
    gettext-devel \
    libjpeg-turbo-devel \
    libxml2-devel \
    xz-devel \
    lua-devel \
    mesa-libGLU-devel \
    libpng-devel \
    SDL2-devel \
    SDL2_mixer-devel \
    SDL2_net-devel \
    SDL2_ttf-devel \
    sqlite-devel \
    libxdg-basedir-devel \
    zlib-devel
