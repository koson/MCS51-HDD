/* ==============================================

Project : IDE-51
Purpose : Interface IDE Harddisk with AT89C51AC2
Date	: Revision 3 2006.04.27
		: Revision 5 2006.05.04 
Author	: ktkoson@kmitl.ac.th

=================================================*/

#include <REG51AC2.h>
#include <stdio.h>
#include <stdlib.h>
#include "rprintf.h"
#include "compdef.h"
#include "ide_51.h"
#define DEBUG_ATA 1



main()
{
	unsigned int i = 0;
  	long reg,dat;
  	char *p;
	char s[] = "     ";

   	AUXR = 0x0C;    //' T89C51AC2 CONFIG SELECT IRAM = 1024 BYTE.
   	Init_serial(); 	//' SERIAL SETTING.
	//IT0 = 1;  		//' Falling edge interrupt 0
	//EX0 = 1;		//' Enable INT0 interrupt
	EA = 1;         //' ENABLE INTERRUPT.
    ES = 1;         //' ENABLE SERIAL INTERRUPT.

	P1 = 0xf8;
	//P2 = 0;

	Bytes_write = 0;
	printf(":RZ00\n\r"); // send hw reset to PC
	printf("=======================\n\r");
	printf("8051-IDE system...\n\r");
	printf("Version 6.0\n\r");
	printf("11 MAy 2006 @19:30\n\r");
	printf("ktkoson@kmitl.ac.th\n\r");
	printf("all right reserved.\n\r");
	printf("=======================\n\r");
	//printf("\n\r");


	printf("Hard disk diag...\n\r");
	printf("\n\r");

	RESETn = 0;
	delay(100);
	RESETn = 1;
	delay(100);
	wait_ATA_ready();
	 
	ataDriveInit();
	fatInit(0);
    fatGetDirEntry(0,2) ;
    //reportbuffers();	
	rep_all_reg ();
	
	/*


   printf("WRITE_ATA_REG(COMMAND_REG, 0xE1);...\r\n");

   WRITE_ATA_REG(COMMAND_REG, 0xE1);
   wait_ATA_ready(); 	
   
   rep_all_reg ();
*/
   //WRITE_ATA_REG(COMMAND_REG, 0xEC);

   wait_ATA_ready(); 	
   printf("hello\n\r");

    while(1){
		if(Bytes_write)
		{
			//printf("Bytes_write %bd auxdata = [%s]\n\r", Bytes_write,Aux_Data);
			if(Aux_Data[0] != ':')
			{
				printf(":err\n\r");
				Bytes_write = 0;
				Aux_Data[0] = 0;
				Aux_Data[1] = 0;
			   	continue;
			} 
			if(Aux_Data[Bytes_write-1] == ':')
			{ 
			 	// printf("Start of code\n");
			   	continue;
			}
			if(Aux_Data[Bytes_write-1] == 0x0d) 
			{
			 	printf("Received %s\n\r",Aux_Data);
				Bytes_write=0;
				switch(Aux_Data[1])
				{
					case  'w' :
					switch (Aux_Data[2])
					{
						case 'b': 
						 	printf("Write Buffer\n\r");
							break;
						case 'r':
						 	printf("Write Register\n\r");
						    s[0] = Aux_Data[3];
						    s[1] = Aux_Data[4];
							reg = strtol (s, &p, 16);
						    s[0] = Aux_Data[5];
						    s[1] = Aux_Data[6];
							dat = strtol (s, &p, 16);
							AtaWriteRegister(reg, dat);
							break;	 
						case 's':
						 	printf("Write Sector\n");

							break;	 
						case 'c':
						    s[0] = Aux_Data[3];
						    s[1] = Aux_Data[4];
							reg = strtol (s, &p, 16);	    
							AtaWriteRegister(COMMAND_REG, reg);
							break;	 

						default:
							printf("Command not implemented @w[2]\n\r");
						break;
					}
					break;

					case 'r':
					switch(Aux_Data[2])
					{
					 	case 'b': 
					 		printf("Read Buffer\n");
							ReadHDDBuffer();							
							break;				
					 	case 'p': 
					 		printf("Report HDD Buffer\n");
							reportbuffers();							
							break;				
						case 'r' :
							rep_all_reg ();
							break;				
						case 's' :
						 	printf("Read Sector\n");
							break;	
						case 'e' :
						 	printf(":RER%bX\n", AtaReadRegister(ERROR_REG));
							break;	
						case 't' :
						 	printf(":RTR%bX\n", AtaReadRegister(PRISTATUS_REG));
							break;	
										
						default:
							printf("Command not implemented @r[2]\n");
						break;
					} 
					break;
					default:
					printf("Command not implemented @[1]\n");
					break;
				}
			}
		}	
	}
}

