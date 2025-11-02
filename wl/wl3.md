第三章

```
 ~/osbook/devenv/run_qemu.sh Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
 ```

レジスタ情報を取得
```
 info registers
RAX=0000000000000000 RBX=0000000000000001 RCX=000000003fb7b1c0 RDX=0000000000000031
RSI=0000000000000400 RDI=000000003fea92a0 RBP=000000000000002e RSP=000000003fea8870
R8 =0000000000000000 R9 =000000003fecc30f R10=0000000000000000 R11=fffffffffffffffc
R12=000000003e67d73e R13=000000003fea8900 R14=000000003fea88b8 R15=000000003f21b920
RIP=000000003e67c411 RFL=00000202 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0030 0000000000000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0038 0000000000000000 ffffffff 00af9a00 DPL=0 CS64 [-R-]
SS =0030 0000000000000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
DS =0030 0000000000000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
FS =0030 0000000000000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
GS =0030 0000000000000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
LDT=0000 0000000000000000 0000ffff 00008200 DPL=0 LDT
TR =0000 0000000000000000 0000ffff 00008b00 DPL=0 TSS64-busy
GDT=     000000003fbee698 00000047
IDT=     000000003f306018 00000fff
CR0=80010033 CR2=0000000000000000 CR3=000000003fc01000 CR4=00000668
DR0=0000000000000000 DR1=0000000000000000 DR2=0000000000000000 DR3=0000000000000000 
DR6=00000000ffff0ff0 DR7=0000000000000400
EFER=0000000000000500
FCW=037f FSW=0000 [ST=0] FTW=00 MXCSR=00001f80
FPR0=0000000000000000 0000 FPR1=0000000000000000 0000
FPR2=0000000000000000 0000 FPR3=0000000000000000 0000
FPR4=0000000000000000 0000 FPR5=0000000000000000 0000
FPR6=0000000000000000 0000 FPR7=0000000000000000 0000
XMM00=0000000000000000 0000000000000000 XMM01=0000000000000000 0000000000000000
XMM02=0000000000000000 0000000000000000 XMM03=0000000000000000 0000000000000000
XMM04=0000000000000000 0000000000000000 XMM05=0000000000000000 0000000000000000
XMM06=0000000000000000 0000000000000000 XMM07=0000000000000000 0000000000000000
XMM08=0000000000000000 0000000000000000 XMM09=0000000000000000 0000000000000000
XMM10=0000000000000000 0000000000000000 XMM11=0000000000000000 0000000000000000
XMM12=0000000000000000 0000000000000000 XMM13=0000000000000000 0000000000000000
XMM14=0000000000000000 0000000000000000 XMM15=0000000000000000 0000000000000000
```

あんまり使われていない?
RIP が次に実行される命令を所持していることくらいしか、、

x コマンド

```
 x /4xb 0x067ae4c4
00000000067ae4c4: 0x00 0x00 0x00 0x00
```

現在 0x067ae4c4 を先頭としたアドレス 4 つ分に 0x00 が入っている

指定アドレスから 2 命令（アドレスではない) 取得する。
```
(qemu) x /2i 0x067ae4c4
0x00000000067ae4c4:  add    %al,(%rax)
0x00000000067ae4c6:  add    %al,(%rax)
```

データが 0x00 なのに特定の機械語に変換された。
うーん、これはどいうことだろう

次に rip に入っている 0x3e67c411 を見る

```
(qemu) x /4xb  0x3e67c411
000000003e67c411: 0xeb 0xfe 0x48 0x83
(qemu) x /2i  0x3e67c411
0x000000003e67c411:  jmp    0x3e67c411 
0x000000003e67c413:  sub    $0x28,%rsp
(qemu) 
```

jmp 0x3e67c411  で無限に自分にジャンプしているようだ。
だから一生 rip が変化しない。

## kernel をロードする

```
 git checkout osbook_day03a
```
 
 ```
clang++ -O2 -Wall -g \
    --target=x86_64-elf \
    -ffreestanding -mno-red-zone \
    -fno-exceptions -fno-rtti \
    -std=c++17 -c main.cpp

 ld.lld --entry KernelMain -z norelro --image-base 0x100000 --static \
 -o kernel.elf main.o
 ```

 0x100000 から順番にプログラムを配置できるようになっている。

```
source edksetup.sh
build

 ~/osbook/devenv/run_qemu.sh Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi ~/workspace/mikanos/kernel/kernel.elf
 ```

 起動した 0x1000000 が表示される。

 ```
 $ cat ~/workspace/mikanos/kernel/main.cpp 
extern "C" void KernelMain() {
  while (1) __asm__("hlt");
}
```
hlt は cpu をアイドル状態で停止する命令

kernel 関数は hlt を読んでいる。
extern "C" を使い c 言語で定義している。

```
RIP=000000003fb73016 
```

