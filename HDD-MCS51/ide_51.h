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

#define ATA_HEAD_USE_LBA	0x40


/*
unsigned int  idata DataSpeed;
unsigned char idata DataBits;
unsigned char idata Parity;
unsigned char idata StopBits;
unsigned char idata SerialSet;
*/
unsigned char g_AtaDevConfig[ATA_DEVICE_COUNT_MAX];
void ataDriveInit(void);

void Init_serial();
unsigned char idata Bytes_write; // len data_ethernet
unsigned char idata Aux_Data[32];           //' TCP AUX DATA = INTERGER < PACKET - 55 BYTE AND CHECKRAM
unsigned char sectordata[512];
unsigned char idata word_counter = 0;
unsigned char idata  buffer[7]; 


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

void ataWriteByte(unsigned char  reg, unsigned char  dat);
unsigned char ataReadSectorsCHS( unsigned char Drive, 
								 unsigned char Head, 
								 unsigned int Track,
								 unsigned char Sector,
								 unsigned int numsectors,
								 unsigned char *Buffer);
unsigned char  ataStatusWait(unsigned char  mask, unsigned char waitStatus);
void ataReadDataBuffer(u08 *Buffer, u16 numSector);
unsigned char ataReadSectors(	unsigned char Drive, 
								unsigned long lba,
								unsigned int numsectors,
                            	unsigned char *Buffer);
unsigned char ataReadSectorsLBA(	unsigned char Drive, 
									unsigned long lba,
									unsigned int numsectors,
                            		unsigned char *Buffer);
void ataDiskErr(void);


//begin from fat.h
extern unsigned char fatInit( unsigned char device);
extern unsigned int fatClusterSize(void);
extern unsigned long fatClustToSect(unsigned long clust);
extern unsigned long fatGetDirEntry(unsigned int entry, unsigned int count);
extern unsigned long fatGetFilesize(void);
extern char* fatGetFilename(void);
extern char* fatGetDirname(void);
extern void fatLoadCluster(unsigned long cluster, unsigned char *buffer);
extern unsigned long fatNextCluster(unsigned long cluster);
// end from fat.h


typedef struct 
{
	unsigned int  cylinders;
	unsigned char heads;
	unsigned char sectors;
	unsigned long sizeinsectors;
	unsigned char LBAsupport;
	char model[41];
} typeDriveInfo;


typeDriveInfo idata ataDriveInfo;
