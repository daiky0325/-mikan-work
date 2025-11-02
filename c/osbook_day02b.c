#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>

// 上記のヘッダファイルを通じてリンク時に標準ライブラリが使えるようになっている

struct MemoryMap {
    UINTN buffer_size; 
    VOID* buffer;
    UINTN map_size;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;
};

// UINTN はポインタサイズ

EFI_STATUS GetMemoryMap(struct MemoryMap* map){

    //buffers つまりメモリーが確保できているかちぇっく

    if (map->buffer == NULL){
        return EFI_BUFFER_TOO_SMALL;
    }

    map->map_size= map->buffer_size;
    return gBS->GetMemoryMap(
        &map->map_size, //メモリマップの大きさ
        (EFI_MEMORY_DESCRIPTOR*)map->buffer, //メモリ領域の先頭ポインタ
        &map->map_key,
        &map->descriptor_size, //メモリマップの各行のサイズ。
        &map->descriptor_version
    );
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}

EFI_STATUS SaveMemoryMap(struct MemoryMap* map,EFI_FILE_PROTOCOL* file){
    CHAR8 buf[256]; // 配列確保
    UINTN len;

    CHAR8* header=
    "Index,Type(name),PysicalStart,NuberOfPages,Attribute \nz";
    len =AsciiStrlen(header);
    file->Write(file,&len,header); // ヘッダを書き込み(ファイル名、ヘッダ長、ヘッダ)

    EFI_PHYSICAL_ADDRESS iter; // アドレスポインタ
    int i;
    for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i=0; //メモリマップの先頭アドレスに合わせる、
        iter <(EFI_PHYSICAL_ADDRESS)map->buffer+map->map_size; // 確保したメモリマップを出たら終了
        iter+= map ->descriptor_size, i++) {//各ディスクリプタ―を追加していく

        EFI_MEMORY_DESCRIPTOR* desc=(EFI_MEMORY_DESCRIPTOR*)iter; // for のために整数型としていた iter をポインタが単変換
        len= AsciiPrint(
            buf, sizeof(buf),
            "%u, %x ,%-ls,%08lx,%lx,%lx\n",
            i,desc->Type,GetMemoryTypeUnicode(desc->Type),
            desc-> PhysicalStart,desc->NumberOfPages,
            desc->Attribute & 0xffffflu); // buf にデータを読み込む関数 戻り値が len になる。
        file->Write(file,&len,buf);
        }
    return EFI_SUCCESS;
    };

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root){
    EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

    // uefi が稼働しているアプリケーションのルートディレクトリを開くためのプログラム

    gBS->OpenProtocol( // どのデバイスから起動されたかがわかる。
        image_handle,
        &gEfiLoadedImageProtocolGuid,
        (VOID**)&loaded_image,
        image_handle,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
    );

    gBS->OpenProtocol( //ファイルシステムプロ所
        loaded_image->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&fs,
        image_handle,
        NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
    );

    fs->OpenVolume(fs,root); // ルートディレクトリを開く

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain( // 以下の変数は uefi 起動時に渡されたもの
        EFI_HANDLE image_handle,
        EFI_SYSTEM_TABLE* system_table
){

    CHAR8 memmap_buf[4096*4]; //16ページ分確保
    struct MemoryMap memmap ={sizeof(memmap_buf),memmap_buf,0,0,0,0};
    GetMemoryMap(&memmap); 

    EFI_FILE_PROTOCOL* root_dir; // ポインタで定義される。
    OpenRootDir(image_handle,&root_dir);

    EFI_FILE_PROTOCOL*memmap_file;
    root_dir->Open(
        root_dir,&memmap_file,L"\\memmap",
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,0
    );
    
    SaveMemoryMap(&memmap,memmap_file);
    memmap_file->Close(memmap_file);

    while(1);
    return EFI_SUCCESS;

}