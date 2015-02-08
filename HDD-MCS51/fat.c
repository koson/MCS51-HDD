#include "fat.h"
#define DRIVE0		0
#include "ata.h"
#include <stdio.h>
#include "rprintf.h"
#include <string.h>

#define DEBUG_FAT 1
#define WE_INFO 1

struct partrecord idata PartInfo;
unsigned char idata Fat32Enabled;
unsigned long idata FirstDataSector;
unsigned int  idata BytesPerSector;
unsigned int  idata SectorsPerCluster;
unsigned long idata FirstFATSector;
unsigned long idata FirstDirSector;
unsigned long idata FileSize;
unsigned long idata FatInCache = 0;
unsigned char *SectorBuffer;
extern unsigned char sectordata[512];
unsigned char DirBuffer[128];
unsigned char LongBuffer[32];
unsigned char *DirNameBuffer;
unsigned char *LongNameBuffer;

//-----------------------------------------------------------------------------
// IDE_SectorWord: Function is used to retrieve a word from the 'currentsector'
// array. The value passed in must be between 0 - 510.
//
// Parameters: Word location within currentsector
//
// Returns: Word of data requested from currentsector
//-----------------------------------------------------------------------------
WORD IDE_SectorWORD(WORD wtc) // Return Word at position specified
{
	 idata WORD dat=0; 
	 idata WORD A,B;
	 
	 A =  wtc & 0xff00;
	 B =  wtc & 0x00ff;
	 dat=(B <<= 8) + (A >>= 8); 		  

	 return (dat);					  			 // Return value
}

//-----------------------------------------------------------------------------
// IDE_SectorUI32: Function is used to retrieve a 32 bit value from the 
// 'currentsector' array. The value passed in must be between 0 - 511.
//
// Parameters: Word offset within current sector
//
// Returns: 32 bit unsigned data as requested
//
// Functions Used: None
//
//-----------------------------------------------------------------------------
DWORD IDE_SectorDWORD(DWORD dwtc) // Return UI32 at position specified
{
    DWORD dat=0; 
	DWORD A,B,C,D;
	
	A = dwtc & 0xff000000;		// Read the four bytes which make up
	B = dwtc & 0x00ff0000;    // the 32-bit value
	C = dwtc & 0x0000ff00;
	D = dwtc & 0x000000ff;         

    dat=(D <<= 24) + (C <<= 8) + ( B >>= 8) + (A >>= 24); //Combine into correct order
	return (dat);					  			 // Return value
}

unsigned long fatClustToSect(unsigned long clust)
{
   
    printf("(("); rprintfu32(clust) ; printf("-2)*");
    rprintfu16(SectorsPerCluster);  printf(")+");  rprintfu32(FirstDataSector);
    printf("=");


	return ((clust-2) * SectorsPerCluster) + FirstDataSector;
}

unsigned int fatClusterSize(void)
{
	// return the number of sectors in a disk cluster
	return SectorsPerCluster;
}

