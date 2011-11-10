#/usr/bin

###PATH=$PATH:/home/yiudou/ndk/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin

PATH=$PATH:/media/86GB/ginger235/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin

export ARCH=arm
export CROSS_COMPILE=arm-eabi-

make tegra_n1_defconfig

make -j3 


##cp ./arch/arm/boot/zImage  /home/yiudou/kitchen168/BOOT-EXTRACTED/
##cp ./drivers/scsi/scsi_wait_scan.ko /home/yiudou/kitchen168/WORKING_081310_012723/system/lib/modules/
##cp ./drivers/net/wireless/bcm4329/bcm4329.ko /home/yiudou/kitchen168/WORKING_081310_012723/system/lib/modules/



