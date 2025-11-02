第二章に入ります。

edk をダウンロード

https://github.com/uchan-nos/mikanos-build#

こいつに従います

```
ln -s ~/workspace/mikanos/MikanLoaderPkg/ ./

lrwxrwxrwx  1 dai dai    43 Oct 18 17:20 MikanLoaderPkg -> /home/dai/workspace/mikanos/MikanLoaderPkg/
```

edk2 に上記のようなsym link 作成

~/workspace/mikanos/MikanLoaderPkg/

にクローンする

```
 git checkout osbook_day02a 
```

edk 配下にシムリンクを張ってそのなかのプロジェクトをタグで管理している。

```
$ ls MikanLoaderPkg/
Loader.inf  Main.c  MikanLoaderPkg.dec  MikanLoaderPkg.dsc
dai@1X21C094-PC:~/workspace/mikanos$ 
dai@1X21C094-PC:~/workspace/mikanos$ 
dai@1X21C094-PC:~/workspace/mikanos$ cat MikanLoaderPkg/Main.c 
#include  <Uefi.h>
#include  <Library/UefiLib.h>

EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table) {
  Print(L"Hello, Mikan World!\n");
  while (1);
  return EFI_SUCCESS;
}
dai@1X21C094-PC:~/workspace/mikanos$ 
```

day02 はこんな感じだ。

```
:~/edk2$ ls -al MikanLoaderPkg/
total 24
drwxr-xr-x 2 dai dai 4096 Oct 18 17:29 .
drwxr-xr-x 4 dai dai 4096 Oct 18 17:29 ..
-rw-r--r-- 1 dai dai  498 Oct 18 17:29 Loader.inf
-rw-r--r-- 1 dai dai  217 Oct 18 17:29 Main.c
-rw-r--r-- 1 dai dai  217 Oct 18 17:26 MikanLoaderPkg.dec
-rw-r--r-- 1 dai dai 1377 Oct 18 17:29 MikanLoaderPkg.dsc
```

edk 内でsymlink で表示された

Loader.inf
```
  ENTRY_POINT                    = UefiMain
```

このかんすうがエントリポイントになっている

```

build.py...
/home/dai/edk2/MikanLoaderPkg/MikanLoaderPkg.dsc(...): error 4000: Instance of library class [RegisterFilterLib] is not found
        in [/home/dai/edk2/MdePkg/Library/BaseLib/BaseLib.inf] [X64]
        consumed by module [/home/dai/edk2/MikanLoaderPkg/Loader.inf]
```

build に失敗
なんか全部やってもらっているこのかんじいやだなあ

https://github.com/uchan-nos/os-from-zero/blob/main/faq.md#registerfilterlib-%E9%96%A2%E4%BF%82%E3%81%AE%E3%82%A8%E3%83%A9%E3%83%BC%E3%81%A7-mikanloaderpkg-%E3%81%8C%E3%83%93%E3%83%AB%E3%83%89%E3%81%A7%E3%81%8D%E3%81%BE%E3%81%9B%E3%82%93

dsc ファイルを変更してみた

build 成功

```
k2$  ls Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
```

```
 ~/osbook/devenv/run_qemu.sh Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
 ```

 Hello Mikan world が立ち上がった！

(Main.c)
 ```
 #include  <Uefi.h>
#include  <Library/UefiLib.h>

EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table) {
  Print(L"Hello, Mikan World!\n");
  while (1);
  return EFI_SUCCESS;
}
```

## メモリマップの取得

```
git checkout osbook_day02b
```

UEFI メモリマップの 1ページの大きさ $KB

今回は OS がメモリマップをきちんと把握できるように UEFI の機能を利用してメモリマップを取得してファイルに保存する。

ソースコードを読む前にいったんメモリマップというものをみてみる。