unsigned char fatInit( unsigned char device)
{
	//struct partrecord *pr;
	struct bpb710 *bpb;

	// read partition table
	// TODO.... error checking
	SectorBuffer = sectordata;
	ataReadSectors(device, 0, 1, SectorBuffer);
	// map first partition record	
	// save partition information to global PartInfo
	PartInfo = *((struct partrecord *) ((struct partsector *) SectorBuffer)->psPart);
//	PartInfo = *pr;
	
	// Read the Partition BootSector
	// **first sector of partition in PartInfo.prStartLBA
	ataReadSectors( device, IDE_SectorDWORD(PartInfo.prStartLBA), 1, SectorBuffer );
	bpb = (struct bpb710 *) ((struct bootsector710 *) SectorBuffer)->bsBPB;


	// setup global disk constants
	FirstDataSector	= IDE_SectorDWORD(PartInfo.prStartLBA);
	if(bpb->bpbFATsecs)
	{
		// bpbFATsecs is non-zero and is therefore valid
		FirstDataSector	+= IDE_SectorWORD(bpb->bpbResSectors) + bpb->bpbFATs * IDE_SectorWORD(bpb->bpbFATsecs);
	}
	else
	{
		// bpbFATsecs is zero, real value is in bpbBigFATsecs
		FirstDataSector	+= IDE_SectorWORD(bpb->bpbResSectors) + bpb->bpbFATs * IDE_SectorDWORD(bpb->bpbBigFATsecs);
	}
	SectorsPerCluster	= bpb->bpbSecPerClust;
	BytesPerSector		= IDE_SectorWORD(bpb->bpbBytesPerSec);
	FirstFATSector		= IDE_SectorWORD(bpb->bpbResSectors) + IDE_SectorDWORD(PartInfo.prStartLBA);

	switch (PartInfo.prPartType)
	{
		case PART_TYPE_DOSFAT16:
		case PART_TYPE_FAT16:
		case PART_TYPE_FAT16LBA:
			// first directory cluster is 2 by default (clusters range 2->big)
			FirstDirSector	= CLUST_FIRST;
			// push data sector pointer to end of root directory area
			//FirstDataSector += (bpb->bpbRootDirEnts)/DIRENTRIES_PER_SECTOR;
			Fat32Enabled = FALSE; // 
			break;
		case PART_TYPE_FAT32LBA:
		case PART_TYPE_FAT32:
			// bpbRootClust field exists in FAT32 bpb710, but not in lesser bpb's
			FirstDirSector = IDE_SectorDWORD(bpb->bpbRootClust);
			// push data sector pointer to end of root directory area
			// need this? FirstDataSector += (bpb->bpbRootDirEnts)/DIRENTRIES_PER_SECTOR;
			Fat32Enabled = TRUE;
			break;
		default:
			printf("Found: No Partition!\r\n");
			//return 1;
			break;
	}


	switch (PartInfo.prPartType)
	{
		case PART_TYPE_DOSFAT16:
				printf("Found: DOSFAT 16\r\n");
				break;
		case PART_TYPE_FAT16:
				printf("Found: FAT16\r\n");
				break;
		case PART_TYPE_FAT16LBA:
				printf("Found: FAT16 LBA\r\n");
				break;
		case PART_TYPE_FAT32LBA:
				printf("Found: FAT32 LBA\r\n");
				break;
		case PART_TYPE_FAT32:
				printf("Found: FAT32\r\n");
				//return 1;	
				break;
		default:
				printf("Found: No Partition!\r\n");
				//return 1;
				break;
	}

#ifdef DEBUG_FAT
	printf("BPB [bpb710]:-----------------\n\r");
	printf("bpbBytesPerSec : ");	rprintfu16(IDE_SectorWORD(bpb->bpbBytesPerSec));	rprintfCRLF();
	printf("bpbSecPerClust : ");	rprintfu08(bpb->bpbSecPerClust);					rprintfCRLF();
	printf("bpbResSectors  : ");	rprintfu16(IDE_SectorWORD(bpb->bpbResSectors));	rprintfCRLF();
	printf("bpbFATs	       : ");	rprintfu08(bpb->bpbFATs);							rprintfCRLF();
	printf("bpbRootDirEnts : ");	rprintfu16(IDE_SectorWORD(bpb->bpbRootDirEnts));	rprintfCRLF();
 	printf("bpbSectors	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbSectors));		rprintfCRLF();
	printf("bpbMedia	   : ");	rprintfu08(bpb->bpbMedia);							rprintfCRLF();
	printf("bpbFATsecs	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbFATsecs));		rprintfCRLF();
	printf("bpbSecPerTrack : ");	rprintfu16(IDE_SectorWORD(bpb->bpbSecPerTrack));	rprintfCRLF();
	printf("bpbHeads	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbHeads));			rprintfCRLF();
	printf("bpbHiddenSecs  : ");	rprintfu32(IDE_SectorDWORD(bpb->bpbHiddenSecs));	rprintfCRLF();
	printf("bpbHugeSectors : ");	rprintfu32(IDE_SectorDWORD(bpb->bpbHugeSectors));	rprintfCRLF();
	printf("bpbBigFATsecs  : ");	rprintfu32(IDE_SectorDWORD(bpb->bpbBigFATsecs));	rprintfCRLF();
	printf("bpbExtFlags	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbExtFlags));		rprintfCRLF();
	printf("bpbFSVers	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbFSVers));		rprintfCRLF();
	printf("bpbRootClust   : ");	rprintfu32(IDE_SectorDWORD(bpb->bpbRootClust));		rprintfCRLF();
	printf("bpbFSInfo	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbFSInfo));		rprintfCRLF();
	printf("bpbBackup	   : ");	rprintfu16(IDE_SectorWORD(bpb->bpbBackup));		rprintfCRLF();
	printf("End [bpb710]--------------------\n\r");
	printf("First sector    : ");		rprintfu32(IDE_SectorDWORD(PartInfo.prStartLBA));	rprintfCRLF();
	printf("Size            : ");	rprintfu32(IDE_SectorDWORD(PartInfo.prSize));		rprintfCRLF();
	printf("First Fat Sector: ");	rprintfu32(FirstFATSector);			rprintfCRLF();
	printf("First Data Sect : ");	rprintfu32(FirstDataSector);		rprintfCRLF();
	printf("First Dir Clust : ");	rprintfu32(FirstDirSector);			rprintfCRLF();
#endif
	return 0;	
}

unsigned int baseentry = 0;
unsigned int entrycount = 0;

unsigned long fatGetDirEntry(unsigned int entry, unsigned int count)
{
	unsigned long idata sector;
	struct direntry *de = 0;	// avoid compiler warning by initializing
	struct winentry *we;
	unsigned int idata hasBuffer;
	unsigned int idata b;
	int i,index;
	char *p;
    DirNameBuffer  = &DirBuffer ;
    LongNameBuffer = &LongBuffer;	
	if(count == 0)
	{
		entrycount = 0;
		DirNameBuffer = 0;
	}
	
    // read dir data
	sector = fatClustToSect(FirstDirSector);
	hasBuffer = 0;
	index = 16;	// crank it up
	do 
	{
		if(index == 16)	// time for next sector ?
		{
			ataReadSectors( DRIVE0, sector++, 1, SectorBuffer);
			de = (struct direntry *) SectorBuffer;
			index = 0;
            
		}	
 	    // printf("found directory : ");
        // printf("%s\r\n",de->deName);

	
		if(*de->deName != 0xE5)
		{
  	//printf("Direntry :-----------------\n");
			//
            // if not a deleted entry
			if(de->deAttributes == ATTR_LONG_FILENAME)
			{
			    printf("Long name entry\r\n",p);
				// we have a long name entry
				we = (struct winentry *) de;
				b = 13 *( (we->weCnt-1) & 0x0f);				// index into string
				p = &LongNameBuffer[b];
				for (i=0;i<5;i++)	*p++ = we->wePart1[i*2];	// copy first part			
				for (i=0;i<6;i++)	*p++ = we->wePart2[i*2];	// second part
				for (i=0;i<2;i++)	*p++ = we->wePart3[i*2];	// and third part
				if (we->weCnt & 0x40) *p = 0;					// in case dirnamelength is multiple of 13
				if ((we->weCnt & 0x0f) == 1) hasBuffer = 1;		// mark that we have a long entry
               // 
        #ifdef WE_INFO        
                printf("weCnt : ");	        rprintfu08(we->weCnt);                          rprintfCRLF();
                printf("weAttributes : ");  rprintfu08(we->weAttributes);	                rprintfCRLF();
            	printf("weReserved1 : ");   rprintfu08(we->weReserved1);	                rprintfCRLF();
            	printf("weChksum : ");         rprintfu08(we->weChksum);	                rprintfCRLF();
            	printf("weReserved2: ");    rprintfu08(IDE_SectorWORD(we->weReserved2));	rprintfCRLF();
        #endif



            }
			else
			{  
			    //printf("Short name entry\r\n");
            	//printf("\r\n");
                rprintfStrLen(de->deName,0,8); 
            	printf(".");
                rprintfStrLen(de->deExtension,0,3);  rprintfCRLF();
                printf("deAttributes: ");	 rprintfu08(de->deAttributes); 	rprintfCRLF();
            	printf("deHighClust:"); rprintfu16(IDE_SectorWORD(de->deHighClust)); 	rprintfCRLF();
            	printf("deStartCluster:"); rprintfu16(IDE_SectorWORD(de->deStartCluster));	rprintfCRLF();
                printf("StartSector:"); rprintfu32(fatClustToSect(IDE_SectorWORD(de->deStartCluster)));rprintfCRLF(); 
                printf("deFileSize: %ld byte",IDE_SectorDWORD(de->deFileSize )) ;	rprintfCRLF();
				// we have a short name entry
				// check if this is the end of a multi-part long name entry
				if(hasBuffer)
				{
					// a long entry name has been collected
					// is it a directory ?
				
                    if(de->deAttributes == ATTR_DIRECTORY)
					{
						unsigned long save = FirstDirSector;
						unsigned int save2 = baseentry;
						unsigned long rval;
						
						strcpy(DirNameBuffer,LongNameBuffer);
						strcat(DirNameBuffer,"/");

						//printf(LongNameBuffer); printf("/"); //EOL();
                        printf("LongNameBuffer = %s\r\n",LongNameBuffer );

						// call recursively
						FirstDirSector = ((unsigned long)de->deHighClust << 16) + de->deStartCluster;
						//rval = fatGetDirEntry(entry,1);
                        rval = 0;
						FirstDirSector = save;
						baseentry = save2;
						if (rval){
                            printf("rval: "); rprintfu32(rval); rprintfCRLF();
							return rval;
                            
                        
                        }
						else	
						{
							// reload original sector
							ataReadSectors( DRIVE0,	sector-1, 1, SectorBuffer);
							entrycount--;			// decrement entry counter		
							*DirNameBuffer = 0;
                 	}
                    
					}
					else // normal file entry
						if(entrycount == entry)		
							break;
                    printf("Normal File entry...\r\n");
					hasBuffer = 0;	// clear buffer	
					entrycount++;	// increment entry counter		
				}
				// else ignore short_name_only entries 
			}
		}
		de++;
		index++;
	}	while (*de->deName || index == 16);	// 0 in de->deName[0] if no more entries

	if (hasBuffer == 0)		// end of entries
		return 0;
	
	FileSize = de->deFileSize;
	return (unsigned long) ((unsigned long)de->deHighClust << 16) + de->deStartCluster;
}

// return the size of the last directory entry
unsigned long fatGetFilesize(void)
{
	return FileSize;
}


// return the long name of the last directory entry
char* fatGetFilename(void)
{	
	return LongNameBuffer;
}


// return the directory of the last directory entry
char* fatGetDirname(void)
{	
	return DirNameBuffer;
}


// load a clusterfull of data
void fatLoadCluster(unsigned long cluster, unsigned char *buffer)
{
	unsigned char idata i;
	// read cluster
	//while ( ataReadSectors( DRIVE0, clust2sect(cluster), SectorsPerCluster, buffer) != 0);
	for(i=0; i<SectorsPerCluster; i++)
	{
//		ataReadSectors( DRIVE0, clust2sect(cluster)+i, 1, buffer+(i<<9) );
		// temporary fix for wierd misaligned cluster problem
		// (only when using FAT16?)
		ataReadSectors( DRIVE0, fatClustToSect(cluster+8)+i, 1, buffer+(i<<9) );
	}
}


// find next cluster in the FAT chain
unsigned long fatNextCluster(unsigned long cluster)
{
	unsigned long nextCluster;
	unsigned long fatMask;
	unsigned long fatOffset;
	unsigned long sector;
	unsigned int offset;

    unsigned int FAT_CACHE_ADDR;
	FAT_CACHE_ADDR = &sectordata;
	// get fat offset in bytes
	if(Fat32Enabled)
	{
		// four FAT bytes (32 bits) for every cluster
		fatOffset = cluster << 2;
		// set the FAT bit mask
		fatMask = FAT32_MASK;
	}
	else
	{
		// two FAT bytes (16 bits) for every cluster
		fatOffset = cluster << 1;
		// set the FAT bit mask
		fatMask = FAT16_MASK;
	}
	
	// calculate the FAT sector that we're interested in
	sector = FirstFATSector + (fatOffset / BytesPerSector);
	// calculate offset of the our entry within that FAT sector
	offset = fatOffset % BytesPerSector;

	// if we don't already have this FAT chunk loaded, go get it
	if (sector != FatInCache)
	{
		// read sector of FAT table
		while (ataReadSectors( DRIVE0, sector, 1, (unsigned char*)FAT_CACHE_ADDR) != 0);
		FatInCache = sector;
	}

	// read the nextCluster value
	nextCluster = (*((unsigned long*) &((char*)FAT_CACHE_ADDR)[offset])) & fatMask;

	// check to see if we're at the end of the chain
	if (nextCluster == (CLUST_EOFE & fatMask))
		nextCluster = 0;

#ifdef DEBUG_FAT
	printf(">");
	rprintfu32(nextCluster);
	rprintfCRLF();
#endif
	
	return nextCluster;
}

