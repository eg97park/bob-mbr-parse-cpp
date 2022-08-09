#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>


#pragma pack(1) 
typedef struct PARTITION_TABLE_ENTRY_
{
    uint8_t boot;
    uint8_t chsAddr1[3];
    uint8_t type;
    uint8_t chsAddr2[3];
    uint32_t lbaAddr;
    uint32_t size;
}PARTITION_TABLE_ENTRY;


typedef struct BR_
{
    uint8_t bootCode[446];
    PARTITION_TABLE_ENTRY part1;
    PARTITION_TABLE_ENTRY part2;
    PARTITION_TABLE_ENTRY part3;
    PARTITION_TABLE_ENTRY part4;
    uint8_t bootRecordSignature[2];
}BR;


typedef struct EBR_
{
    uint8_t unused1[446];
    PARTITION_TABLE_ENTRY part;
    PARTITION_TABLE_ENTRY next;
    PARTITION_TABLE_ENTRY unused2;
    PARTITION_TABLE_ENTRY unused3;
    uint8_t bootRecordSignature[2];
}EBR;


uint32_t printEbrInfo(uint8_t* buff_, uint8_t* cur_buff_, const uint32_t firstEbrSector, const uint32_t lastEbrSector);
int printPartitionInfo(uint8_t* buff_, const PARTITION_TABLE_ENTRY* PARTITION_TABLE_ENTRY_, const uint32_t firstEbrSector, const uint32_t lastEbrSector);
uint8_t* readFile(const char* filePath_, size_t* sz_);


int main(int argc, char* argv[]){
    if (argc != 2){
        printf("usage: %s [IMAGE_FILE]\n", argv[0]);
        return 1;
    }

    size_t readSize = 0;
    uint8_t* buff = readFile(argv[1], &readSize);
    if (buff == NULL){
        printf("@readFile error\n");
        return -1;
    }

    BR* brHdr = (BR*)buff;
    printPartitionInfo(buff, &(brHdr->part1), 0, 0);
    printPartitionInfo(buff, &(brHdr->part2), 0, 0);
    printPartitionInfo(buff, &(brHdr->part3), 0, 0);
    printPartitionInfo(buff, &(brHdr->part4), 0, 0);

    free(buff);
    brHdr = NULL;
    buff = NULL;
    return 0;
}


uint32_t printEbrInfo(uint8_t* buff_, uint8_t* cur_buff_, const uint32_t firstEbrSector, const uint32_t lastEbrSector){
    static int refCnt = 0;
    refCnt++;
    
    EBR* EBR_ = (EBR*)(cur_buff_);
    switch (EBR_->part.type)
    {
    case 0x07:
        printf("NTFS %x %x\n", (EBR_->part.lbaAddr + lastEbrSector) * 512, (EBR_->part.size) * 512);
        break;
    case 0x0B:
        printf("FAT32 %x %x\n", (EBR_->part.lbaAddr + lastEbrSector) * 512, (EBR_->part.size) * 512);
        break;
    case 0x0C:
        printf("FAT32 %x %x\n", (EBR_->part.lbaAddr + lastEbrSector) * 512, (EBR_->part.size) * 512);
        break;
    case 0x05:
        printf("???@0");
        return 0;
        break;
    default:
        break;
    }

    switch (EBR_->next.type)
    {
    case 0x07:
        printf("???@1");
        return 0;
        break;
    case 0x0B:
        printf("???@2");
        return 0;
        break;
    case 0x0C:
        printf("???@3");
        return 0;
        break;
    case 0x05:
        printf("EBR %x %x\n", (EBR_->next.lbaAddr + firstEbrSector) * 512, (EBR_->next.size) * 512);
        if (refCnt == 1){
            printEbrInfo(buff_, buff_ + (EBR_->next.lbaAddr + firstEbrSector) * 512, firstEbrSector, EBR_->next.lbaAddr + lastEbrSector);
        }
        else{
            printEbrInfo(buff_, buff_ + (EBR_->next.lbaAddr + firstEbrSector) * 512, firstEbrSector, EBR_->next.lbaAddr + lastEbrSector);
        }
        break;
    default:
        break;
    }
    return 0;
}


int printPartitionInfo(uint8_t* buff_, const PARTITION_TABLE_ENTRY* PARTITION_TABLE_ENTRY_, const uint32_t firstEbrSector, const uint32_t lastEbrSector){
    static int currentEbrRefCnt = 0;
    const uint8_t type = PARTITION_TABLE_ENTRY_->type;
    const uint32_t lbaAddr = (PARTITION_TABLE_ENTRY_->lbaAddr) + lastEbrSector;
    const uint32_t size = PARTITION_TABLE_ENTRY_->size;
    switch (type)
    {
    case 0x07:
        printf("NTFS %x %x\n", lbaAddr * 512, size * 512);
        break;
    case 0x0B:
        printf("FAT32 %x %x\n", lbaAddr * 512, size * 512);
        break;
    case 0x0C:
        printf("FAT32 %x %x\n", lbaAddr * 512, size * 512);
        break;
    case 0x05:
        printf("EBR %x %x\n", lbaAddr * 512, size * 512);
        printEbrInfo(buff_, buff_ + (lbaAddr * 512), lbaAddr, lbaAddr);
    default:
        break;
    }
    return 0;
}


uint8_t* readFile(const char* filePath_, size_t* sz_){
    FILE* fp = fopen(filePath_, "rb");
    if (fp == NULL){
        printf("@fopen error\n");
        return NULL;
    }

    if (fseek(fp, 0L, SEEK_END) != 0){
        printf("@fseek error\n");
        fclose(fp);
        return NULL;
    }

    size_t sz = ftell(fp);
    if (sz == -1){
        printf("@ftell error\n");
        fclose(fp);
        return NULL;
    }
    *sz_ = sz;
    rewind(fp);

    uint8_t* buff = (uint8_t*)malloc(sizeof(uint8_t) * sz);
    if (buff == NULL){
        printf("@malloc error\n");
        fclose(fp);
        return NULL;
    }

    size_t rsz = fread(buff, sizeof(uint8_t), sz, fp);
    if (rsz != sz){
        printf("@fread error\n");
        free(buff);
        buff = NULL;
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return buff;
}

