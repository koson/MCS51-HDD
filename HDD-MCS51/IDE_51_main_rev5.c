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

#define N 'N'
#define E 'E'
#define O 'O'
#define n 'N'
#define e 'E'
#define o 'O'

#define ATA_DEVICE_COUNT_MAX  2

#define ATA_ST_ERR			0x01
#define ATA_ST_INDEX		0x02 >> 1 
#define ATA_ST_CORR			0x04 >> 2
#define ATA_ST_DRQ			0x08 >> 3
#define ATA_ST_DSC			0x10 >> 4
#define ATA_ST_DWF			0x20 >> 5
#define ATA_ST_RDY			0x40 >> 6
#define ATA_ST_BUSY			0x80 >> 7

#define ATA_SR_BSY		0x80
#define ATA_SR_DRDY		0x40
#define ATA_SR_DF		0x20
#define ATA_SR_DSC		0x10
#define ATA_SR_DRQ		0x08
#define ATA_SR_CORR		0x04
#define ATA_SR_IDX		0x02
#define ATA_SR_ERR		0x01


#define ATA_ERR_AMNF		0x01
#define ATA_ERR_TK0NF		0x02 >> 1
#define ATA_ERR_ABRT		0x04 >> 2
#define ATA_ERR_MCR			0x08 >> 3
#define ATA_ERR_IDNF		0x10 >> 4
#define ATA_ERR_MC			0x20 >> 5
#define ATA_ERR_UNC			0x40 >> 6
#define ATA_ERR_BBK			0x80 >> 7


#define DATA_REG			0xF0
#define ERROR_REG			0xF1
#define SECTOR_COUNT		0xF2
#define SECTOR_NUMBER		0xF3
#define CYLINDER_LOW		0xF4
#define CYLINDER_HI			0xF5
#define DRIVE_HEAD			0xF6
#define PRISTATUS_REG		0xF7
#define COMMAND_REG			0xF7
#define ALTSTATUS_REG		0xEE



/*
#define regATA_DATALSB		0x60	// data reg         in/out
#define regATA_DATAMSB		0x43	// data reg         in/out
#define regATA_DATA			0xF0	// data reg         in/out
#define regATA_ERROR		0xF1	// error            in    
#define regATA_FEATURE		0xF1	// feature reg         out
#define regATA_SECCNT		0xF2	// sector count     in/out
#define regATA_SECNR		0xF3	// sector number    in/out
#define regATA_CYLLSB		0xF4	// cylinder low     in/out
#define regATA_CYLMSB		0xF5	// cylinder high    in/out
#define regATA_DEVHEAD		0xF6	// device head      in/out
#define regATA_PRISTAT		0xF7	// primary status   in    
#define regATA_DEVCMD   	0xF7	// command             out
#define regATA_ALTSTAT		0xEE	// alternate status in    
#define regATA_DEVCTRL		0xEE	// device control      out
#define regATA_DEVADDR    	0xEF   	// device address   in     
*/
// ATA error register (regATA_ERROR) bitmasks (msk)

#define mskATA_ER_ICRC		0x80    // ATA Ultra DMA bad CRC
#define mskATA_ER_BBK  		0x80    // ATA bad block
#define mskATA_ER_UNC  		0x40    // ATA uncorrected error
#define mskATA_ER_MC   		0x20    // ATA media change
#define mskATA_ER_IDNF 		0x10    // ATA id not found
#define mskATA_ER_MCR  		0x08    // ATA media change request
#define mskATA_ER_ABRT 		0x04    // ATA command aborted
#define mskATA_ER_NTK0 		0x02	// ATA track 0 not found
#define mskATA_ER_NDAM 		0x01    // ATA address mark not found


// ATAPI Interrupt Reason bits in the Sector Count reg (regATA_SECCNT)

#define regATA_SECCNT_P_TAG	0xf8   // ATAPI tag (mask)
#define regATA_SECCNT_P_REL	0x04   // ATAPI release
#define regATA_SECCNT_P_IO	0x02   // ATAPI I/O
#define regATA_SECCNT_P_CD	0x01   // ATAPI C/D

// bits 7-4 of the device/head (regATA_DEVHEAD) reg

#define regATA_DEVHEAD_DEV0	 0x00 // 0xe0   // select device 0
#define regATA_DEVHEAD_DEV1	 0x10 //0xf0  // select device 1

// status reg (regATA_PRISTAT and regATA_ALTSTAT) bits

#define regATA_PRISTAT_BSY  0x80  // busy
#define regATA_PRISTAT_RDY  0x40  // ready
#define regATA_PRISTAT_DF   0x20  // device fault
#define regATA_PRISTAT_WFT  0x20  // write fault (old name)
#define regATA_PRISTAT_SKC  0x10  // seek complete
#define regATA_PRISTAT_SERV 0x10  // service
#define regATA_PRISTAT_DRQ  0x08  // data request
#define regATA_PRISTAT_CORR 0x04  // corrected
#define regATA_PRISTAT_IDX  0x02  // index
#define regATA_PRISTAT_ERR  0x01  // error

