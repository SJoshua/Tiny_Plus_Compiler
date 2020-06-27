/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

static char * temp_v;

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{
	char temp[256];
	char *label;
	TreeNode * p1, * p2, * p3;
  int savedLoc1,savedLoc2;
  switch (tree->kind.stmt) {

      case IfK :
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         p3 = tree->child[2] ;
         /* generate code for test expression */
		 if(p1->kind.exp != OpK)
			if(p1->kind.exp == IdK)
				strcpy(temp, p1->attr.name);
			else
			sprintf(temp, "%d", p1->attr.val);
         else
		 {
			cGen(p1);
			strcpy(temp, temp_v);
		 }
         savedLoc1 = emitSkip(1) ;
         /* recurse on then part */
         cGen(p2);
		 if(p3 != NULL)
			savedLoc2 = emitSkip(1);
		 emit("label","","",label=newlabel());
         emitBackup(savedLoc1) ;
         emit("j=",temp,"false",label);
         emitRestore() ;
         /* recurse on else part */
		 if(p3 != NULL)
		 {
			cGen(p3);
			emit("label","","",label=newlabel());
			emitBackup(savedLoc2) ;
			emit("goto","","",label);
			emitRestore() ;
		 }
         break; /* if_k */

      case RepeatK:
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
		 emit("label","","",label=newlabel());
         /* generate code for body */
         cGen(p1);
         /* generate code for test */
         cGen(p2);
         emit("j=",temp_v,"false",label);
         break; /* repeat */

	  case WhileK:
		  p1 = tree->child[0] ;
		  p2 = tree->child[1] ;
		  emit("label","","",label=newlabel());
		  cGen(p1);
		  strcpy(temp, temp_v);
		  savedLoc1 = emitSkip(1);
		  cGen(p2);
		  emit("goto","","",label);
		  emit("label","","",label=newlabel());
		  emitBackup(savedLoc1);
		  emit("j=",temp,"false",label);
		  emitRestore();
		  break;

      case AssignK:
         if(tree->child[0]->kind.exp != OpK)
			 if(tree->child[0]->kind.exp == IdK)
				emit(":=",tree->child[0]->attr.name,"",tree->attr.name);
			 else
			 {
			sprintf(temp, "%d", tree->child[0]->attr);
				emit(":=",temp,"",tree->attr.name);
			 }
         else
		 {
			 cGen(tree->child[0]);
			 emit(":=",temp_v,"",tree->attr.name);
		 }
         break; /* assign_k */

      case ReadK:
         emit("read","","",tree->attr.name);
         break;

      case WriteK:
         if(tree->child[0]->kind.exp != OpK)
			 if(tree->child[0]->kind.exp == IdK)
				emit("write",tree->child[0]->attr.name,"","");
			 else
			 {
			sprintf(temp, "%d", tree->child[0]->attr.val);
				emit("write",temp,"","");
			 }
         else
		 {
			 cGen(tree->child[0]);
			 emit("write",temp_v,"","");
		 }
         break;
      default:
         break;
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{
	char temp[256],temp2[256];
	TreeNode * p1=NULL, * p2=NULL;
	switch (tree->kind.exp) {

    case OpK :
        p1 = tree->child[0];
		if(tree->child[1] != NULL)
			p2 = tree->child[1];
        
		if(p1->kind.exp != OpK)
			if(p1->kind.exp == IdK)
				strcpy(temp, p1->attr.name);
			else
				sprintf(temp, "%d", p1->attr.val);
        else
		{
			cGen(p1);
			strcpy(temp, temp_v);
		}

        if(p2 != NULL)
			if(p2->kind.exp != OpK)
				if(p2->kind.exp == IdK)
					strcpy(temp2, p2->attr.name);
				else
			sprintf(temp2, "%d", p2->attr.val);
			else
			{
				cGen(p2);
				strcpy(temp2, temp_v);
			}
		
		temp_v=newtemp();
        switch (tree->attr.op) {
            case PLUS :
               emit("+",temp,temp2,temp_v);
               break;
            case MINUS :
               emit("-",temp,temp2,temp_v);
               break;
            case TIMES :
               emit("*",temp,temp2,temp_v);
               break;
            case OVER :
               emit("/",temp,temp2,temp_v);
               break;
            case LT :
               emit("<",temp,temp2,temp_v);
               break;
			case LE:
				emit("<=",temp,temp2,temp_v);
				break;
			case GT:
				emit(">",temp,temp2,temp_v);
				break;
			case GE:
				emit(">=",temp,temp2,temp_v);
				break;
            case EQ :
               emit("=",temp,temp2,temp_v);
               break;
			case AND:
				emit("and",temp,temp2,temp_v);
				break;
			case OR:
				emit("or",temp,temp2,temp_v);
				break;
			case NOT:
				emit("not",temp,"",temp_v);
				break;
            default:
               emit("BUG: Unknown operator","","","");
               break;
        } /* case op */
        break; /* OpK */

	default:
		break;
	}
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{
	if (tree != NULL)
	{
		switch (tree->nodekind)
		{
		case StmtK:
			genStmt(tree);
			break;
		case ExpK:
			genExp(tree);
			break;
		default:
			break;
		}
		cGen(tree->sibling);
	}
}

void codeGen(TreeNode * syntaxTree)
{
	cGen(syntaxTree);
	emit("label","","","L0");
	output();
}
