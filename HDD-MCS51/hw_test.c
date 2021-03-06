#include <REG51AC2.h>
#include <stdio.h>

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

#define ATA_ERR_AMNF		0x01
#define ATA_ERR_TK0NF		0x02 >> 1
#define ATA_ERR_ABRT		0x04 >> 2
#define ATA_ERR_MCR			0x08 >> 3
#define ATA_ERR_IDNF		0x10 >> 4
#define ATA_ERR_MC			0x20 >> 5
#define ATA_ERR_UNC			0x40 >> 6
#define ATA_ERR_BBK			0x80 >> 7

#define DATA_PORT			0xF0
#define ERROR_REG			0xF1
#define SECTOR_COUNT		0xF2
#define SECTOR_NUMBER		0xF3
#define CYLINDER_LOW		0xF4
#define CYLINDER_HI			0xF5
#define DRIVE_HEAD			0xF6
#define STATUS_REG			0xF7
#define COMMAND_REG			0xF7

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

//#define mskATAPI_ER_SNSKEY 	0xf0   // ATAPI sense key (mask)
//#define mskATAPI_ER_MCR    	0x08   // ATAPI Media Change Request
//#define mskATAPI_ER_ABRT   	0x04   // ATAPI command abort
//#define mskATAPI_ER_EOM    	0x02   // ATAPI End of Media
//#define mskATAPI_ER_ILI    	0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (regATA_SECCNT)

#define regATA_SECCNT_P_TAG	0xf8   // ATAPI tag (mask)
#define regATA_SECCNT_P_REL	0x04   // ATAPI release
#define regATA_SECCNT_P_IO	0x02   // ATAPI I/O
#define regATA_SECCNT_P_CD	0x01   // ATAPI C/D

// bits 7-4 of the device/head (regATA_DEVHEAD) reg

#define regATA_DEVHEAD_DEV0	0xe0  // 0xa0  // select device 0
#define regATA_DEVHEAD_DEV1	0xf0 //0xb0  // select device 1

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
unsigned int  DataSpeed;
unsigned char DataBits;
unsigned char Parity;
unsigned char StopBits;
unsigned char SerialSet;

void Init_serial();
unsigned int Bytes_write; // len data_ethernet
unsigned char Aux_Data[16];           //' TCP AUX DATA = INTERGER < PACKET - 55 BYTE AND CHECKRAM

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


main()
{
	unsigned char testbyte;
	unsigned int delay_time = 0;

	
   	AUXR = 0x0C;             //' T89C51AC2 CONFIG SELECT IRAM = 1024 BYTE.
   	Init_serial(); //' SERIAL SETTING.
	EA = 1;                  //' ENABLE INTERRUPT.
    ES = 1;                  //' ENABLE SERIAL INTERRUPT.
	
	P1 = 0xf8;
	P2 = 0;

	Bytes_write = 0;
	printf("=======================\n\r");
	printf("8051-IDE system...\n\r");
	printf("Version 1.0 \n\r");
	printf("14 April 2006 @19:30\n\r");
	printf("ktkoson@kmitl.ac.th\n\r");
	printf("all right reserved.\n\r");
	printf("=======================\n\r");
	printf("\n\r");

    while(1){
		if(Bytes_write)
		{
			Bytes_write = 0;
			switch (Aux_Data[0])
			{
				case '7': 
			   		testbyte = 0x80;
					printf("RESET\r\n");
					break;
				case '6': 
			   		testbyte = 0x40;
					printf("DIOR\r\n");
					break;
				case '5': 
			   		testbyte = 0x20;
					printf("DIOW\r\n");
					break;
				case '4': 
			   		testbyte = 0x10;
					printf("CS1\r\n");
					break;
				case '3': 
			   		testbyte = 0x08;
					printf("CS0\r\n");
					break;
				case '2': 
			   		testbyte = 0x04;
					printf("DA2\r\n");
					break;
				case '1': 
			   		testbyte = 0x02;
					printf("DA1\r\n");
					break;
				case '0': 
			   		testbyte = 0x01;
					printf("DA0\r\n");
					break;
				case 'u':
					printf("%d\r\n",delay_time++);
					break;
				case 'd': 
					printf("%d\r\n",delay_time--);
					break;
			}
		}
		P1 =  testbyte;
		delay(delay_time);
		P1 =  0;
		delay(delay_time);

	}
}


void delay(unsigned int dd)
{
	while(dd--) ;
}

/*"(---------------------------------------------------------------------)"*/
/*"(-           DEFINE INTERNAL DATA OF SERIAL COMMUNICATION            -)"*/
/*"(---------------------------------------------------------------------)"*/
/*"(---------------------------------------------------------------------)"*/
/*"(-                      INITIAL FUNCTION SERIAL                      -)"*/
/*"(---------------------------------------------------------------------)"*/
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

