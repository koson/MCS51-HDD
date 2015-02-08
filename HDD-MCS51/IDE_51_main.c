#include <REG51AC2.h>
#include <stdio.h>

#define N 'N'
#define E 'E'
#define O 'O'
#define n 'N'
#define e 'E'
#define o 'O'

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

unsigned int  DataSpeed;
unsigned char DataBits;
unsigned char Parity;
unsigned char StopBits;
unsigned char SerialSet;


void Init_serial();
unsigned int Bytes_write; // len data_ethernet
unsigned char Aux_Data[16];           //' TCP AUX DATA = INTERGER < PACKET - 55 BYTE AND CHECKRAM

unsigned char ReadDiskStatus();
unsigned char ReadDiskErrorReg();
void ReportDiskStatus();
void ReportErrorStatus();
void ReportDiskIdentify();
void ReportAllregister();
void checkpowermode(void);




sbit RESETn = P1^7;
sbit RDn    = P1^6;
sbit WRn    = P1^5;
sbit CS1n   = P1^4;
sbit CS0n   = P1^3;
sbit A2     = P1^2;
sbit A1     = P1^1;
sbit A0     = P1^0;
sbit tstbit = P4^0;

void disk_sleep(void);
void disk_wakeup(void);
unsigned char ATA_Read_Register(unsigned char breg);
void ATA_Write_Register(unsigned char breg, unsigned char value);


void delay(unsigned int);

main()
{
	unsigned char dev_head_reg;

	
   	AUXR = 0x0C;             //' T89C51AC2 CONFIG SELECT IRAM = 1024 BYTE.
   	Init_serial(); //' SERIAL SETTING.
	EA = 1;                  //' ENABLE INTERRUPT.
    ES = 1;                  //' ENABLE SERIAL INTERRUPT.
	
	P1 = 0xf8;
	P2 = 0;




/*	delay(200);
	RESETn = 0;
	delay(200);
	RESETn = 1;
	delay(200);
*/	
	
	Bytes_write = 0;

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
 	
// software reset
    WRn = 1;
	P1 = 0xEE;
	P0 = 0xC0;
	delay(100);
    WRn = 1;
	delay(100);

// DRIVE_RELEASE_CMD
    WRn = 1;
	P1 = 0xEE;
	P0 = 0x08;
	delay(100);
    WRn = 1;
	delay(100);

	while( ATA_Read_Register(STATUS_REG)>>7 & 1) ; // wait for busy = 0
//	ReportAllregister(); 
//  acall get_devhead_register
	dev_head_reg = ATA_Read_Register(DRIVE_HEAD);
	printf("dev_head_reg = %bx\r\n",dev_head_reg );
//  clr   ATA_DATA_LS.4             ;set device to 0
	dev_head_reg = dev_head_reg &0xEF; 	
//  acall set_devhead_register
	ATA_Write_Register(DRIVE_HEAD, dev_head_reg);
	printf("dev_head_reg = %bx\r\n",dev_head_reg );

//  acall bsy_wait
	while( ATA_Read_Register(STATUS_REG)>>7 & 1) ; // wait for busy = 0

	printf("ATA_Read_Register(CYLINDER_LOW) = %bx\r\n",ATA_Read_Register(CYLINDER_LOW));
	printf("ATA_Read_Register(CYLINDER_HI) = %bx\r\n",ATA_Read_Register(CYLINDER_HI));
	 

	ATA_Write_Register(COMMAND_REG, 0x90);
	while( ATA_Read_Register(STATUS_REG)>>7 & 1) ; // wait for busy = 0


	ReportDiskStatus();
//	ReportDiskError();

    while(1){
		tstbit = !tstbit;
		delay(10);
		if(Bytes_write){
			if(Aux_Data[0] == 's') 
			{
			 	printf("\r\n execute command ReportDiskStatus() ... \r\n");
				ReportDiskStatus();
			}
			else if (Aux_Data[0] == 'e') 
			{
			 	printf("\r\n execute command ReportErrorStatus() ... \r\n");
				ReportErrorStatus();
			}
			else if (Aux_Data[0] == 'i') 
			{
			 	printf("\r\n execute command ReportDiskIdentify() ... \r\n");
				ReportDiskIdentify();
			}
			else if (Aux_Data[0] == 'w') 
			{
			 	printf("\r\n execute command disk_wakeup() ... \r\n");
				disk_wakeup();
			}
			else if (Aux_Data[0] == 'd') 
			{
			 	printf("\r\n execute command disk_sleep() ... \r\n");
				disk_sleep();
			}
			else if (Aux_Data[0] == 'a') 
			{
			 	printf("\r\n execute command ReportAllregister() ... \r\n");
				ReportAllregister();
			}
			else if (Aux_Data[0] == 'm') 
			{
			 	printf("\r\n execute command getmediastatus() ... \r\n");
				ATA_Write_Register(COMMAND_REG,0xDA);
			}
			else if (Aux_Data[0] == 'p') 
			{
			 	printf("\r\n execute command checkpowermode() ... \r\n");
				checkpowermode();
			}
 			else if (Aux_Data[0] == 'r') 
			{
			 	printf("\r\n execute command devicereset() ... \r\n");
				ATA_Write_Register(COMMAND_REG,0x08);
				
			}
 			else if (Aux_Data[0] == 'g') 
			{
			 	printf("\r\n execute command devicediag() ... \r\n");
				ATA_Write_Register(COMMAND_REG,0x90);
			}


			Bytes_write = 0;
		}
	} 
}

