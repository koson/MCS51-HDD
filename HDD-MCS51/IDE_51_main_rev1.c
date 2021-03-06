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



unsigned int  DataSpeed;
unsigned char DataBits;
unsigned char Parity;
unsigned char StopBits;
unsigned char SerialSet;

unsigned char g_AtaDevConfig[ATA_DEVICE_COUNT_MAX];


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
void rep_all_reg ();
void ReportErrorStatus();
void wait_ATA_ready();
int AtaQueryDevice(void);
int AtaResetAdapter(void);
unsigned char AtaReadRegister(unsigned char p_ucRegister);
void AtaWriteRegister(unsigned char p_ucRegister, unsigned char p_ucData);
char AtaSelectDevice(unsigned char p_ucDevice);
unsigned char word_counter = 0;





main()
{
	unsigned int i = 0;
	
   	AUXR = 0x0C;    //' T89C51AC2 CONFIG SELECT IRAM = 1024 BYTE.
   	Init_serial(); 	//' SERIAL SETTING.
	//IT0 = 1;  		//' Falling edge interrupt 0
	//EX0 = 1;		//' Enable INT0 interrupt
	EA = 1;         //' ENABLE INTERRUPT.
    ES = 1;         //' ENABLE SERIAL INTERRUPT.

	P1 = 0xf8;
	P2 = 0;

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
	printf("Harddisk ready...\r\n");

	rep_all_reg ();
	
	//printf("AtaQueryDevice() = %d... \r\n ",  AtaQueryDevice());

 
   // AtaWriteRegister(regATA_DEVCMD, cmdATA_STANDBY_IMMEDIATE1 );

	//rep_all_reg ();	 
	//AtaSelectDevice(0);
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
			 	//printf("%s\n",Aux_Data);
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
							break;	 
						case 's':
						 	printf("Write Sector\n");
							break;	 
						default:
							printf("Command not implemented\n");
						break;
					}
					break;

					case 'r':
					switch(Aux_Data[2])
					{
					 	case 'b': 
					 		printf("Read Buffer\n");
							break;				
						case 'r' :
							rep_all_reg ();
							break;				
						case 's' :
						 	printf("Read Sector\n");
							break;	
						case 'e' :
						 	printf(":RER%c\n", AtaReadRegister(regATA_ERROR));
							break;	
						case 't' :
						 	printf(":RTR%c\n", AtaReadRegister(regATA_PRISTAT));
							break;	
										
						default:
							printf("Command not implemented\n");
						break;
					} 
					break;
					default:
					printf("Command not implemented\n");
					break;
				}
			}
		}	
	}
}



void int0_isr (void) interrupt 0
{
	sectordata[word_counter++] = AtaReadRegister( regATA_DATA );
	sectordata[word_counter++] = P2;
}

/*********************
 * AtaQueryDevice
 *	Checks the host adapter and determines the number and type of drives attached.
 *	
 *	This function fills the global variable g_AtaDevConfig.
 *	For values see ata.h, the DEVCONFIG values.
 *
 *	The number of drives found is returned as the function result
 */
