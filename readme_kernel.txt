HOW TO BUILD KERNEL 2.6.36 FOR GT-I9103_SWA_DD

1. How to Build
	- get Toolchain
	Visit http://www.codesourcery.com/, download and install Sourcery G++ Lite 2009q3-68 toolchain for ARM EABI.
	Extract kernel source and move into the top directory.
	$ toolchain\arm-eabi-4.4.3
	$ cd Kernel/
	$ make tegra_n1_defconfig
	$ make
	
2. Output files
	- Kernel : Kernel/arch/arm/boot/zImage
	- module : Kernel/drivers/*/*.ko