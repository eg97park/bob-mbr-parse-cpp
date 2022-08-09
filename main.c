#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>


const uint8_t NTFS = 0x07;
const uint8_t FAT32_CHS = 0x08;
const uint8_t FAT32_LBA = 0x0C;
const uint8_t EXTENDED_PARTITION_CHS = 0x05;
const uint8_t EXTENDED_PARTITION_LBA = 0x0F;


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


void printBlanc(int refCnt);
int printPartitionInfo(uint8_t* img_ptr_, const PARTITION_TABLE_ENTRY* entry_);
int printEbrInfo(const uint8_t* img_ptr_, const uint8_t* cur_img_ptr, const uint32_t mbrEbrSector, const uint32_t lastEbrSector);
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
    if (printPartitionInfo(buff, &(brHdr->part1)) != 0){
        free(buff);
        buff = NULL;
        brHdr = NULL;
        return -1;
    }
    if (printPartitionInfo(buff, &(brHdr->part2)) != 0){
        free(buff);
        buff = NULL;
        brHdr = NULL;
        return -1;
    }
    if (printPartitionInfo(buff, &(brHdr->part3)) != 0){
        free(buff);
        buff = NULL;
        brHdr = NULL;
        return -1;
    }
    if (printPartitionInfo(buff, &(brHdr->part4)) != 0){
        free(buff);
        buff = NULL;
        brHdr = NULL;
        return -1;
    }

    free(buff);
    buff = NULL;
    brHdr = NULL;
    return 0;
}


void printBlanc(int refCnt){
    for (int _ = 0; _ < refCnt; _++){
        printf("   ");
    }
}


int printPartitionInfo(uint8_t* img_ptr_, const PARTITION_TABLE_ENTRY* entry_){
    static int currentEbrRefCnt = 0;
    const uint8_t type = entry_->type;
    const uint32_t lbaAddr = (entry_->lbaAddr);
    const uint32_t size = entry_->size;
    if (type == NTFS){
        printf("NTFS %d %d\n", lbaAddr * 512, size * 512);
    }
    else if (type == FAT32_CHS || type == FAT32_LBA){
        printf("FAT32 %d %d\n", lbaAddr * 512, size * 512);
    }
    else if (type == EXTENDED_PARTITION_CHS || type == EXTENDED_PARTITION_LBA){
        printf("EXTENDED_PARTITION %d %d\n", lbaAddr * 512, size * 512);
        if (printEbrInfo(img_ptr_, img_ptr_ + (lbaAddr * 512), lbaAddr, lbaAddr) != 0){
            printf("@printEbrInfo ERROR\n");
            return -1;
        }
    }
    else{
        return -1;
    }
    return 0;
}


int printEbrInfo(const uint8_t* img_ptr_, const uint8_t* cur_img_ptr, const uint32_t mbrEbrSector, const uint32_t lastEbrSector){
    static int refCnt = 0;
    refCnt++;

    const EBR* EBR_ = (const EBR*)(cur_img_ptr);
    const uint8_t partType = EBR_->part.type;
    const uint8_t nextType = EBR_->next.type;

    if (partType == NTFS){
        printBlanc(refCnt);
        printf("NTFS %d %d\n", (EBR_->part.lbaAddr + lastEbrSector) * 512, (EBR_->part.size) * 512);
    }
    else if (partType == FAT32_CHS || partType == FAT32_LBA){
        printBlanc(refCnt);
        printf("FAT32 %d %d\n", (EBR_->part.lbaAddr + lastEbrSector) * 512, (EBR_->part.size) * 512);
    }
    else{
        return -1;
    }

    if (nextType == EXTENDED_PARTITION_CHS || nextType == EXTENDED_PARTITION_LBA){
        printBlanc(refCnt);
        printf("EXTENDED_PARTITION %d %d\n", (EBR_->next.lbaAddr + mbrEbrSector) * 512, (EBR_->next.size) * 512);
        printEbrInfo(img_ptr_, img_ptr_ + (EBR_->next.lbaAddr + mbrEbrSector) * 512, mbrEbrSector, EBR_->next.lbaAddr + lastEbrSector);
    }
    else{
        return -1;
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
