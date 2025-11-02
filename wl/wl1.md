
がんばるぞー

x サーバーのダウンロード

```
sudo apt install qemu-utils ak
```

## 環境

```
$ cat /etc/os-release 
PRETTY_NAME="Ubuntu 22.04.1 LTS"
NAME="Ubuntu"
VERSION_ID="22.04"
VERSION="22.04.1 LTS (Jammy Jellyfish)"
VERSION_CODENAME=jammy
ID=ubuntu
ID_LIKE=debian
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
UBUNTU_CODENAME=jammy

$ uname -a
Linux 1X21C094-PC 5.15.133.1-microsoft-standard-WSL2 #1 SMP Thu Oct 5 21:02:42 UTC 2023 x86_64 x86_64 x86_64 GNU/Linux
dai@1X21C094-PC:~/devDir/mikan$ 
```

```
$ free -h
               total        used        free      shared  buff/cache   available
Mem:           3.7Gi       1.3Gi       447Mi       2.0Mi       2.0Gi       2.1Gi
Swap:          1.0Gi          0B       1.0Gi
```


> このリポジトリは uchan が開発している教育用 OS MikanOS をビルドする手順およびツールを収録しています。 Ubuntu 18.04 で動作を確認しています

まあいっか

## 実験

BOOTX64.EFI をさくせい

```
$ sum work/BOOTX64.EFI 
26913     2
```

バイナリエディタ向いていない。okteta
一生ムリだ

泣く泣くダウンロードしてきた

```
$ diff work/BOOTX64.EFI mikanos-build/day01/bin/hello.efi 
Binary files work/BOOTX64.EFI and mikanos-build/day01/bin/hello.efi differ
```

うーん違うのか
一回自作のやつで残りの工程を行ってみよう

今回検証用のPC も USB も存在しないのでいったん qemu でおこないます。

```
$ qemu-img create -f raw disk.img 200M
Formatting 'disk.img', fmt=raw size=209715200

$$ mkfs.fat -n "MIKAN OS" -s 2 -f 2 -R 32 -F 32 disk.img
mkfs.fat 4.2 (2021-01-31)

$ sudo mount  -o loop disk.img mnt/

$ lsblk -f
NAME  FSTYPE FSVER LABEL UUID FSAVAIL FSUSE% MOUNTPOINTS
loop0                          198.4M     0% /home/dai/devDir/mikan/mnt
sda                                          
sdb                                          [SWAP]
sdc                            943.7G     1% /mnt/wslg/distro

```

マウントされた
vfat が　uefi の標準のファイルシステムかー
200 M のブート領域なのはみたことあるな

```
$ sudo mkdir -p mnt/EFI/BOOT/BOOTX64.EFI
$ sudo umount mnt
```

img.disk に EFIファイルがコピーされた

```
$ qemu-system-x86_64 \
-drive if=pflash,file=$HOME/osbook/devenv/OVMF_CODE.fd \
-drive if=pflash,file=$HOME/osbook/devenv/OVMF_VARS.fd \
-hda disk.img
```

```
sudo apt install qemu-system-x86   
```

-drive は BIOS ではなく QEMU エミュレータを UEFI で起動させたいがためであるらしい。

```
$ qemu-system-x86_64 \
-drive if=pflash,file=$HOME/osbook/devenv/OVMF_CODE.fd \
-drive if=pflash,file=$HOME/osbook/devenv/OVMF_VARS.fd \
-hda disk.img
WARNING: Image format was not specified for '/home/dai/osbook/devenv/OVMF_CODE.fd' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
WARNING: Image format was not specified for '/home/dai/osbook/devenv/OVMF_VARS.fd' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
WARNING: Image format was not specified for 'disk.img' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
```

なんか警告出たけど

でも hello world は表示されない
スクリプトも使ってみるのだ

エミュレータ重すぎてプロセスきる方法しかわからない

```
$HOME/osbook/devenv/run_qumu.sh $HOME/day01/bin/hello.efi
```

```
 ~/osbook/devenv/run_qemu.sh work/hello.efi 
 ```

 こんな感じで勝手に UEFI のブート領域にマウントした QUME エミュレータを起動してくれる機能もある。
 sugoi

 ちなみに自分の作成したバイナリファイルでは表示できないことがわかったのだ、、、

 ### 感想

 とりあえず UEFI のブート領域に PE/CONF 形式の efi ファイルを置いて、 helloworld が読み込まれることがわかった。
 最終的にはカーネルを読み込むような動作が入ってくるのだろうか
 今のところファイルシステムが fat の 200 M のディスクで qemu を起動しているが、今後どのようにして fxs のような一般的なファイルシステムをさくせいするのだろうか
また、そのような fat のパーティションがある形式を GPT と言いそうだな

## ビルドしてみる

```
$ clang  -target x86_64-pc-win32-coff \
-mno-red-zone -fno-stack-protector -fshort-wchar -Wall -c ~/osbook/day01/c/hello.c 
```

clang は gcc みたいなやつ
hello.o というオブジェクトファイルが作成された

COFF ファイルというのもを作成

```
 lld-link /subsystem:efi_application /entry:EfiMain /out:hello.efi hello.o
````

lld も ld みたいなやつ
PE ファイル形式でさくせいされる。


一切パッケージを使用しない hello world
hello.c
```
typedef unsigned short CHAR16;
typedef unsigned long long EFI_STATUS;
typedef void *EFI_HANDLE;

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (*EFI_TEXT_STRING)(
  struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  CHAR16                                   *String);

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  void             *dummy;
  EFI_TEXT_STRING  OutputString;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
  char                             dummy[52];
  EFI_HANDLE                       ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
} EFI_SYSTEM_TABLE;

EFI_STATUS EfiMain(EFI_HANDLE        ImageHandle,
                   EFI_SYSTEM_TABLE  *SystemTable) {
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello, world!\n");
  while (1);
  return 0;
}
```

