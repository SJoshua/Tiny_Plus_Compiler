/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
int EchoSource = TRUE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

int main(int argc, char *argv[]) {
	TreeNode *syntaxTree;
	char pgm[120]; /* source code file name */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	strcpy(pgm, argv[1]);
	if (strchr(pgm, '.') == NULL)
		strcat(pgm, ".tny");

	source = fopen(pgm, "r");
	if (source == NULL) {
		fprintf(stderr, "File %s not found\n", pgm);
		exit(1);
	}
	listing = stdout; /* send listing to screen */
	fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
	while (getToken() != ENDFILE)
		;
#else
	syntaxTree = parse();
	if (TraceParse) {
		fprintf(listing, "\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	if (!Error) {
		if (TraceAnalyze) fprintf(listing, "\nChecking Symbol Table...\n");
		buildSymtab(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nSymbol Table Checking Finished\n");
	}
	if (!Error) {
		if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
	}
#if !NO_CODE
	if (!Error) {
		code = listing;
		fprintf(code, "\nOutput Intermediate Code:\n");
		codeGen(syntaxTree);
	}
#endif
#endif
#endif
	fclose(source);
	return 0;
}