// digital output reg (regATA_DEVCTRL) bitmasks

#define regATA_DEVCTRL_HD15   0x00	// 0x08  // bit should always be set to one
#define regATA_DEVCTRL_SRST   0x04  // soft reset
#define regATA_DEVCTRL_NIEN   0x02  // disable interrupts

/**************************************************************/


#define ATA_DEVCONFIG_NONE    0
#define ATA_DEVCONFIG_UNKNOWN 1
#define ATA_DEVCONFIG_ATA     2
#define ATA_DEVCONFIG_ATAPI   3


// Most mandatory and optional ATA commands (from ATA-3),

#define cmdATA_CFA_ERASE_SECTORS            0xC0
#define cmdATA_CFA_REQUEST_EXT_ERR_CODE     0x03
#define cmdATA_CFA_TRANSLATE_SECTOR         0x87
#define cmdATA_CFA_WRITE_MULTIPLE_WO_ERASE  0xCD
#define cmdATA_CFA_WRITE_SECTORS_WO_ERASE   0x38
#define cmdATA_CHECK_POWER_MODE1            0xE5
#define cmdATA_CHECK_POWER_MODE2            0x98
#define cmdATA_DEVICE_RESET                 0x08
#define cmdATA_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#define cmdATA_FLUSH_CACHE                  0xE7
#define cmdATA_FORMAT_TRACK                 0x50
#define cmdATA_ATANTIFY_DEVICE              0xEC
#define cmdATA_ATANTIFY_DEVICE_PACKET       0xA1
#define cmdATA_ATANTIFY_PACKET_DEVICE       0xA1
#define cmdATA_IDLE1                        0xE3
#define cmdATA_IDLE2                        0x97
#define cmdATA_IDLE_IMMEDIATE1              0xE1
#define cmdATA_IDLE_IMMEDIATE2              0x95
#define cmdATA_INITIALIZE_DRIVE_PARAMETERS  0x91
#define cmdATA_INITIALIZE_DEVICE_PARAMETERS 0x91
#define cmdATA_NOP                          0x00
#define cmdATA_PACKET                       0xA0
#define cmdATA_READ_BUFFER                  0xE4
#define cmdATA_READ_DMA                     0xC8
#define cmdATA_READ_DMA_QUEUED              0xC7
#define cmdATA_READ_MULTIPLE                0xC4
#define cmdATA_READ_SECTORS                 0x20
#define cmdATA_READ_VERIFY_SECTORS          0x40
#define cmdATA_RECALIBRATE                  0x10
#define cmdATA_SEEK                         0x70
#define cmdATA_SET_FEATURES                 0xEF
#define cmdATA_SET_MULTIPLE_MODE            0xC6
#define cmdATA_SLEEP1                       0xE6
#define cmdATA_SLEEP2                       0x99
#define cmdATA_STANDBY1                     0xE2
#define cmdATA_STANDBY2                     0x96
#define cmdATA_STANDBY_IMMEDIATE1           0xE0
#define cmdATA_STANDBY_IMMEDIATE2           0x94
#define cmdATA_WRITE_BUFFER                 0xE8
#define cmdATA_WRITE_DMA                    0xCA
#define cmdATA_WRITE_DMA_QUEUED             0xCC
#define cmdATA_WRITE_MULTIPLE               0xC5
#define cmdATA_WRITE_SECTORS                0x30
#define cmdATA_WRITE_VERIFY                 0x3C

#define ATA_IDENT_DEVICETYPE	0		// specifies ATA/ATAPI, removable/non-removable
#define ATA_IDENT_CYLINDERS		1		// number of logical cylinders
#define ATA_IDENT_HEADS			3		// number of logical heads
#define ATA_IDENT_SECTORS		6		// number of sectors per track
#define ATA_IDENT_SERIAL		10		// drive model name (20 characters)
#define ATA_IDENT_MODEL			27		// drive model name (40 characters)
#define ATA_IDENT_FIELDVALID	53		// indicates field validity of higher words (bit0: words54-58, bit1: words 64-70)
#define ATA_IDENT_LBASECTORS	60		// number of sectors in LBA translation mode


unsigned int  DataSpeed;
unsigned char DataBits;
unsigned char Parity;
unsigned char StopBits;
unsigned char SerialSet;

unsigned char g_AtaDevConfig[ATA_DEVICE_COUNT_MAX];
void ataDriveInit(void);

void Init_serial();
unsigned char Bytes_write; // len data_ethernet
unsigned char Aux_Data[32];           //' TCP AUX DATA = INTERGER < PACKET - 55 BYTE AND CHECKRAM
unsigned char sectordata[512];