unsigned char ataReadSectorsCHS( unsigned char Drive, 
								 unsigned char Head, 
								 unsigned int Track,
								 unsigned char Sector,
								 unsigned int numsectors,
								 unsigned char *Buffer)
{
  	unsigned char temp;

	// Wait for drive to be ready
	temp = ataStatusWait(ATA_SR_BSY, ATA_SR_BSY);
/*	printf(":RSC%02bX\n", READ_ATA_REG(SECTOR_COUNT));
	printf(":RSN%02bX\n", READ_ATA_REG(SECTOR_NUMBER));
	printf(":RCL%02bX\n", READ_ATA_REG(CYLINDER_LOW));
	printf(":RCH%02bX\n", READ_ATA_REG(CYLINDER_HI));
	printf(":RSD%02bX\n", READ_ATA_REG(DRIVE_HEAD));
	printf(":RTR%02bX\n", READ_ATA_REG(PRISTATUS_REG));
	printf(":RER%02bX\n", READ_ATA_REG(ERROR_REG));
*/
  	// Prepare parameters...
	ataWriteByte(DRIVE_HEAD, 0xA0+(Drive ? 0x10:00)+Head); // CHS mode/Drive/Head
	ataWriteByte(CYLINDER_HI, Track>>8);  		// MSB of track
	ataWriteByte(CYLINDER_LOW, Track);     		// LSB of track
  	ataWriteByte(SECTOR_NUMBER, Sector);    	// sector
	ataWriteByte(SECTOR_COUNT, numsectors);	// # of sectors

	// Issue read sector command...
  	ataWriteByte(COMMAND_REG, 0x21);

	// Wait for drive to be ready
	temp = ataStatusWait(ATA_SR_BSY, ATA_SR_BSY);

	if (temp & ATA_SR_ERR)
	{
		printf("RD ERR\r\n");
		return 1;
	}

	// Wait for drive to request data transfer
	ataStatusWait(ATA_SR_DRQ, 0);

	// read data from drive
	// ataReadDataBuffer(Buffer, 512*numsectors);
	ataReadDataBuffer(Buffer, numsectors);

	// Return the error bit from the status register...
	temp = AtaReadRegister(COMMAND_REG);	// read status register

	return (temp & ATA_SR_ERR) ? 1:0;
}

unsigned char ataReadSectors(	unsigned char Drive, 
										unsigned long lba,
										unsigned int numsectors,
                            	unsigned char *Buffer)
{
  	unsigned int cyl, head, sect;
  	unsigned char temp;
	// check if drive supports native LBA mode
	if(ataDriveInfo.LBAsupport)
	{
		// drive supports using native LBA
		temp = ataReadSectorsLBA(Drive, lba, numsectors, Buffer);
		#ifdef DEBUG_ATA
			//printf("Info : drive supports using native LBA\n");
			printf("Drive support LBA\tDrive : ");	rprintfu08(Drive);	 
			printf(" LBA : ");	rprintfu32(lba);	 
			printf(" NumSectors	: ");	rprintfu16(numsectors);		rprintfCRLF();
		#endif
	}
	else
	{
		// drive required CHS access
		//printf("Info : drive required CHS access\n");

		//#ifdef DEBUG_ATA
			// do this defore destroying lba
		//printf("ATA LBA for CHS read: ");
		//	printf("LBA="); rprintfu32(lba); printf (" ");
		//#endif

		// convert LBA to pseudo CHS
		// remember to offset the sector count by one
		sect = (u08) (lba % ataDriveInfo.sectors)+1;
		lba = lba / ataDriveInfo.sectors;
		head = (u08) (lba % ataDriveInfo.heads);
		lba = lba / ataDriveInfo.heads;
		cyl = (u16) lba;

		#ifdef DEBUG_ATA
			printf("C:H:S=");
			rprintfu16(cyl); printf(":");
			rprintfu08(head); printf(":");
			rprintfu08(sect); rprintfCRLF();
		#endif

		temp = ataReadSectorsCHS( Drive, head, cyl, sect, numsectors, Buffer );
	}

    reportbuffers();
	if(temp)
		ataDiskErr();
	return temp;
}

