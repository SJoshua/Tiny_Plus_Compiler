/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void insertNode( TreeNode * t)
{
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case AssignK:
        case ReadK:
          if (st_lookup(t->attr.name) == -1)
          // not yet in table, so treat as new definition
            symtabError(t->lineno,"undeclared identifier");
          else
          // already in table, so ignore location, add line number of use only
            st_addline(t->attr.name,t->lineno);
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          if (st_lookup(t->attr.name) == -1)
          // not yet in table, so treat as new definition
            symtabError(t->lineno,"undeclared identifier");
          else
          // already in table, so ignore location, add line number of use only
            st_addline(t->attr.name,t->lineno);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)	
{
	switch (t->nodekind)
	{
	case ExpK:
		switch (t->kind.exp)
		{
		case OpK:
			if (t->attr.op == NOT && t->child[0]->type != Boolean)
				typeError(t,"\'not\' operator needs a boolean expression");
			else if(t->attr.op != NOT && t->child[0]->type != t->child[1]->type)
				typeError(t,"the types of operands are not equal");
			if (t->attr.op == EQ || t->attr.op == LT || t->attr.op == LE || t->attr.op == GT || t->attr.op == GE)
				t->type = Boolean;
			else if(t->attr.op == AND || t->attr.op == OR || t->attr.op == NOT)
				t->type = Boolean;
			else
				t->type = Integer;
			break;
		case ConstK:
			t->type = Integer;
			break;
		case IdK:
			t->type = st_gettype(t->attr.name);
			break;
		case StrK:
			t->type = String;
			break;
		case BoolK:
			t->type = Boolean;
			break;
        default:
          break;
		}
		break;
	case StmtK:
		switch (t->kind.stmt)
		{ 
		case IfK:
          if (t->child[0]->type != Boolean)	
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
			t->type = st_gettype(t->attr.name);	
          if (t->child[0]->type != t->type)	
            typeError(t->child[0],"assignment of a different type value");
          break;
		case ReadK:
			t->type = st_gettype(t->attr.name);
			break;
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
        case RepeatK:
          if (t->child[1]->type != Boolean)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
		case WhileK:
			if(t->child[0]->type != Boolean)
				typeError(t->child[0],"while test is not Boolean");
			break;
        default:
          break;
		}
		break;
    default:
		break;
	}
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
