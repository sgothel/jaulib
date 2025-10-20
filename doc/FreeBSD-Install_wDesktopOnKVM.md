# FreeBSD Installation and Setup Summary
This is good for server and desktop on metal or as a virtual machine `KVM`.

## Used documentations
- [FreeBSD Handbook](https://docs.freebsd.org/en/books/handbook/)
  - [x11](https://docs.freebsd.org/en/books/handbook/x11/)
  - [desktop](https://docs.freebsd.org/en/books/handbook/desktop/#desktop-environments)
- [FreeBSD on QEMU and KVM](https://mertkont.com/2024/12/25/my-freebsd-installation-on-qemu-kvm/)

## Used FreeBSD Release 14.3

## Create KVM
All `KVM` specific settings are marked below.

- UEFI Q35
  - Disable secure-boot attempt in UEFI
- GPU: QXL
- USB Mouse
- Use CD-Cdrom SCSI

## Install
- Enable Ports
- Enable ntpd

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
pkg install vim rsync sudo bash curl 7-zip hwstat lscpu
pkg install git cmake libunwind doxygen autoconf
ln -s /usr/local/bin/bash /bin/bash

pkg install openjdk24 openjdk21 junit apache-ant
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Update
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    freebsd-update fetch
    freebsd-update install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Config File Summaries

### /etc/sysctl.conf:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # mouse evdev
    kern.evdev.rcpt_mask=6
    # kde perf
    net.local.stream.recvspace=65536
    net.local.stream.sendspace=65536
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
apply: sysctl -f /etc/sysctl.conf

### /etc/fstab
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # add noatime + noasync (default) to rootfs
    /dev/<rootfs>  /        <fs>     rw,noatime,noasync      1       1

    fdesc   /dev/fd         fdescfs         rw      0       0
    proc    /proc           procfs          rw      0       0
    # consider adding option size=<x>g, w/ <x> = 1/4 ram?
    tmpfs   /tmp	        tmpfs           rw      0       0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### /boot/loader.conf
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    snd_driver_load="YES"
    coretemp_load="YES"
    tmpfs_load="YES"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### /etc/rc.conf
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    powerd_flags="-n minimum -a minimum -b minimum -m 800 -M 2800"
    #powerd_flags="-n adaptive -a adaptive -b minimum -m 800 -M 2800"
    performance_cx_lowest="C3"
    sddm_enable="YES"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### /etc/make.conf
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WITHOUT_X11=yes
WITH_CCACHE_BUILD=yes
MAKE_JOBS_NUMBER=14
BATCH=yes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### /usr/local/etc/ccache.conf
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
max_size = 5.0G
cache_dir = /var/cache/ccache
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Via shell:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mkdir /var/cache/ccache
chmod ug+rwx /var/cache/ccache
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## User mods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pw groupmod wheel -m <your_username>
    pw groupmod video -m <your_username>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Base Config
- sysctl machdep.bootmethod
  Should show: UEFI
- /etd/ssh/sshd_config: PermitRootLogin yes
- pkg install sudo
  - pw groupmod wheel -m <your_username>
  - visudo
    ## Uncomment to allow members of group wheel to execute any command
    %wheel ALL=(ALL:ALL) ALL
- vim
  - config ~/.exrc ~/.vimrc
  - add to ~/.shrc ~/.bashrc
      alias vi='vim'
- Adjust the Video Resolution Values
  - vi /boot/loader.conf
    kern.vt.fb.default.mode="1920x1080"
    efi_max_resolution="1920x1080"
    vbe_max_resolution="1920x1080"
- Sound
  - kldload snd_driver
  - add to /boot/loader.conf
    snd_driver_load="YES"
    coretemp_load="YES"
  - test:
    - dmesg | grep pcm
    - cat /dev/sndstat
    - beep
- Power
  - kldload coretemp
  - add to /boot/loader.conf
    coretemp_load="YES"
  - test:
    sysctl -a | grep temperature
  - service powerd enable
  - service powerd start
  - sysctl dev.cpu.0
  -

## The X Window System:
- pkg install xorg
  To enable evdev in such a device, run the following:
  Add the following to /etc/sysctl.conf:
    kern.evdev.rcpt_mask=6
  - apply: sysctl -f /etc/sysctl.conf

- pw groupmod video -m <your_username>

## KDE
- pkg install kde
  - sysrc dbus_enable="YES"
  - Add the following lines to /etc/sysctl.conf sysctl.conf(5):
    net.local.stream.recvspace=65536
    net.local.stream.sendspace=65536
  - apply: sysctl -f /etc/sysctl.conf
- pkg install sddm
  - sysrc sddm_enable="YES"
  - add /usr/local/etc/sddm.conf
    User=jogamp

## Configure X Window System:
- user logged in, allow access to all:
  xhost +
### Framebuffer on UEFI
- pkg install xf86-video-scfb
- X -configure
- mv /root/xorg.conf.new /etc/X11/xorg.conf
- vi /etc/X11/xorg.conf
  Driver "modesetting" -> "scfb"

### Intel GPU
- pkg install xf86-video-intel
- X -configure