```
 source edksetup.sh 
 build
 ~/osbook/devenv/run_qemu.sh Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi

 $ ls mnt/
EFI  memmap
dai@1X21C094-PC:~/edk2$ cat mnt/memmap 
Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute
0, 3, EfiBootServicesCode, 00000000, 1, F
1, 7, EfiConventionalMemory, 00001000, 9F, F
2, 7, EfiConventionalMemory, 00100000, 700, F
3, A, EfiACPIMemoryNVS, 00800000, 8, F
4, 7, EfiConventionalMemory, 00808000, 8, F
5, A, EfiACPIMemoryNVS, 00810000, F0, F
6, 4, EfiBootServicesData, 00900000, B00, F
7, 7, EfiConventionalMemory, 01400000, 3AB36, F
8, 4, EfiBootServicesData, 3BF36000, 20, F
9, 7, EfiConventionalMemory, 3BF56000, 2726, F
10, 1, EfiLoaderCode, 3E67C000, 2, F
11, 4, EfiBootServicesData, 3E67E000, A, F
12, 9, EfiACPIReclaimMemory, 3E688000, 1, F
13, 4, EfiBootServicesData, 3E689000, 1F7, F
14, 3, EfiBootServicesCode, 3E880000, B4, F
15, A, EfiACPIMemoryNVS, 3E934000, 12, F
16, 0, EfiReservedMemoryType, 3E946000, 1C, F
17, 3, EfiBootServicesCode, 3E962000, 10A, F
18, 6, EfiRuntimeServicesData, 3EA6C000, 5, F
19, 5, EfiRuntimeServicesCode, 3EA71000, 5, F
20, 6, EfiRuntimeServicesData, 3EA76000, 5, F
21, 5, EfiRuntimeServicesCode, 3EA7B000, 5, F
22, 6, EfiRuntimeServicesData, 3EA80000, 5, F
23, 5, EfiRuntimeServicesCode, 3EA85000, 7, F
24, 6, EfiRuntimeServicesData, 3EA8C000, 8F, F
25, 4, EfiBootServicesData, 3EB1B000, 6F7, F
26, 7, EfiConventionalMemory, 3F212000, 4, F
27, 4, EfiBootServicesData, 3F216000, 6, F
28, 7, EfiConventionalMemory, 3F21C000, 1, F
29, 4, EfiBootServicesData, 3F21D000, 7FE, F
30, 7, EfiConventionalMemory, 3FA1B000, 1, F
31, 3, EfiBootServicesCode, 3FA1C000, 17F, F
32, 5, EfiRuntimeServicesCode, 3FB9B000, 30, F
33, 6, EfiRuntimeServicesData, 3FBCB000, 24, F
34, 0, EfiReservedMemoryType, 3FBEF000, 4, F
35, 9, EfiACPIReclaimMemory, 3FBF3000, 8, F
36, A, EfiACPIMemoryNVS, 3FBFB000, 4, F
37, 4, EfiBootServicesData, 3FBFF000, 201, F
38, 7, EfiConventionalMemory, 3FE00000, 8D, F
39, 4, EfiBootServicesData, 3FE8D000, 20, F
40, 3, EfiBootServicesCode, 3FEAD000, 20, F
41, 4, EfiBootServicesData, 3FECD000, 9, F
42, 3, EfiBootServicesCode, 3FED6000, 1E, F
43, 6, EfiRuntimeServicesData, 3FEF4000, 84, F
44, A, EfiACPIMemoryNVS, 3FF78000, 88, F
45, 6, EfiRuntimeServicesData, FFC00000, 400, 1
```

UEFI がメモリマップを作成する理由
ブーろプロセスの時に、OSが「使っていい場所」と「触ってはいけない場所」を間違えないようにするため です。

