
cp ./arch/arm/boot/zImage  /home/prinyiu/kitchen179/BOOT-EXTRACTED/
cp ./drivers/bluetooth/bthid/bthid.ko  		/home/prinyiu/kitchen179/BOOT-EXTRACTED/boot.img-ramdisk/lib/modules/
cp ./drivers/net/wireless/bcm4330/dhd.ko  	/home/prinyiu/kitchen179/BOOT-EXTRACTED/boot.img-ramdisk/lib/modules/
cp ./drivers/scsi/scsi_wait_scan.ko  		/home/prinyiu/kitchen179/BOOT-EXTRACTED/boot.img-ramdisk/lib/modules/
cp ./drivers/misc/fm_si4709/Si4709_driver.ko  	/home/prinyiu/kitchen179/BOOT-EXTRACTED/boot.img-ramdisk/lib/modules/

sync