void ReportAllregister()
{
	ReportDiskStatus();
	ReportErrorStatus();
	printf("Drive/head register = %bx\r\n", ATA_Read_Register(DRIVE_HEAD));
	printf("Sector count register = %bx\r\n", ATA_Read_Register(SECTOR_COUNT));
	printf("Sector number register = %bx\r\n", ATA_Read_Register(SECTOR_NUMBER));
	printf("Cylinder register (low) = %bx\r\n", ATA_Read_Register(CYLINDER_LOW));
	printf("Cylinder register (hi) = %bx\r\n", ATA_Read_Register(CYLINDER_HI));
	printf("data port register = %bx\r\n", ATA_Read_Register(DATA_PORT));

}


/*
unsigned char ReadDiskStatus()
{
	unsigned char reg_val;
	CS1n   = 1;
	CS0n   = 1;
	A2     = 1;
	A1     = 1;
	A0     = 1;
	RDn = 0;
	reg_val = P0;
	RDn = 1;
	return reg_val;
}

unsigned char ReadDiskErrorReg()
{
	unsigned char reg_val;
	P1 &= ERROR_REG;
	RDn = 0;
	reg_val = P0;
	RDn = 1;
	return reg_val;
}
 */
void ReportDiskStatus()
{
	unsigned char diskstatus;
	diskstatus = ATA_Read_Register(STATUS_REG);
	printf("--------------------------------\n\r");
	printf("status register value  = %bX\n\r",diskstatus);
	printf("BUSY DRDY  DWF  DSC  DRQ  CORR INDEX ERROR \n\r");
	printf("  %bX    %bX    ",diskstatus >> 7 &1  ,diskstatus >> 6 &1);
	printf("%bX    %bX    "  ,diskstatus >> 5 &1,diskstatus >> 4 &1);
	printf("%bX     %bX    " ,diskstatus >> 3 &1,diskstatus >> 2 &1);
	printf("%bX     %bX\n\r" ,diskstatus >> 1 &1,diskstatus &1);
	delay (100);

}

unsigned char ATA_Read_Register(unsigned char breg)
{
	unsigned char return_val;
	P1 = breg;
	delay(100);
	RDn = 0;
	delay(100);
	return_val = P0;
	delay(100);
	RDn = 1;
	return return_val;
}

void ATA_Write_Register(unsigned char breg, unsigned char value)
{
	P1 = breg;
	P0 = value;
	delay(100);
	WRn = 0;
	delay(100);
	WRn = 1;
	delay(100);
}


void ReportErrorStatus()
{
	unsigned char diskerrorcode;
	diskerrorcode = ATA_Read_Register(ERROR_REG);

	printf("----------------------------------\n\r");
	printf("Error register value  = %bX\n\r",diskerrorcode);
	printf("BBK  UNC   ---- INDF ---  ABRT TK0NF AMNF\n\r");
	printf("  %bX    %bX    ",diskerrorcode >> 7 &1  ,diskerrorcode >> 6 &1);
	printf("%bX    %bX    "  ,diskerrorcode >> 5 &1   ,diskerrorcode >> 4 &1);
	printf("%bX     %bX    " ,diskerrorcode >> 3 &1  ,diskerrorcode >> 2 &1);
	printf("%bX     %bX\n\r" ,diskerrorcode >> 1 &1 ,diskerrorcode  &1);
}

void ReportDiskIdentify()
{
		P1 = 0xF7;
		P0 = 0xEC;
		WRn = 0;
		delay(30000);
		WRn = 1;
		printf("ATA_Write_Register(COMMAND_REG,0xEC)\n\r");
}

void disk_sleep(void)
{
		P1 = 0xF7;
		P0 = 0xE1;
		delay(3000);
		WRn = 0;
		delay(30000);
		WRn = 1;
		delay(30000);
}

void disk_wakeup(void)
{
		P1 = 0xF7;
		P0 = 0xE6;
		delay(30000);
		WRn = 0;
		delay(30000);
		WRn = 1;
		delay(30000);
}


void checkpowermode(void)
{
		P1 = 0xF7;
		P0 = 0xE5;
		WRn = 0;
		delay(30000);
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

