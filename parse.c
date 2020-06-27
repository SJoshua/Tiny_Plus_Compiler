/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "symtab.h"

static TokenType token; /* holds current token */

/* counter for variable memory locations */
static int location = 0;

/* function prototypes for recursive calls */
static void declarations(void);
static TreeNode *stmt_sequence(void);
static TreeNode *statement(void);
static TreeNode *if_stmt(void);
static TreeNode *repeat_stmt(void);
static TreeNode *assign_stmt(void);
static TreeNode *read_stmt(void);
static TreeNode *write_stmt(void);
static TreeNode *expr(void);
static TreeNode *simple_exp(void);
static TreeNode *term(void);
static TreeNode *factor(void);
static TreeNode *bool_exp(void);
static TreeNode *bterm(void);
static TreeNode *bfactor(void);
static TreeNode *while_stmt(void);

static void syntaxError(char *message) {
	fprintf(listing, ">>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;
}

static void match(TokenType expected) {
	if (token == expected)
		token = getToken();
	else {
		if (expected == SEMI)
			syntaxError("missing \';\'\n");
		else if (expected == THEN)
			syntaxError("missing \'then\'\n");
		else if (expected == END)
			syntaxError("missing \'end\'\n");
		else if (expected == UNTIL)
			syntaxError("missing \'until\'\n");
		else if (expected == ASSIGN) {
			if (token == EQ) {
				syntaxError("should be \':=\' instead of \'=\'\n");
				token = getToken();
			} else
				syntaxError("missing \':=\'\n");
		} else if (expected == DO)
			syntaxError("missing \'do\'\n");
		else if (expected == ID)
			syntaxError("need a id\n");
		else if (expected == RPAREN)
			syntaxError("parenthesis matching error, need a right parenthesis");
	}
}

TreeNode *stmt_sequence(void) {
	TreeNode *t = statement();
	TreeNode *p = t;
	while ((token != ENDFILE) && (token != END) &&
		   (token != ELSE) && (token != UNTIL)) {
		TreeNode *q;
		match(SEMI);
		q = statement();
		if (q != NULL) {
			if (t == NULL)
				t = p = q;
			else /* now p cannot be NULL either */
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode *statement(void) {
	TreeNode *t = NULL;
	switch (token) {
		case IF:
			t = if_stmt();
			break;
		case REPEAT:
			t = repeat_stmt();
			break;
		case ID:
			t = assign_stmt();
			break;
		case READ:
			t = read_stmt();
			break;
		case WRITE:
			t = write_stmt();
			break;
		case WHILE:
			t = while_stmt();
			break;
		case ENDFILE:
			syntaxError("unexpected file end");
			break;
		default:
			syntaxError("unexpected token\n");
			token = getToken();
			break;
	} /* end case */
	return t;
}

TreeNode *if_stmt(void) {
	TreeNode *t = newStmtNode(IfK);
	match(IF);
	if (t != NULL) t->child[0] = bool_exp();
	match(THEN);
	if (t != NULL) t->child[1] = stmt_sequence();
	if (token == ELSE) {
		match(ELSE);
		if (t != NULL) t->child[2] = stmt_sequence();
	}
	match(END);
	return t;
}

TreeNode *repeat_stmt(void) {
	TreeNode *t = newStmtNode(RepeatK);
	match(REPEAT);
	if (t != NULL) t->child[0] = stmt_sequence();
	match(UNTIL);
	if (t != NULL) t->child[1] = bool_exp();
	return t;
}

TreeNode *assign_stmt(void) {
	TreeNode *t = newStmtNode(AssignK);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	match(ASSIGN);
	if (t != NULL) t->child[0] = expr();
	return t;
}

TreeNode *read_stmt(void) {
	TreeNode *t = newStmtNode(ReadK);
	match(READ);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	return t;
}

TreeNode *write_stmt(void) {
	TreeNode *t = newStmtNode(WriteK);
	match(WRITE);
	if (t != NULL) t->child[0] = expr();
	return t;
}

TreeNode *expr(void) {
	TreeNode *t = NULL;
	if (token == STR) {
		t = newExpNode(StrK);
		if (t != NULL)
			t->attr.name = copyString(tokenString);
		match(STR);
	} else if (token == NUM || token == ID || token == BTRUE || token == BFALSE || token == NOT || token == LPAREN)
		t = bool_exp();
	return t;
}

TreeNode *simple_exp(void) {
	TreeNode *t = term();
	while ((token == PLUS) || (token == MINUS)) {
		TreeNode *p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();
		}
	}
	return t;
}

TreeNode *term(void) {
	TreeNode *t = factor();
	while ((token == TIMES) || (token == OVER)) {
		TreeNode *p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

TreeNode *factor(void) {
	TreeNode *t = NULL;
	switch (token) {
		case NUM:
			t = newExpNode(ConstK);
			if ((t != NULL) && (token == NUM))
				t->attr.val = atoi(tokenString);
			match(NUM);
			break;
		case ID:
			t = newExpNode(IdK);
			if ((t != NULL) && (token == ID))
				t->attr.name = copyString(tokenString);
			match(ID);
			break;
		case LPAREN:
			match(LPAREN);
			t = bool_exp();
			match(RPAREN);
			break;
		default:
			syntaxError("unexpected token\n");
			token = getToken();
			break;
	}
	return t;
}

TreeNode *bool_exp(void) {
	TreeNode *t = bterm();
	while (token == OR) {
		TreeNode *p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = bterm();
		}
	}
	return t;
}

TreeNode *bterm(void) {
	TreeNode *t = bfactor();
	while (token == AND) {
		TreeNode *p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			p->child[1] = bfactor();
		}
	}
	return t;
}

TreeNode *bfactor(void) {
	TreeNode *t = NULL;
	switch (token) {
		case BTRUE:
		case BFALSE:
			t = newExpNode(BoolK);
			if ((t != NULL) && (token == BTRUE)) {
				t->attr.val = 1;  //true
				match(BTRUE);
			} else {
				t->attr.val = 0;  //false
				match(BFALSE);
			}
			break;
		case NOT:
			t = newExpNode(OpK);
			t->attr.op = token;
			match(NOT);
			if (t != NULL)
				t->child[0] = bfactor();
			break;
		case NUM:
		case ID:
		case LPAREN:
			t = simple_exp();
			if ((token == LT) || (token == EQ) || (token == LE) || (token == GT) || (token == GE)) {
				TreeNode *p = newExpNode(OpK);
				if (p != NULL) {
					p->child[0] = t;
					p->attr.op = token;
					t = p;
				}
				match(token);
				if (t != NULL)
					t->child[1] = simple_exp();
			}
			break;
		default:
			syntaxError("unexpected token\n");
			token = getToken();
			break;
	}
	return t;
}

void declarations(void) {
	ExpType type;
	while (token == INT || token == BOOL || token == STRING)  //decl;
	{
		switch (token) {
			case INT:
				type = Integer;
				break;
			case BOOL:
				type = Boolean;
				break;
			case STRING:
				type = String;
				break;
			default:
				type = Void;
		}
		//type-specifier
		match(token);
		//varlist
		st_insert(tokenString, type, lineno, location++);
		match(ID);
		while (token == COMMA) {
			match(COMMA);
			st_insert(tokenString, type, lineno, location++);
			match(ID);
		}
		match(SEMI);  //';' is expected
	}
}

TreeNode *while_stmt(void) {
	TreeNode *t = newStmtNode(WhileK);
	match(WHILE);
	if (t != NULL)
		t->child[0] = bool_exp();
	match(DO);
	if (t != NULL) t->child[1] = stmt_sequence();
	match(END);
	return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly 
 * constructed syntax tree
 */
TreeNode *parse(void) {
	TreeNode *t;
	token = getToken();
	if (token == INT || token == BOOL || token == STRING) {
		declarations();
	}
	t = stmt_sequence();
	if (token != ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}
