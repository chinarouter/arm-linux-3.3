cmd_drivers/watchdog/built-in.o :=  /opt/gm8136/toolchain_gnueabi-4.4.0_ARMv5TE/usr/bin/arm-unknown-linux-uclibcgnueabi-ld -EL    -r -o drivers/watchdog/built-in.o drivers/watchdog/ftwdt010_wdt.o ; scripts/mod/modpost drivers/watchdog/built-in.o