int AtaQueryDevice(void)
{
  int           iNumDev = 0;
  unsigned char ucSecCnt;
  unsigned char ucSecNr;
  unsigned char ucCylLow;
  unsigned char ucCylHigh;
  unsigned char ucStatus;
  int           iATA;

  iNumDev = 0;

  // Query the host adapter to determine the drives attached
  for (iATA = 0; iATA < ATA_DEVICE_COUNT_MAX; iATA++)
  {
    // Assume we have no device attached at all
    g_AtaDevConfig[iATA] = ATA_DEVCONFIG_NONE;
    
    // Issue a reset command for this (possible) device
    AtaResetAdapter();

    // Check the device again by reading the signature in the sectorcount/number registers.
    AtaWriteRegister(regATA_DEVHEAD, iATA == 0 ? regATA_DEVHEAD_DEV0 : regATA_DEVHEAD_DEV1);
    ucSecCnt = AtaReadRegister( regATA_SECCNT );
    ucSecNr  = AtaReadRegister( regATA_SECNR );
    
    if ((ucSecCnt == 0x01) && (ucSecNr == 0x01))
    {
      // At least something is out there, but still unknown...
      g_AtaDevConfig[iATA] = ATA_DEVCONFIG_UNKNOWN;
      
      // To determine ATA or ATAPI, read the signature in the cylinder registers
      ucCylLow  = AtaReadRegister( regATA_CYLLSB );
      ucCylHigh = AtaReadRegister( regATA_CYLMSB );
      ucStatus  = AtaReadRegister( regATA_PRISTAT );
      
      // Signature is ATAPI ???
      if ((ucCylLow == 0x14) && (ucCylHigh == 0xeb))
      {
        g_AtaDevConfig[iATA] = ATA_DEVCONFIG_ATAPI;
        printf("ATAPI\r\n");
        iNumDev++;
      }
      else
      {
        // Signature is ATA ???
        if ( ( ucCylLow == 0x00 ) && ( ucCylHigh == 0x00 ) && ( ucStatus != 0x00 ) )
        {
          g_AtaDevConfig[iATA] = ATA_DEVCONFIG_ATA;
          printf("ATA\r\n");
          iNumDev++;
        }
      }
    }
  }
  return iNumDev;
}

/*********************
 * AtaResetAdapter
 *	Resets the device(s) attached to the host adapter and automatically
 *	selects device 0.
 */

int AtaResetAdapter(void)
{
  unsigned char ucStatus;
  unsigned char ucDevCtrl;

  // setup register values
  ucDevCtrl = regATA_DEVCTRL_HD15 | regATA_DEVCTRL_NIEN;

  // Set and then reset the soft reset bit in the Device Control
  // register.  This causes device 0 be selected.

  AtaWriteRegister( regATA_DEVCTRL, ucDevCtrl | regATA_DEVCTRL_SRST);
  AtaWriteRegister( regATA_DEVCTRL, ucDevCtrl );
  
  while (1)
  {
    ucStatus = AtaReadRegister(regATA_PRISTAT);
    if ((ucStatus & (regATA_PRISTAT_BSY | regATA_PRISTAT_DRQ)) == 0)
      break;
  }
  AtaWriteRegister(regATA_DEVHEAD, regATA_DEVHEAD_DEV0);
  while (1)
  {
    ucStatus = AtaReadRegister(regATA_PRISTAT);
    if ((ucStatus & (regATA_PRISTAT_BSY | regATA_PRISTAT_DRQ)) == 0)
      break;
  }
  
  return 0;
}



unsigned char AtaReadRegister(unsigned char p_ucRegister)
{
  unsigned char ata_reg;
  ata_reg = READ_ATA_REG((unsigned char)p_ucRegister);
  //printf("READ_ATA_REG (%bx) = %bx \r\n",p_ucRegister, ata_reg);
  return ata_reg; 
}



/*********************
 * AtaWriteRegister
 *	Wrapper (should become obsolete) to call the assembly function to write an 8-bit register
 *	given by the register argument and the 8-bit data to write.
 *
 */
void AtaWriteRegister(unsigned char p_ucRegister, unsigned char p_ucData)
{
 
  	WRITE_ATA_REG((unsigned char)p_ucRegister, p_ucData);
	//printf("WRITE_ATA_REG(%bx, %bx) \r\n",p_ucRegister, p_ucData);

}