unsigned char ataReadSectorsLBA(	unsigned char Drive, 
											unsigned long lba,
											unsigned int numsectors,
                            		unsigned char *Buffer)
{
  	unsigned  int idata cyl, head, sect;
  	unsigned char idata temp;

#ifdef DEBUG_ATA
	printf("ATA LBA read ");
	rprintfu32(lba); printf(" ");
	rprintfu16(numsectors); printf(" ");
	rprintfu16((unsigned int)Buffer); 
	rprintfCRLF();
#endif

	sect = (int) ( lba & 0x000000ffL );
	lba = lba >> 8;
	cyl = (int) ( lba & 0x0000ffff );
	lba = lba >> 16;
	head = ( (int) ( lba & 0x0fL ) ) | ATA_HEAD_USE_LBA;

	temp = ataReadSectorsCHS( Drive, head, cyl, sect, numsectors, Buffer );

	if(temp)
		ataDiskErr();
	return temp;
}

void ataReadDataBuffer(u08 *Buffer, u16 numSector)
{
	unsigned int idata i;

	//sbi(MCUCR, SRW);			// enable RAM waitstate
    Buffer = &sectordata;
	// read data from drive
	for (i=0; i<numSector; i++)
	{
		ReadHDDBuffer();
	}
	//cbi(MCUCR, SRW);			// disable RAM waitstate
	
}

void ataDiskErr(void)
{
	unsigned char idata  b;

	b = AtaReadRegister(ERROR_REG);	
	printf("ATA Error: "); 
	rprintfu08(b); 
	rprintfCRLF();
}


unsigned char ataReadByte(unsigned char  reg)
{
	return AtaReadRegister(reg);
}

void ataWriteByte(unsigned char  reg, unsigned char  dat)
{
	 AtaWriteRegister(reg,dat);
}			  

unsigned char  ataStatusWait(unsigned char  mask, unsigned char waitStatus)
{
	register unsigned char  status;

	delay(100);

	// wait for desired status
	while( ((status = ataReadByte(COMMAND_REG)) & mask) == waitStatus );

	return status;
}

