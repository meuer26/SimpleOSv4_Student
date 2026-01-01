# Copyright (c) 2023-2026 Dan O’Malley
# This file is licensed under the MIT License. See LICENSE for details.


default: build qemu

debug-stage2: build qemu-debug-stage2

debug-kernel: build qemu-debug-kernel

debug-shell: build qemu-debug-shell

debug-user: build qemu-debug-user

debug: build qemu-debug

build:
	zip SUBMIT-ME.zip *
	nasm -f bin bootloader-stage1.asm
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c bootloader-stage2.cpp -o bootloader-stage2.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c screen.cpp -o screen.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c fs.cpp -o fs.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c file.cpp -o file.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c vm.cpp -o vm.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c keyboard.cpp -o keyboard.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c trap.cpp -o trap.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c interrupts.cpp -o interrupts.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c syscalls.cpp -o syscalls.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c kernel.cpp -o kernel.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c shell2.cpp -o shell2.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c simpleOSlibc.cpp -o simpleOSlibc.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c frame-allocator.cpp -o frame-allocator.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c vmmonitor.cpp -o vmmonitor.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c exceptions.cpp -o exceptions.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c x86.cpp -o x86.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c sound.cpp -o sound.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c schedule.cpp -o schedule.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c top.cpp -o top.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c myprog.cpp -o myprog.o -Wunused-variable
	gcc -ggdb -m32 -fno-pie -ffreestanding -fno-stack-protector -c ed.cpp -o ed.o -Wunused-variable
	ld -m elf_i386 -e main -Ttext 0x9000 fs.o screen.o vm.o bootloader-stage2.o simpleOSlibc.o frame-allocator.o exceptions.o x86.o -o bootloader-stage2
	mkdir ./image-source
	ld -m elf_i386 -e main -Ttext 0x301000 syscalls.o interrupts.o trap.o keyboard.o fs.o screen.o vm.o simpleOSlibc.o frame-allocator.o vmmonitor.o exceptions.o file.o sound.o schedule.o x86.o kernel.o -o ./image-source/kernel
	ld -m elf_i386 -e main -Ttext 0x100000 screen.o fs.o vm.o keyboard.o simpleOSlibc.o frame-allocator.o exceptions.o x86.o shell2.o -o ./image-source/shell2
	ld -m elf_i386 -e main -Ttext 0x100000 screen.o x86.o vm.o simpleOSlibc.o frame-allocator.o exceptions.o top.o -o ./image-source/top
	ld -m elf_i386 -e main -Ttext 0x100000 screen.o x86.o vm.o simpleOSlibc.o frame-allocator.o exceptions.o myprog.o -o ./image-source/myprog
	ld -m elf_i386 -e main -Ttext 0x100000 screen.o fs.o vm.o keyboard.o simpleOSlibc.o frame-allocator.o exceptions.o x86.o ed.o -o ./image-source/ed
	dd if=/dev/zero of=tmp-ext2fs bs=1K count=1920
	echo "3E 03" | xxd -r -p > ./image-source/mpass
	cp genesis ./image-source/genesis
	cp sample ./image-source/sample
	mkfs.ext2 tmp-ext2fs -I 128 -b 1K -d ./image-source/
	dd if=/dev/zero of=fs.img bs=1M count=2
	cat bootloader-stage1 | dd of=fs.img bs=1 seek=0 conv=notrunc
	cat bootloader-stage2 | dd of=fs.img bs=1 seek=512 conv=notrunc
	cat tmp-ext2fs | dd of=fs.img bs=1 seek=131072 conv=notrunc
	find . -name '*.cpp' -o -name '*.h' -o -name '*.asm' | xargs wc -l

qemu:	
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

qemu-debug-stage2:
	gnome-terminal -- bash -c "gdb ./bootloader-stage2 -x gdb_script.txt"
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -gdb tcp::9000 -S -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

qemu-debug-kernel:
	gnome-terminal -- bash -c "gdb ./image-source/kernel -x gdb_script.txt"
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -gdb tcp::9000 -S -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

qemu-debug-shell:
	gnome-terminal -- bash -c "gdb ./image-source/shell2 -x gdb_script.txt"
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -gdb tcp::9000 -S -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

qemu-debug-user:
	gnome-terminal -- bash -c "gdb ./image-source/user -x gdb_script.txt"
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -gdb tcp::9000 -S -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

qemu-debug:
	gnome-terminal
	qemu-system-i386 -smp 2 -drive file=fs.img,format=raw -monitor stdio -gdb tcp::9000 -S -vga virtio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker

clean:
	rm -f fs.img
	rm -f bootloader-stage1
	rm -f bootloader-stage2
	rm -f bootloader-stage2.o
	rm -f md5.txt
	rm -f screen.o
	rm -f fs.o
	rm -f kernel.o
	rm -f vm.o
	rm -f keyboard.o
	rm -f interrupts.o
	rm -f trap.o
	rm -f syscalls.o
	rm -f kernel
	rm -f tmp-ext2fs
	rm -f shell2.o
	rm -f log.txt
	rm -f simpleOSlibc.o
	rm -f frame-allocator.o
	rm -f vmmonitor.o
	rm -f exceptions.o
	rm -f ed.o
	rm -f x86.o
	rm -f sound.o
	rm -f file.o	
	rm -f schedule.o
	rm -f top.o
	rm -f myprog.o
	rm -f ./image-source/*
	rm -f SUBMIT-ME.zip
	rmdir ./image-source