## make ファイル入門

```
$ make
clang++     -I/home/hradaiky/osbook/devenv/x86_64-elf/include/c++/v1 -I/home/hradaiky/osbook/devenv/x86_64-elf/include -I/home/hradaiky/osbook/devenv/x86_64-elf/include/freetype2     -I/home/hradaiky/edk2/MdePkg/Include -I/home/hradaiky/edk2/MdePkg/Include/X64     -nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS     -DEFIAPI='__attribute__((ms_abi))' -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 -c main.cpp
ld.lld -L/home/hradaiky/osbook/devenv/x86_64-elf/lib --entry KernelMain -z norelro --image-base 0x100000 --static -o kernel.elf main.o
```

Makefile の中身は以下のようになっている
```
TARGET = kernel.elf
OBJS = main.o

CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
            -fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += --entry KernelMain -z norelro --image-base 0x100000 --static


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
        rm -rf *.o

kernel.elf: $(OBJS) Makefile
        ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

%.o: %.cpp Makefile
        clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<
```

```
make all
```
ビルドする

中間ファイルの削除
```
make clean 
```

きょうせいてきにじっこう
```
make -B
```

## ブートローダの改造

### elf の構造

#### elf ヘッダー

entrypoint などが格納されている部分

```
l$ readelf -h kernel.elf 
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x101000
  Start of program headers:          64 (bytes into file)
  Start of section headers:          12008 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         4
  Size of section headers:           64 (bytes)
  Number of section headers:         16
  Section header string table index: 14
```

### プログラムヘッダ

```
l$ readelf -l kernel.elf 

Elf file type is EXEC (Executable file)
Entry point 0x101000
There are 4 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000100040 0x0000000000100040
                 0x00000000000000e0 0x00000000000000e0  R      0x8
  LOAD           0x0000000000000000 0x0000000000100000 0x0000000000100000
                 0x00000000000001b0 0x00000000000001b0  R      0x1000
  LOAD           0x0000000000001000 0x0000000000101000 0x0000000000101000
                 0x0000000000001000 0x0000000000001000  R E    0x1000
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x0

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .rodata 
   02     .text 
   03     
```

```
LOAD #1
FileSiz: 0x1b0
MemSiz : 0x1b0

LOAD #2
FileSiz: 0x1000
MemSiz : 0x1000
```

.bss を含んでいないと load の上記の値はいっしょになるっぽい　
elf ヘッダ系の話は hello hello world にも詳しくあったな


なるほどねー
第四章では主にピクセルの描画とブートローダーの改造の話がかかれているけど、ブートローダーの改造について UefiMain 関数だけでも写経しましょう