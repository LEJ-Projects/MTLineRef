﻿/**********************************************************************
**
** Program:   MTLineRef.cpp
**
** Purpose:  Reads a BASIC file and outputs the first 10 characters of
**           lines referenced by other lines followed by the line numbers 
**           that reference them.
**
** Author: L. Johnson,
** Created: 03 Apr 23
** Current Version: 1.1
***********************************************************************
**  Revision   Date       Engineer       Description of Change
**  --------   --------   ------------   ------------------------------
**  1.0        9 Apr 23   L. Johnson     Initial Release 
**  1.1        5 May 23   L. Johnson     Address Cntl-Z problem & other issues                                  
**********************************************************************/
#define TRUE  1
#define FALSE 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//using namespace std;
void cont(void);
int main(int argc, char** argv) {

FILE  *fi, *fo;
  int i,j;                         /* Loop index */
  int done=0;
  char fi_name[256];      // Input file name
  char fo_name[256];      // Output file name
  char str_in[256];       // Input String
  char wrk_str[256];      // Working Sttring (Input string less spaces)
  char ln_str[256];       // Line  string
  char tmpstr[256];       // Temporary string
  int numq;               // Number of double quotes in a line
  int qi;                 // Quote index (increments on end quotes)
  int qidxb[256];          // Quote begins
  int qidxe[256];		// Quote ends
  int qfnd;		// Quote Found flag
  int scrOut=0;   // Screen output is off...
  int los;  // Length of string
  int ns;   // number of items scanned
  int lnr[5000];  //  Line number doing the referencing
  int rln[5000];  //   Referenced line number (in line number doing the referencing)
  int lri=0;   // line reference index (zero indixed0
  int total_lines=0;
  int remF;   // Remark Flag...Set to TRUE if a remark found otherwise FALSE
  int delSp=TRUE;   // Delete space flag set to FALSE

    // Keyword Parameters
  int srchType;   // 0: inquotes don't search, 1: Search for keywordsk
                  // 2:get line numbers
  int p2i;        // Pass 2 char increment
  int qreg;       // Quote Region - True when between quotes otherwise false
  int kwfnd;	// Number of keywords found in a line
  int kib[256];   // Key index begin
  int kie[256];   // key  index ends

    // Line Number Search Parameters
  int lval;    // Value of line number being processed
  int lnfnd;   // Number of line numbers found in the line
  int lnb[256];     // Line number begin index
  int lne[256];     // Line number end index
  int lccnt=0;   // Line character count...
  int lnv[256];  // Line number value
  int rlnv;     // Referenced line number value
  int li=0;   // line increment (zero indixed0

  typedef struct {
    int val;   //line number value
    char ln10ch[10];  // 1st 10 characters of the line
    } lnStruct;
  lnStruct ln[5000];  // Line structure


  printf("MTLineRef - Version 1.1\n");


  printf("Enter input file name: ");
  scanf("%s",fi_name);

  fi = fopen(fi_name,"rb");
  if (fi==NULL) {
    printf("Unable to open %s for input.\n",fi_name);
    cont();
	exit(1);
  }


  printf("Enter output file name: ");
  scanf("%s",fo_name);
  fo = fopen(fo_name,"w");
  if (fo==NULL) {
    printf("Unable to open %s for output.\n",fo_name);
    cont();
    exit(1);
  }

// 
  done=0;
  while (done == 0) {

    if (fgets(str_in,256,fi) == NULL) done=1;
    if (strlen(str_in)<3) done=1; // nothing much in this line so we quit
    if (str_in[0]==26) done=1;  // A control-Z as the 1st char will terminate this input file
    if (done==0) {
      // Remove trailing carriage return & linefeed
      // Remove carriage return & linefeed
      los=strlen(str_in);  // length of input string
      for (j=0;j<los;j++) {
        if((str_in[j]==13) ||(str_in[j]==10)) str_in[j]=0;
      }
      los=strlen(str_in);  // length of input string
      ns=sscanf(str_in,"%d %s",&lval, ln_str);
      ln[li].val=lval;

      strcpy(str_in,strstr(str_in,ln_str));   // basically the string without the line number
      los=strlen(str_in);  // length of input string

      // strip spaces ( & convert all text to upper case
      //  (except for remarks and data statements)
      qfnd=FALSE;
      numq=0;
      remF=FALSE;
      j=0;  // wrk_str index
      for (i=0;i<los;i++){
        if (str_in[i] =='"') {  //The case for quotes
          numq++;
          wrk_str[j]=str_in[i];
          j++;
        } else {  // The case for non quotes
          if ((numq%2)>0) {  // We are inside a quote, copy the
                             // chararacter and do nothing more
            wrk_str[j]=str_in[i];
            j++;
          } 
          if ((numq%2)==0) {  // Outside quotes, we got work to do...
            //  First check for comments (treat Data statements like comments ...
            if ((((str_in[i] & 0xdf) =='R') &&  
              ((str_in[i+1] & 0xdf) =='E') &&
              ((str_in[i+2] & 0xdf) =='M'))  ||
              (str_in[i] == 0x27)  || 
              ((str_in[i]  =='D') &&  
              (str_in[i+1] =='A') &&
              (str_in[i+2] =='T') &&
              (str_in[i+3] =='A'))) remF=TRUE;           
            if (remF==TRUE) {   // It is a remark - ignore rest of line
              strcpy(&wrk_str[j],&str_in[i]);
              i=los;
            }
            if (remF==FALSE) {   // It is not a remark - process the character
              if ((str_in[i]>96)  && (str_in[i]<123)) str_in[i]=str_in[i]-32;
              if ((str_in[i] != ' ') || (delSp==FALSE)) 
              { // if not a space, copy to the working string
                wrk_str[j]=str_in[i];
                j++;
              } // if (str_in[i] != ' ') {  // if not a space, copy to the working string
            } // if (remF==FALSE) {   // It is not a remark - process the character
          }  // if ((numq%2)==0) {  // Outside quotes, we got work to do...
        }  // if (str_in[i] =='"')  (else portion) - the case for non quotes
      } //      for (i=0;i<los;i++){
      if (remF==FALSE) wrk_str[j]=0;   // Terminate string for non remarks...
      // Count double quotes & note their location  (might be able to do this earlier)
      numq=0;
      qi=0;
      qfnd=FALSE;
      for (i=0;i<(int)strlen(wrk_str);i++){
        if (wrk_str[i] =='"') {
          if ((numq%2)==0) {
            qidxb[qi]=i;
            qfnd=TRUE;
          }
          if ((numq%2)>0) {
            qidxe[qi]=i;
            qi++;
          }
          numq++;
        } // if (ln_str[i] =='"') {
      }  //for (i=0;i<strlen(wrk_str);i++){
      // Parse keywords & line numbers)
      srchType=1;   // Search for keywords
      kwfnd=0;  // Set keyword found flag to 0
      lnfnd=0;  // Set line number found flag to 0
      for (p2i=0;p2i<(int)strlen(wrk_str);p2i++) {
      // Check if we are in a quote region
        if  (qfnd==TRUE) {
          qreg=FALSE;   // Assume not in a quote region
          for (i=0;i<qi;i++) if ((p2i>=qidxb[i]) && (p2i<=qidxe[i])) qreg=TRUE;
          if((qreg==FALSE) && (srchType==0)) srchType=1; // Go back to Key search
          if (qreg==TRUE) srchType=0;   // No key search or number search while in quote region
        }  // if  (qfnd==TRUE) {
        switch (srchType) {
        case 1:
          // Check for keyword search...
          if (strncmp(&wrk_str[p2i],"THEN",4) ==0) {// Found THEN
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+3;
            p2i=p2i+3;
            while (wrk_str[p2i+1]==' ')p2i++;  // skip spaces before possible number
            if(isdigit(wrk_str[p2i+1]))srchType=2; // Time to look for numbers (maybe)
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"THEN",4) ==0)
          if (strncmp(&wrk_str[p2i],"ELSE",4) ==0) {// Found ELSE
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+3;
            p2i=p2i+3;
            while (wrk_str[p2i+1]==' ')p2i++;  // Trim spaces before possible number
            if(isdigit(wrk_str[p2i+1]))srchType=2; // Time to look for numbers (maybe)
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"ELSE",4) ==0)
          if (strncmp(&wrk_str[p2i],"GOTO",4) ==0) {// Found GOTO
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+3;
            p2i=p2i+3;
            srchType=2; // Time to look for numbers
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"GOTO",4) ==0)
          if (strncmp(&wrk_str[p2i],"GOSUB",5) ==0) {// Found GOSUB
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+4;
            p2i=p2i+4;
            srchType=2; // Time to look for numbers
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"GOSUB",5) ==0)
          if (strncmp(&wrk_str[p2i],"RESTORE",7) ==0) {// Found RESTORE
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+6;
            p2i=p2i+6;
            srchType=2; // Time to look for numbers
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"RESTORE",7) ==0)
          if (strncmp(&wrk_str[p2i],"RESUME",6) ==0) {// Found RESUME
            kib[kwfnd]=p2i;
            kie[kwfnd]=p2i+5;
            p2i=p2i+5;
            srchType=2; // Time to look for numbers
            kwfnd++;
          }  // if (strncmp(&ln_str[p2i],"RESUME",7) ==0)
          if ((strncmp(&wrk_str[p2i],"REM",3) ==0)  //Checking for comments
            || (wrk_str[p2i]=='\''))
            p2i=(int)strlen(wrk_str);  // No need to look further
            break;
        case 2:
          // Check for line numbers search...
          if (isdigit(wrk_str[p2i])) {
                // Save the character
                tmpstr[lccnt]=wrk_str[p2i];
                if (lccnt==0) lnb[lnfnd]=p2i;
                lne[lnfnd]=p2i;  
                lccnt++;
          }  // if (isdigit(wrk_str[p2i])) {
          if (!isdigit(wrk_str[p2i+1]) && (lccnt>0)) {
            // If next character is not a digit and a linc number was found, terminate line string
            tmpstr[lccnt]=0;  //  Terminate string,.
            lnv[lnfnd]=atoi(tmpstr); // store value
            rlnv=atoi(tmpstr); // store value
            lnfnd++;
            lccnt=0;
            if (wrk_str[p2i+1]!=',') srchType=1;  // Switch search type to key words if not a comma
            // For comma case, continue searching for line numbers for ongoto and ongosub cases
            if (strncmp(&wrk_str[p2i],"ELSE",4)==0) {
              p2i=p2i-1;   //Decrement so keyword is found on next loop increment
              srchType=1; // Continuing search for keywords so "ELSE" will be found.
            } // if (strncmp(&wrk_str[p2i],"ELSE",4)==0) {
          }// if ((!isdigit(wrk_str[p2i]) &&  (wrk_str[p2i]!=' ') {
          break;
        } // switch (srchType) {
      } //for (p2i=0;p2i<strlen(wrk_str);p2i++) {
      if (scrOut==1)  printf("%d ",lval);
      strcpy(tmpstr,wrk_str);
      if (strlen(tmpstr)<10) {
          strcat(tmpstr,"          ");
        // *DEBUG*  
        // for (j=0;j<strlen(tmpstr);j++) printf("%d ",tmpstr[j]);
          //  printf("%s \n", tmpstr);
         // cont();
      }
      tmpstr[10]=0;  // Terminate the string
      strcpy(ln[li].ln10ch, tmpstr);
      if (scrOut==1)  printf("%s ",ln[li].ln10ch);
      if (lnfnd==0){
        if (scrOut==1)  printf("\n");
      }
      if (lnfnd>0) { // There are line numbers to record  
        for (i=0;i<lnfnd;i++) {   // There might be more than one
          lnr[lri]=lval;
          rln[lri]=lnv[i];
          if (scrOut==1)  printf(" %d ",rln[lri]);
          lri++;;
        }  // for (i=0;i<lnfnd;i++) {
        if (scrOut==1)  printf("\n");
      } // if lnfnd>0) { // There are line numbers to record
      li++;  // Increment line count.
    } else {
	    done = 1;
    }  // if (done==0)
  } // end while

  total_lines=li;
  fclose(fi);

  // Lets now look at what we got
  printf("Ready to examine data\n");
  cont();

  for (li=0;li<total_lines;li++) {
    lnfnd = 0; 
    lval=ln[li].val;
    for (i=0;i<lri;i++) {
      if (rln[i]==lval) {
        if (lnfnd==0) { // 1st one - print some stuff...
          printf("%d %s ",lval, ln[li].ln10ch);
          fprintf(fo,"%d %s ",lval, ln[li].ln10ch);
        }
        printf(" %d ",lnr[i]);
        fprintf(fo," %d ",lnr[i]);
        lnfnd++;
      }  //if (rln[i]==lval) {
    }  // for (i=0;i<lri;i++) {
    if (lnfnd>0) {
      printf("\n");
      fprintf(fo,"\n");
    }
  }  // for (li=0;li<total_lines) {

  fclose(fo);
  printf("I guess we're done.\n");
  cont();

 }
/****************************************************************
** Function:  cont()
**
** Description:  Prompts operator to press enter to continue.
**
****************************************************************/
void cont(void) {

  getchar();   /* Seems to be necessary to flush stdin */
  printf("Press enter to continue:");
  getchar();   /* This is the one that counts */
}

