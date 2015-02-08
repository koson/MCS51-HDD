/* This is a small demo file for FAT32 library usage
Written by Tamas Bodorics (Tomi)

Some notes:
-----------
 - I use it with a PIC18F6720 (3.5kByte RAM) HW SPI @11.0592 MHz crystal; tested with 64-256MB MMCs
 - The RAM area is moved to upper region by #locate directives
 - The MMC has 2 GND pins; one of them is pulled up by a resistor (10k) to +5V and connected to PIN_A4
   so the MMC is "hot swappable".
 - The MMC has to format FAT32, 512bytes/cluster
 - The number of opened files at a time is limited to 2 because of RAM limitations (some dsPICs have 8K RAM... ;-) )
 - No long names; use DOS-like 8+3 name format
 - No MKDIR and RMDIR functions (not yet...) so create the subdirs right after the format (and use 8+3 capital characters...)
   I use the following DOS batch in XP:

      format I: /A:512 /V:TOMI /FS:FAT32
      mkdir I:\BERS
      mkdir I:\WINDS

 - Use '/' as directory separator, e.g. "MYDIR/ELEMENT.WND" "MYDIR/SUBDIR/OTHER.TXT" etc.
 - The possible modes for file open (char and NOT string as in std. C): 'r'(read) 'w'(write; the previous content will overwritten) 'a'(append)
 - fputs is reserved keyword by CCS C so use fputstring() instead
 - In the example the EVENT.LOG file uses the Hungarian date format (YYYY.MM.DD. HH.mm.SS) sorry...
 - There are some functions normally not to use:
       -  fflush(f) Use it only if you want to flush data into MMC while you keep the file opened
       -  TryFile() tries to open the file incl. resolving the path
       -  cwd(fname,f) is used by TryFile to resolve the path name
       -  fcreate() If you open a file with 'a' or 'w' the file is automatically created if not exist
*/

#include <18F6720.h>
..........
#include <string.h>

typedef struct  {
unsigned long    tm_year;
char            tm_mon;
char            tm_day;
char            tm_mday;    
char            tm_hour;    
char            tm_min;     
char            tm_sec;     
} TimeRecord;

....................

TimeRecord myrec; // this variable is updated in regular intervals in DoIdle()

....................
....................
....................

#include "MyMMCFat32.h"
#include "MyMMCFat32.c"

....................
....................

void main()
{
char f,v,msg[64],gfilename[32];
char gPrevCard,gActCard; // previous and actual card states (inserted, removed)

.................... // other declarations
....................

.................... // INIT code parts
....................

   InitClockInt(); // init the clock chip
   ReadClock();    // read the current time

   if (!input(CardInserted)) { // if MMC card is in place
      do {
         output_high(REDLED);
         v = MMCInit(); // init the card
         delay_ms(50);
         output_low(REDLED);
         delay_ms(50);
      } while (!v);
      output_high(YELLOWLED);
      InitFAT(); // init the file system
      output_low(YELLOWLED);
      output_low(REDLED);
   }
..................

   strcpy(gfilename,"EVENTS.LOG");
   f = fopen(gfilename,'a'); // open EVENTS.LOG for append
   if (f < MAXFILES) {
      sprintf(msg,"%04lu.%02u.%02u. %02u:%02u:%02u ",myrec.tm_year,myrec.tm_mon,myrec.tm_mday,myrec.tm_hour,myrec.tm_min,myrec.tm_sec);
      fputstring(msg,f);
      strcpy(msg,"System started\r\n");
      fputstring(msg,f);
      fclose(f);
   }

   while (1) {
   restart_wdt();
   ResetPorts();
      DoIdle(); // Idle function incl. clock update

....................
....................

      gActCard = input(CardInserted);
      if (gActCard) output_high(REDLED);
         else output_low(REDLED);
      if (gActCard == 0 && gPrevCard != 0) { // card was pulled out then pushed back now
         delay_ms(50);
         do {
            output_high(REDLED);
         v = MMCInit();
            delay_ms(50);
            output_low(REDLED);
            delay_ms(50);
         } while (!v);
         output_high(YELLOWLED);
         InitFAT();
         output_low(YELLOWLED);
         output_low(REDLED);
         delay_ms(50);
         strcpy(gfilename,"EVENTS.LOG");
         f = fopen(gfilename,'a');
         if (f < MAXFILES) {
            sprintf(msg,"%04lu.%02u.%02u. %02u:%02u:%02u ",myrec.tm_year,myrec.tm_mon,myrec.tm_mday,myrec.tm_hour,myrec.tm_min,myrec.tm_sec);
            fputstring(msg,f);
            strcpy(msg,"Memory card replacement\r\n");
            fputstring(msg,f);
            fclose(f);
         }
      }
      gPrevCard = gActCard;
   }
}

/* other short examples:
1: to send out a file content to serial line

               strcpy(gfilename,"SMSDATA.TXT");
               f = fopen(gfilename,'r');
               if (f < MAXFILES) {
                  while (fgetch(&c,f)) fputc(c,HOST2);
                  fclose(f);
               }

2: to save daily measures:
      sprintf(gfilename,"BERS/%04lu%02u%02u.BER",myrec.tm_year,myrec.tm_mon,myrec.tm_mday); // the file name is YYYYMMDD.BER
      f = fopen(gfilename,'a');
      for (actidx=0;actidx<22;actidx++) { // to save the last 22 updated measures
         if (!bit_test(saveflags,actidx)) continue; // skip if this record is not updated
         gBER = gLastBERs[actidx];
         if (f < MAXFILES) fwrite(&gBER,sizeof(gBER),f);
      }
      fclose(f);

*/ 