```
(qemu) x /2i 0x3fb73016
0x000000003fb73016:  cmpq   $0x0,0x40(%rsp)
0x000000003fb7301c:  je     0x3fb73016
```

hlt しないなー

```
$ readelf -h kernel.elf 
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
  Entry point address:               0x101120
  Start of program headers:          64 (bytes into file)
  Start of section headers:          1048 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         4
  Size of section headers:           64 (bytes)
  Number of section headers:         14
  Section header string table index: 12
dai@1X21C094-PC:~/workspace/mikanos/kernel$ 
```

エントリポイントが 指定したものと違う

ん－なんでこんな挙動になるのだろう
わからないなら写経してみるか日本人だし。

```
code ~/devDir/mikan/c/osbook_day03a.c
```

- コンパイル時に決定していないメモリ番地はどのように表されているのか
- プログラムで使用されるメモリや readelf で取得可能な entry のメモリはどのように決定するか
- qume のような 仮想マシンの場合のメモリ

efi ファイルで エントリポイントをしていしているが、それが  0x101120 でただしくよみとれていないかのうせいが高い

うーん、既知の問題

https://github.com/uchan-nos/os-from-zero/issues/134

> RIP=000000003fb73016

やっぱり、、、 ubuntu のばーじょんが新しいからこういう感じのリンカのバグはあるだろうなーっておもっていただけどこんなところで弊害がでるとは。

```
2$ clang -v
Ubuntu clang version 14.0.0-1ubuntu1.1
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/bin
Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/11
Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/11
Candidate multilib: .;@m64
Selected multilib: .;@m64
```

```
$ ld.lld -v
Ubuntu LLD 14.0.0 (compatible with GNU linkers)
```

だから ubuntu 18 や 20 でやれって書いてあったのか
こういうところ雑なんだよなあ

```
sudo apt-get update
sudo apt-get install lld-7
```

```
$ apt-cache search lld | grep lld-
firewalld-tests - installed tests for firewalld
liblld-11 - LLVM-based linker, library
liblld-11-dev - LLVM-based linker, header files
liblld-12 - LLVM-based linker, library
liblld-12-dev - LLVM-based linker, header files
liblld-13 - LLVM-based linker, library
liblld-13-dev - LLVM-based linker, header files
liblld-14 - LLVM-based linker, library
liblld-14-dev - LLVM-based linker, header files
lld-11 - LLVM-based linker
lld-12 - LLVM-based linker
lld-13 - LLVM-based linker
lld-14 - LLVM-based linker
liblld-15 - LLVM-based linker, library
liblld-15-dev - LLVM-based linker, header files
lld-15 - LLVM-based linker
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ 
```

アップストリームには存在しない様子

```
cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="lld" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/opt/llvm-7 \
  -DLLVM_ENABLE_WERROR=OFF \
  -DCMAKE_CXX_FLAGS="-Wno-error -Wno-array-bounds"
ninja
```

```
    |            ^~~
/home/dai/devDir/mikan/work/llvm-project/llvm/include/llvm/IR/ValueHandle.h:278:12: note: because ‘llvm::AssertingVH<const llvm::Value>’ has user-provided ‘ValueTy* llvm::AssertingVH<ValueTy>::operator=(const llvm::AssertingVH<ValueTy>&) [with ValueTy = const llvm::Value]’
  278 |   ValueTy *operator=(const AssertingVH<ValueTy> &RHS) {
      |            ^~~~~~~~
c++: fatal error: Killed signal terminated program cc1plus
compilation terminated.
[1070/2361] Building CXX object lib/Target/AMDGPU/CMakeFiles/LLVMAMDGPUCodeGen.dir/GCNHazardRecognizer.cpp.o
In file included from /home/dai/devDir/mikan/work/llvm-project/llvm/include/llvm/CodeGen/TargetSubtargetInfo.h:22,
                 from /home/dai/devDir/mikan/work/llvm-project/llvm/include/llvm/CodeGen/MachineRegisterInfo.h:32,
```

oom killer が働いた
うーん

```
$ free -h
               total        used        free      shared  buff/cache   available
Mem:           3.7Gi       336Mi       3.3Gi       0.0Ki       120Mi       3.2Gi
Swap:          1.0Gi       212Mi       811Mi
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
[sudo] password for dai: 
Setting up swapspace version 1, size = 2 GiB (2147479552 bytes)
no label, UUID=34209031-67d1-453e-8445-09f78121322c
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ 
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ 
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ free -h
               total        used        free      shared  buff/cache   available
Mem:           3.7Gi       341Mi       3.3Gi       0.0Ki       135Mi       3.2Gi
Swap:          3.0Gi       210Mi       2.8Gi
dai@1X21C094-PC:~/devDir/mikan/work/llvm-project/build$ 
```

おもっていたよりしんどいな

```
wsl -d Ubuntu-20.04
```