```
$ 
dai@1X21C094-PC:~/edk2$ cat /proc/iomem
00000000-00000000 : Reserved
00000000-00000000 : System RAM
00000000-00000000 : Reserved
00000000-00000000 : System ROM
00000000-00000000 : ACPI Tables
00000000-00000000 : System RAM
  00000000-00000000 : Kernel code
  00000000-00000000 : Kernel rodata
  00000000-00000000 : Kernel data
  00000000-00000000 : Kernel bss
00000000-00000000 : c4b741f5-5582-4c98-8f8b-2e082933c396
00000000-00000000 : 8b938bd5-438a-4b63-be12-06e889de266e
00000000-00000000 : bcb24690-39ff-4646-9b8d-5ad4a4d10d3e
00000000-00000000 : PNP0003:00
00000000-00000000 : Local APIC
  00000000-00000000 : PNP0003:00
00000000-00000000 : System RAM
00000000-00000000 : c4b741f5-5582-4c98-8f8b-2e082933c396
  00000000-00000000 : 5582:00:00.0
    00000000-00000000 : virtio-pci-modern
  00000000-00000000 : 5582:00:00.0
  00000000-00000000 : 5582:00:00.0
    00000000-00000000 : virtio-pci-modern
00000000-00000000 : 711dad3a-73ce-468b-90a9-ede6906841b2
00000000-00000000 : bcb24690-39ff-4646-9b8d-5ad4a4d10d3e
  00000000-00000000 : 39ff:00:00.0
    00000000-00000000 : virtio1
  00000000-00000000 : 39ff:00:00.0
    00000000-00000000 : virtio-pci-modern
  00000000-00000000 : 39ff:00:00.0
dai@1X21C094-PC:~/edk2$ 
```

wsl から現在のメモリマップにアクセスしてみた
しかし、wsl からはアクセスできないようだ。

## メモリマップコード読んでみるか。

```
code ~/devDir/mikan/c/[tag]
```

## ポインタの理解

ポインタとは変数のメモリ上にある場所（アドレス）を格納する変数
C 言語では変数が格納されているメモリ上のアドレス番号の先頭の値をポインタとしている。
一つのアドレスに 1 byte(8 bit) のデータを保存することができ、変数のデータの方によって、必要なアドレス数が異なる。

（例)
```
0x0000 
0x0001 0x004a
...
0x004a 5
```

関数内で引数のデータを変更したい場合、C 言語では変数の中身しか、引数として渡すことができない。
そのため、ポインタで渡さないといけない。

## CPU 視点

cpu 視点では、変数という概念がない。
そのため、コンパイルされたアセンブルは全てメモリ上のアドレスおよびレジスタの値の操作で事足りる。
だから、c 言語では、変数定義はコンパイル時にメモリ上の空間を確保するために行われる。
確保されたメモリはヒープ等を使用していない限り、関数が終了したら解放される。
そのアドレスはコンパイルの方法によって、固定することも、空いているところから最適化してしゅとくすることもかのうである。

- c 言語ファイル test.c

- コンパイル

- アセンブリファイル test.s
この段階ではもうすでにメモリ上のアドレスおよびレジスタの値の操作の羅列がある。(アドレスが決まっている場合はそのきさいもある。)
mov とか jmp とか人間は何をしているか把握可能

- アセンブル

- オブジェクトファイル test.o
逆アセンブルしない限り、人間には解読不可能
システムコールやほかのライブラリを使用するようなまだ処理 (外部シンボル（未解決関数やライブラリ）)が不明瞭なところはそのままである。  

- リンク
外部シンボル（未解決関数やライブラリ）に対して他のオブジェクトファイルをリンクさせて、一つの機械語プログラムを完成させる。
静的に処理を埋め込む方法と、実行時に動的に処理を読み込むように細工する方法の二つがある。

- 実行ファイル test.elf

## struct 構造体の理解

struct->member でメンバにアクセスできる原理は、機械語変換の時に、struct のポインタ+ そのメンバの前の変数のサイズをすべて足したものがポインタとなるからであるということ。

配列のように連続した空間に各メンバがそんざいする。

## UEFI について

ディスクのブート領域に書き込まれたプログラムで、デバイス(ファームウェア) によって必ず起動時に読み込まれる。
PC やサーバーがシャットダウンしているときはメモリ上 (揮発性 RAM)にデータが 0 の状態となっている。
そのため、EC2 インスタンスを起動したときやシャットダウン状態の PC を起動したときには毎回動作するプログラムである。
具体的には、メモリ上にカーネルを読み込ませたりしている。


