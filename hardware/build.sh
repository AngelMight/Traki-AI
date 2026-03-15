


rm -rf build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 -DPICO_SDK_PATH=${HOME}/pico-sdk -DPICO_EXTRAS_PATH=${HOME}/pico-extras -DWIFI_SSID="Voltron" -DWIFI_PASSWORD=""

mingw32-make -j 16


echo "Waitng for build/EEE.uf2 to appear"
elapsed=0
timeout=10
while [ ! -f build/EEE.uf2 ] && [ $elapsed -lt $timeout ]; do
    sleep 1
    elapsed=$((elapsed + 1))
done


if [ ! -f build/EEE.uf2 ]; then
    echo "ERROR"
else
	 cp build/EEE.uf2 /h
    echo "DONE"
fi


