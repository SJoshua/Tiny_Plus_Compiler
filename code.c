/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <ctype.h>
#include "globals.h"
#include "code.h"

/* TM location number for current instruction emission */
static int emitLoc = 0 ;

/* Highest TM location emitted so far
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
static int highEmitLoc = 0;

struct{
	char *op;
	char *a;
	char *b;
	char *c;
}mcode[512];	

void emit( char * op, char * a, char * b, char * c )
{
	mcode[emitLoc].op = (char *)malloc(sizeof(char)*(strlen(op)+1));
	mcode[emitLoc].a = (char *)malloc(sizeof(char)*(strlen(a)+1));
	mcode[emitLoc].b = (char *)malloc(sizeof(char)*(strlen(b)+1));
	mcode[emitLoc].c = (char *)malloc(sizeof(char)*(strlen(c)+1));
	strcpy(mcode[emitLoc].op, op);
	strcpy(mcode[emitLoc].a, a);
	strcpy(mcode[emitLoc].b, b);
	strcpy(mcode[emitLoc].c, c);
	emitLoc++;
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
}

void output()
{
	int i;
	for(i=0;i<highEmitLoc;i++)
	{
		if(strcmp(mcode[i].op, "+")==0 ||
			strcmp(mcode[i].op, "-")==0 ||
			strcmp(mcode[i].op, "*")==0 ||
			strcmp(mcode[i].op, "/")==0 ||
			strcmp(mcode[i].op, "and")==0 ||
			strcmp(mcode[i].op, "or")==0 ||
			strcmp(mcode[i].op, "<")==0 ||
			strcmp(mcode[i].op, "<=")==0 ||
			strcmp(mcode[i].op, ">")==0 ||
			strcmp(mcode[i].op, ">=")==0 ||
			strcmp(mcode[i].op, "=")==0)
			fprintf(code,"%5d)  %s := %s %s %s\n",i,mcode[i].c,mcode[i].a,mcode[i].op,mcode[i].b);
		else if(strcmp(mcode[i].op, "read")==0)
			fprintf(code,"%5d)  %s %s\n",i,mcode[i].op,mcode[i].c);
		else if(strcmp(mcode[i].op, "write")==0)
			fprintf(code,"%5d)  %s %s\n",i,mcode[i].op,mcode[i].a);
		else if(strcmp(mcode[i].op, ":=")==0)
			fprintf(code,"%5d)  %s %s %s\n",i,mcode[i].c,mcode[i].op,mcode[i].a);
		else if(strcmp(mcode[i].op, "label")==0 ||
			strcmp(mcode[i].op, "goto")==0)
			fprintf(code,"%5d)  %s %s\n",i,mcode[i].op,mcode[i].c);
		else if(strcmp(mcode[i].op, "j=")==0)
			fprintf(code,"%5d)  if %s = %s goto %s\n",i,mcode[i].a,mcode[i].b,mcode[i].c);
	}
}

char * newtemp()
{
	static int n = 0;
	char tmpn[3]={'\0'};	
	char * temp = (char *)malloc(sizeof(char)*6);	
	sprintf(tmpn, "%d", n++);
	strcpy(temp,"t");
	strcat(temp,tmpn);
	return temp;
}

char * newlabel()
{
	static int n = 1;
	char * temp = (char *)malloc(sizeof(char)*4);
	char tmpn[3]={'\0'};
	strcpy(temp,"L");
	sprintf(tmpn, "%d", n++);
	strcat(temp,tmpn);
	return temp;
}

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip( int howMany)
{  int i = emitLoc;
   emitLoc += howMany ;
   if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
   return i;
} /* emitSkip */

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc)
{ //if (loc > highEmitLoc) emitComment("BUG in emitBackup");
  emitLoc = loc ;
} /* emitBackup */

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void)
{ emitLoc = highEmitLoc;}