void ataDriveInit(void)
{
	unsigned char  i;
	unsigned char* buffer = &sectordata; //(0x1E00)

	// read drive identity
	printf("\r\nScanning IDE interface...\r\n");
	// Wait for drive to be ready
	ataStatusWait(ATA_SR_BSY, ATA_SR_BSY);
	// issue identify command
	ataWriteByte(COMMAND_REG, 0xEC);
	// wait for drive to request data transfer
	ataStatusWait(ATA_SR_DRQ, ATA_SR_DRQ);
	delay(200);
	// read in the data
	ReadHDDBuffer();
	//ataReadDataBuffer(buffer, 512);

	// set local drive info parameters
	ataDriveInfo.cylinders =	(unsigned int)(sectordata[ATA_IDENT_CYLINDERS*2+1]*256 + sectordata[ATA_IDENT_CYLINDERS*2]);
	ataDriveInfo.heads =		 sectordata[ATA_IDENT_HEADS*2];
	ataDriveInfo.sectors =		 sectordata[ATA_IDENT_SECTORS*2];
	ataDriveInfo.LBAsupport =	 sectordata[ATA_IDENT_FIELDVALID*2];
	ataDriveInfo.sizeinsectors = (unsigned long) sectordata[123];
	ataDriveInfo.sizeinsectors = (unsigned long) ataDriveInfo.sizeinsectors *256 + sectordata[122];
	ataDriveInfo.sizeinsectors = (unsigned long) ataDriveInfo.sizeinsectors *256 + sectordata[121];
	ataDriveInfo.sizeinsectors = (unsigned long) ataDriveInfo.sizeinsectors *256 + sectordata[120];

    
	printf("ataDriveInfo.cylinders  = %d [%XH]\n\r",ataDriveInfo.cylinders ,ataDriveInfo.cylinders   );
	printf("ataDriveInfo.heads = %bd [%bX]\n\r",ataDriveInfo.heads ,ataDriveInfo.heads );
	printf("ataDriveInfo.sectors  = %bd [%bX]\n\r",ataDriveInfo.sectors ,ataDriveInfo.sectors );
	printf("ataDriveInfo.LBAsupport  = %bd [%bX]\n\r",ataDriveInfo.LBAsupport ,ataDriveInfo.LBAsupport);
 	printf("ataDriveInfo.sizeinsectors  = %ld [%lX]\n\r",ataDriveInfo.sizeinsectors ,ataDriveInfo.sizeinsectors);


	// copy model string

	for(i=0; i<40; i+=2)
	{
		// correct for byte order
		ataDriveInfo.model[i  ] = buffer[(ATA_IDENT_MODEL*2) + i + 1];
		ataDriveInfo.model[i+1] = buffer[(ATA_IDENT_MODEL*2) + i    ];
	}
	// terminate string
	ataDriveInfo.model[40] = 0;

	// process and print info
	if(ataDriveInfo.LBAsupport)
	{
		// LBA support
		printf("Drive 0: %ldMB ",  ataDriveInfo.sizeinsectors/(1000000/512) );
		//printf("Drive 0: %d Sectors ", ataDriveInfo.sizeinsectors/512 );
		printf("LBA mode -- MODEL: ");
	}
	else
	{
		// CHS, no LBA support
		// calculate drive size
		ataDriveInfo.sizeinsectors = (unsigned long) ataDriveInfo.cylinders*
												ataDriveInfo.heads*ataDriveInfo.sectors;
		printf("Drive 0: %dMB ", ataDriveInfo.sizeinsectors/(1000000/512) );
		printf("CHS mode C=%d H=%d S=%d -- MODEL: ", ataDriveInfo.cylinders, ataDriveInfo.heads, ataDriveInfo.sectors );
	}
	// print model information	
	rprintfStr(ataDriveInfo.model); printf("\n\r");

	//reportbuffers();
	// initialize local disk parameters
	//ataDriveInfo.cylinders = ATA_DISKPARM_CLYS;
	//ataDriveInfo.heads = ATA_DISKPARM_HEADS;
	//ataDriveInfo.sectors = ATA_DISKPARM_SECTORS;

}

void reportbuffers()

{
	int s,t;
	unsigned char a,b;
	printf("ADDR ");
	for(t=0;t <= 15; t++ )
	{
		printf("%02bX ",(unsigned char ) t);
	}	  	
	printf("\r\n");
	for(s=0;s <= 31; s++ )
	{
		printf("%04X ",s*16);
		for(t=0; t < 8; t++ )
		{
			printf("%02bX %02bX ",sectordata[(8*s+t)*2], sectordata[(8*s+t)*2+1]);
		}	  	
		printf("\t");
		for(t=0;t < 8; t++ )
		{
		    a = sectordata[(8*s+t)*2+1] ;		    
			if((a > 'z') || (a < ' ')) a = '.';
			b = sectordata[(8*s+t)*2];
		    if((b > 'z') || (b < ' ')) b = '.';
			printf("%c%c", b, a);
		}	  	
		printf("\n\r");
		delay(10);
	}
}


