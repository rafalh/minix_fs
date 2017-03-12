MINIX Filesystem Driver
=======================

General
-------
This is read/write Minix Filesystem driver for Windows.
Driver operates in user mode and uses a Dokan library to communicate with the kernel.
Driver was made for educational purposes in few days and gave me a lot of fun.

Requirements
------------

Dokan has to be installed before using this driver.
You can download it from: http://dokan-dev.net/en/download/
Filesystem has been tested with version 0.6. It may not work with new versions.

Usage
-----

Listing subpartitions in disk image:

	minix -f image_path -l

Mounting partition:

	minix -f image_path p0s0=mount_point_path

Mount point has to be unused partition letter or path to a non-existing folder.
You can specify more than one subpartion and mount point in command line.
To unmount all partitions terminate program by pressing Ctrl+C.

Notes
-----
Using this program can make damage to your disk image, so be sure to make a backup.
Blue Screens could happen too. I am not responsible for any loss of data.