sbit RESETn = P1^7;
sbit RDn    = P1^6;
sbit WRn    = P1^5;
sbit CS1n   = P1^4;
sbit CS0n   = P1^3;
sbit A2     = P1^2;
sbit A1     = P1^1;
sbit A0     = P1^0;
sbit tstbit = P4^0;

void delay(unsigned int);
unsigned char READ_ATA_REG (unsigned char REG );
unsigned char READ_ATA_REG_HI (unsigned char REG );
void WRITE_ATA_REG(unsigned char breg, unsigned char value);
void ReportDiskStatus();
void ReadHDDBuffer(void);
void rep_all_reg ();
void ReportErrorStatus();
void wait_ATA_ready();
void reportbuffers(void);
//int AtaQueryDevice(void);
int AtaResetAdapter(void);
unsigned char AtaReadRegister(unsigned char p_ucRegister);
void AtaWriteRegister(unsigned char p_ucRegister, unsigned char p_ucData);
char AtaSelectDevice(unsigned char p_ucDevice);
unsigned char word_counter = 0;
unsigned char  buffer[7]; 
void ataWriteByte(unsigned char  reg, unsigned char  dat);


typedef struct 
{
	unsigned int  cylinders;
	unsigned char heads;
	unsigned char sectors;
	unsigned long sizeinsectors;
	unsigned char LBAsupport;
	char model[41];
} typeDriveInfo;


 typeDriveInfo ataDriveInfo;


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
	printf(":RZ00\n"); // send hw reset to PC
	printf("=======================\n\r");
	printf("8051-IDE system...\n\r");
	printf("Version 1.0 \n\r");
	printf("14 April 2006 @19:30\n\r");
	printf("ktkoson@kmitl.ac.th\n\r");
	printf("all right reserved.\n\r");
	printf("=======================\n\r");
	printf("\n\r");


	printf("Hard disk diag...\n\r");
	printf("\n\r");

	RESETn = 0;
	delay(100);
	RESETn = 1;
	delay(100);
	wait_ATA_ready();
	 
	ataDriveInit();
	rep_all_reg ();
	
	/*


   printf("WRITE_ATA_REG(COMMAND_REG, 0xE1);...\r\n");

   WRITE_ATA_REG(COMMAND_REG, 0xE1);
   wait_ATA_ready(); 	
   
   rep_all_reg ();
*/
   //WRITE_ATA_REG(COMMAND_REG, 0xEC);

   wait_ATA_ready(); 	
   printf("hello\n");

    while(1){
		if(Bytes_write)
		{
			//printf("Bytes_write %bd auxdata = [%s]\n\r", Bytes_write,Aux_Data);
			if(Aux_Data[0] != ':')
			{
				printf(":err\n");
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
			 	printf("Received %s\n",Aux_Data);
				Bytes_write=0;
				switch(Aux_Data[1])
				{
					case  'w' :
					switch (Aux_Data[2])
					{
						case 'b': 
						 	printf("Write Buffer\n");
							break;
						case 'r':
						 	printf("Write Register\n");
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
							printf("Command not implemented @w[2]\n");
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


unsigned char  ataReadByte(unsigned char  reg)
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

void rprintfStr(char str[])
{
	// send a string stored in RAM
	// check to make sure we have a good pointer
	if (!str) return;

	// print the string until a null-terminator
	while (*str)
		putchar(*str++);
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


//	printf("ataDriveInfo.cylinders  = %d\n",ataDriveInfo.cylinders ,ataDriveInfo.cylinders   );
//	printf("ataDriveInfo.heads = %bd\n",ataDriveInfo.heads ,ataDriveInfo.heads );
//	printf("ataDriveInfo.sectors  = %bd\n",ataDriveInfo.sectors ,ataDriveInfo.sectors );
//	printf("ataDriveInfo.LBAsupport  = %bd\n",ataDriveInfo.LBAsupport ,ataDriveInfo.LBAsupport);
// 	printf("ataDriveInfo.sizeinsectors  = %ld\n",ataDriveInfo.sizeinsectors ,ataDriveInfo.sizeinsectors);


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
	rprintfStr(ataDriveInfo.model); printf("\n");

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
	printf("\n");
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
		printf("\n");
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
	reportbuffers();	

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
	printf("\n");
	printf(":RSC%02bX\n", READ_ATA_REG(SECTOR_COUNT));
	printf(":RSN%02bX\n", READ_ATA_REG(SECTOR_NUMBER));
	printf(":RCL%02bX\n", READ_ATA_REG(CYLINDER_LOW));
	printf(":RCH%02bX\n", READ_ATA_REG(CYLINDER_HI));
	printf(":RSD%02bX\n", READ_ATA_REG(DRIVE_HEAD));
	printf(":RTR%02bX\n", READ_ATA_REG(PRISTATUS_REG));
	printf(":RER%02bX\n", READ_ATA_REG(ERROR_REG));
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