/*
char AtaSelectDevice(unsigned char p_ucDevice)
{
  unsigned char ucStatus;

   while ( 1 ) 
   { 
      ucStatus = AtaReadRegister( regATA_PRISTAT ); 
      printf("ATA-HOST_BSY0_DRQ0 = %bx\r\n", ucStatus);  
      if ( ( ucStatus & (regATA_PRISTAT_ERR) ) != 0 ) 
         break; 
       if ( ( ucStatus & (regATA_PRISTAT_BSY  | regATA_PRISTAT_DRQ ) ) == 0 )  
         break; 
   } 


  // Here we select the drive we really want to work with by
  // putting 0xA0 or 0xB0 in the Drive/Head register.

  AtaWriteRegister(regATA_DEVHEAD, p_ucDevice ? regATA_DEVHEAD_DEV1 : regATA_DEVHEAD_DEV0 );

  ucStatus = AtaReadRegister( regATA_ALTSTAT );

  // we always return 0. All errors are ignored.
  return 0;
}

*/


void wait_ATA_ready()
{
	//printf("Wait for ata ready...\r\n");
	while( (READ_ATA_REG(STATUS_REG) & 0x80)) ;	  // wait for not busy
	while(!(READ_ATA_REG(STATUS_REG) & 0x40));	  // wait for ready

}


void rep_all_reg ()
{
	printf("\n");
	printf(":RSC%c\n", READ_ATA_REG(SECTOR_COUNT));
	printf(":RSN%c\n", READ_ATA_REG(SECTOR_NUMBER));
	printf(":RCL%c\n", READ_ATA_REG(CYLINDER_LOW));
	printf(":RCH%c\n", READ_ATA_REG(CYLINDER_HI));
	printf(":RSD%c\n", READ_ATA_REG(DRIVE_HEAD));
	printf(":RTR%c\n", READ_ATA_REG(STATUS_REG));
	printf(":RER%c\n", READ_ATA_REG(ERROR_REG));
}
/*
void ReportDiskStatus()
{
	unsigned char diskstatus;
	diskstatus = READ_ATA_REG(STATUS_REG);
	printf("--------------------------------\n\r");
	printf("status register value  = %bX\n\r",diskstatus);
	printf("BUSY DRDY  DWF  DSC  DRQ  CORR INDEX ERROR \n\r");
	printf("  %bX    %bX    ",diskstatus >> 7 &1  ,diskstatus >> 6 &1);
	printf("%bX    %bX    "  ,diskstatus >> 5 &1,diskstatus >> 4 &1);
	printf("%bX     %bX    " ,diskstatus >> 3 &1,diskstatus >> 2 &1);
	printf("%bX     %bX\n\r" ,diskstatus >> 1 &1,diskstatus &1);
	delay (100);

}

void ReportErrorStatus()
{
	unsigned char diskerrorcode;
	diskerrorcode = READ_ATA_REG(ERROR_REG);

	printf("----------------------------------\n\r");
	printf("Error register value  = %bX\n\r",diskerrorcode);
	printf("BBK  UNC   ---- INDF ---  ABRT TK0NF AMNF\n\r");
	printf("  %bX    %bX    ",diskerrorcode >> 7 &1  ,diskerrorcode >> 6 &1);
	printf("%bX    %bX    "  ,diskerrorcode >> 5 &1   ,diskerrorcode >> 4 &1);
	printf("%bX     %bX    " ,diskerrorcode >> 3 &1  ,diskerrorcode >> 2 &1);
	printf("%bX     %bX\n\r" ,diskerrorcode >> 1 &1 ,diskerrorcode  &1);
}

*/
unsigned char READ_ATA_REG (unsigned char REG )
{
	unsigned char return_val;
	P1 = REG;
	RDn = 0;
	return_val = P0;
	RDn = 1;
	return return_val;
}		   
/*
unsigned char READ_ATA_REG_HI (unsigned char REG )
{
	unsigned char return_val;
	P1 = REG;
	RDn = 0;
	return_val = P2;
	RDn = 1;
	return return_val;
}		   
*/
void WRITE_ATA_REG(unsigned char breg, unsigned char value)
{
  	while( (READ_ATA_REG(STATUS_REG) & 0x80)) ;
	P0 = value;
	delay(50);
	P1 = breg;
	delay(50);
	WRn = 0;
	delay(50);
	WRn = 1;
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

