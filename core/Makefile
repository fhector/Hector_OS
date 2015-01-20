
# デフォルト動作

default :
	../toolchain/make.exe img

# ファイル生成規則

ipl.bin : ipl.nas Makefile
	../toolchain/nask.exe ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	../toolchain/edimg.exe   imgin:../toolchain/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img

# コマンド

asm :
	../toolchain/make.exe -r ipl.bin

img :
	../toolchain/make.exe -r helloos.img

run :
	../toolchain/make.exe img
	copy helloos.img ..\toolchain\qemu\fdimage0.bin
	../toolchain/make.exe -C ../toolchain/qemu

install :
	../toolchain/make.exe img
	../toolchain/imgtol.com w a: helloos.img

clean :
	-del ipl.bin
	-del ipl.lst

src_only :
	../toolchain/make.exe clean
	-del helloos.img
