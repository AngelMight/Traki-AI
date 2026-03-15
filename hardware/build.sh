


rm -rf build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 -DPICO_SDK_PATH=${HOME}/pico-sdk -DPICO_EXTRAS_PATH=${HOME}/pico-extras -DWIFI_SSID="Voltron" -DWIFI_PASSWORD=""

mingw32-make -j 16

sleep 2

cp build/EEE.uf2 /h/