void ReadHDDBuffer(void)
{
	int s;
	//unsigned char a,b;
	while( (READ_ATA_REG(ALTSTATUS_REG) & 0x80)) ;
	READ_ATA_REG(PRISTATUS_REG);
	for(s=0;s <= 256; s++ )
	{
		while(!(READ_ATA_REG(PRISTATUS_REG)&& 0x80)) ;
		P0 = 0xff;
		P1 = DATA_REG;
		RDn = 0;
		sectordata[s*2] = P0;
		sectordata[(s*2)+1] = P2;
		RDn = 1;   		
	}
	//reportbuffers();	

}


void int0_isr (void) interrupt 0
{
//	sectordata[word_counter++] = AtaReadRegister( DATA_REG );
//	sectordata[word_counter++] = P2;
}

/*===================================================================
   ATA support function
=====================================================================*/
unsigned char AtaReadRegister(unsigned char p_ucRegister)
{
	unsigned char ata_reg;
	ata_reg = READ_ATA_REG((unsigned char)p_ucRegister);
	return ata_reg; 
}

void AtaWriteRegister(unsigned char p_ucRegister, unsigned char p_ucData)
{
  	while( (READ_ATA_REG(PRISTATUS_REG) & 0x80)) ;
	P0 = p_ucData ;
	P1 = p_ucRegister;
	WRn = 0;
	delay(50);
	WRn = 1;

}
void wait_ATA_ready()
{
	while( (READ_ATA_REG(PRISTATUS_REG) & 0x80)) ;	  // wait for not busy
	while(!(READ_ATA_REG(PRISTATUS_REG) & 0x40));	  // wait for ready
}

void rep_all_reg ()
{
	printf("\n\r");
	printf(":RSC%02bX\n\r", READ_ATA_REG(SECTOR_COUNT));
	printf(":RSN%02bX\n\r", READ_ATA_REG(SECTOR_NUMBER));
	printf(":RCL%02bX\n\r", READ_ATA_REG(CYLINDER_LOW));
	printf(":RCH%02bX\n\r", READ_ATA_REG(CYLINDER_HI));
	printf(":RSD%02bX\n\r", READ_ATA_REG(DRIVE_HEAD));
	printf(":RTR%02bX\n\r", READ_ATA_REG(PRISTATUS_REG));
	printf(":RER%02bX\n\r", READ_ATA_REG(ERROR_REG));
}

unsigned char READ_ATA_REG (unsigned char REG )
{
	unsigned char return_val;
	P0 = 0xff;
	P1 = REG;
	RDn = 0;
	return_val = P0;
	RDn = 1;
	return return_val;
}		   

/*=======================================
    utility function

=========================================*/

/*
function	: void delay(unsigned int dd)
return 		: none
parameter 	: int dd - delay counter (1 to 65535)
*/
void delay(unsigned int dd)
{
	while(dd--) ;
}


/*=======================================
 Serial port support function
=========================================*/
void Init_serial() 
{
   //' SET TH1 AND PCON AT FEQUENCY. = 18.432 MHz
   PCON |= 0x80; 
   TH1 = 246;
   SCON = 0x50; 
   TMOD = 0x20; 
   TR1= 1;     //' TIMER 1 RUN.
   TI   = 0;   //' SET TI SEND FIRST CHAR OF UART.
}

/*"(---------------------------------------------------------------------)"*/
/*"(-                      APPLICATIONS SEND DATA                       -)"*/
/*"(---------------------------------------------------------------------)"*/
char putchar(unsigned char c) 
{
	SBUF = c; 
	while(!TI);
	TI = 0;      //' CLEAR BIT FLAG TI_0
	return (c);  //' MOV DATA TO SERIAL 
}

/*"(---------------------------------------------------------------------)"*/
/*"(-                    APPLICATIONS RECEIVE DATA                      -)"*/
/*"(---------------------------------------------------------------------)"*/
void RxdReceive () interrupt 4 
{
	unsigned char c;
	if (RI == 1) 
	{ 
		c = SBUF;
		Aux_Data[Bytes_write++] = c; //   
		Aux_Data[Bytes_write+1] = '\0'; //   
		RI = 0;
	}
	return;
}

