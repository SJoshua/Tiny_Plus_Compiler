/****************************************************/
/* File: code.h                                     */
/* Code emitting utilities for the TINY compiler    */
/* and interface to the TM machine                  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _CODE_H_
#define _CODE_H_

/* code emitting utilities */

void emit( char * op, char * a, char * b, char * c );
void output();
char * newtemp();
char * newlabel();


/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip( int howMany);

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc);

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void);


#endif
