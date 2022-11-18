#include <stdio.h>
#include <stdarg.h>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <math.h>

extern FILE* LOGFILEPTR;
typedef char* Elem_t;
#include "diff.h"

Node* nodeCtor ()
{
    Node* newDataPtr = (Node*) calloc (1, sizeof(*newDataPtr));
    assert (newDataPtr != nullptr);

    newDataPtr->type     = Unknown;
    newDataPtr->opValue  = UnknownOp;
    newDataPtr->numValue = 0;
    newDataPtr->varValue = nullptr;
    newDataPtr->left     = nullptr;
    newDataPtr->right    = nullptr;

    return newDataPtr;
}

Node* createNode (Type type, OP opValue, double numValue, char* varValue, Node* left, Node* right)
{
    Node* newNode = nodeCtor ();

    newNode->type     = type;
    newNode->left     = left;
    newNode->right    = right;
    newNode->numValue = numValue;
    newNode->varValue = varValue;
    newNode->opValue  = opValue;
    
    return newNode;
}
int treeCtor (Tree* tree)
{
    assert (tree != nullptr);

    int errors = noErrors;
    Tree* newDataPtr = (Tree*) calloc (1, sizeof(*newDataPtr));

    if (newDataPtr == nullptr)
        return errors |= treePtrError;

    tree->root   = nodeCtor ();
    tree->size   = 1;
    tree->status = noErrors;

    return noErrors;
}


void parseNodeData (Node* curNode, FILE* DBFileptr)
{
    char*  data = (char*) calloc (MAXDATASIZE, sizeof(*data));
    double numValue = 0;

    if (fscanf (DBFileptr, "%lf", &numValue))
    {
        curNode->type     = Num_t;
        curNode->numValue = numValue;
    }

    else if (fscanf (DBFileptr, "%[^()]", data))
    {
        if (strchr (data, '*'))
        {
            curNode->type    = OP_t;
            curNode->opValue = OP_MUL;
        }

        else if (strchr (data, '/'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_DIV;
        }

        else if (strchr (data, '+'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_ADD;
        }

        else if (strchr (data, '-'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_SUB;
        }

        else if (strchr (data, '^'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_POW;
        }

        else if (strcmp (data, "sin") == 0)
        {
            curNode->type = OP_t;
            curNode->opValue = OP_SIN;
        }

        else if (strcmp (data, "cos") ==0)
        {
            curNode->type = OP_t;
            curNode->opValue = OP_COS;
        }

        else 
        {
            curNode->type = Var_t;
            curNode->varValue = data;
        }
    }
}

Node* treeParse (Node* node, FILE* DBFileptr)
{
    char   bracket           = '\0';

    bracket = (char) fgetc (DBFileptr);

    Node* curNode = nullptr; 
    curNode = nodeCtor ();

    if (bracket == '(')
    {
        curNode->left = treeParse (curNode, DBFileptr);
    }
    else 
        ungetc (bracket, DBFileptr);

    bracket = (char) fgetc (DBFileptr);

    if (bracket == ')')
    {
        return curNode;
    }
    else
        ungetc (bracket, DBFileptr);

    parseNodeData (curNode, DBFileptr);

    bracket = (char) fgetc (DBFileptr);

    if (bracket == '(')
    {
        curNode->right = treeParse (curNode, DBFileptr);
    }
    else 
        ungetc (bracket, DBFileptr);


    bracket = (char) fgetc (DBFileptr);

    if (bracket == ')')
    {
        return curNode;
    }
    else 
    {
        ungetc (bracket, DBFileptr);
        return curNode;
    }

    assert (0);
    return nullptr;
}

#define dumpprint(...) fprintf(GraphFilePtr, __VA_ARGS__);
void makeGraph (Tree* tree)
{
    FILE* GraphFilePtr = fopen(GraphFile, "w");
    assert (tree       != nullptr);
    assert (tree->root != nullptr);

    dumpprint ("digraph MyGraph {\n")
    dumpprint ("    node [color=black, shape=record, style=\"rounded, filled\"];\n")
    dumpprint ("    rankdir=TB;\n")
    dumpprint ("    edge[constraint=true];\n")

    dumpprint ("    nd%p [fillcolor=\"#54e3c2\", label=\"%d\"];\n",
                tree->root, tree->root->type);
        dumpprint ("    nd%p [fillcolor=\"#54e3c2\", label=\"{ %d | ",
                    tree->root, tree->root->type);
        
        switch (tree->root->type)
        {
            case OP_t: 
                dumpprint ("%c }", ShortCMD[tree->root->opValue - 1]);
                break;

            case Num_t: 
                dumpprint ("%lf }", tree->root->numValue); 
                break;
            
            case Var_t:
                dumpprint ("%s }", tree->root->varValue);
                break;

            case Unknown:
                dumpprint ("Unknown }");
                break;

            default:
                assert (0);
        }
        dumpprint ("\"];\n");

   treeGraph (tree->root, GraphFilePtr);

    dumpprint ("}\n")

    fclose(GraphFilePtr);
    static int picVersion = 0;

    char buf[MAXCMDSIZE] = "";
    sprintf(buf, "dot -Tsvg -Gcharset=latin1 GraphFile.txt > src/pic%d.svg", picVersion);
    picVersion++;

    system (buf);
}

void treeDump (Tree* tree, const char* str, ...)
{
    assert(tree != nullptr);
    fprintf(LOGFILEPTR, "<hr>\n");

    va_list argPtr = nullptr;
    va_start (argPtr, str);

    fprintf (LOGFILEPTR, "<h2>");
    vfprintf (LOGFILEPTR, str, argPtr);
    fprintf (LOGFILEPTR, "</h2>\n");
    
    makeGraph (tree);
    static int picVersion = 0;
    fprintf (LOGFILEPTR, "<img src = \"src/pic%d.svg\"/>\n", picVersion++);

    return; 
}

void treeGraph (const Node* node, FILE* GraphFilePtr)
{
    assert (node != nullptr);
    assert (GraphFilePtr != nullptr);

    if (node->left)
    {
        dumpprint ("    nd%p [fillcolor=\"#54e3c2\", label=\"{ %d | ",
                    node->left, node->left->type);
        
        switch (node->left->type)
        {
            case OP_t: 
                dumpprint ("%c }", ShortCMD[node->left->opValue - 1]);
                break;

            case Num_t: 
                dumpprint ("%lf }", node->left->numValue); 
                break;

            case Var_t:
                dumpprint ("%s }", node->left->varValue);
                break;

            case Unknown:
                dumpprint ("Unknown }");
                break;

            default:
                assert (0);
        }
        dumpprint (" \"];\n");

        dumpprint ("    nd%p -> nd%p;\n", node, node->left);

        treeGraph (node->left, GraphFilePtr);
    }

    if (node->right)
    {
        dumpprint ("    nd%p [fillcolor=\"#54e3c2\", label=\" { %d |",
                    node->right, node->right->type);

        switch (node->right->type)
        {
            case OP_t: 
                dumpprint ("%c }", ShortCMD[node->right->opValue - 1]);
                break;

            case Num_t: 
                dumpprint ("%lf }", node->right->numValue); 
                break;

            case Var_t:
                dumpprint ("%s }", node->right->varValue);
                break;

            case Unknown:
                dumpprint ("Unknown }");
                break;

            default:
                assert (0);
        }
        dumpprint (" \"];\n");


        dumpprint ("    nd%p -> nd%p;\n", node, node->right);

        treeGraph (node->right, GraphFilePtr);
    }

    return;
}

Node* treeCpy (const Node* node)
{
    Node* newNode = nodeCtor();
    newNode->type = node->type;
    newNode->opValue = node->opValue;
    newNode->numValue = node->numValue;
    newNode->varValue = node->varValue;

    if (node->left)
    {
        newNode->left = treeCpy (node->left);
    }

    if (node->right)
    {
        newNode->right = treeCpy (node->right);
    }

    return newNode;
}

int compareOperation (const Node* node, const Node* childNode)
{
    int isPrint = 0;

    if (childNode->type == OP_t && node->type == OP_t)
    {
        if (node->opValue - childNode->opValue > 1 || fabs (node->opValue - childNode->opValue) == 1)
        {
            isPrint = 1;
        }
        else 
            isPrint = 0;
    }
    else if (node->type != OP_t)
    {
        isPrint = 1;
    }
    else 
        isPrint = 0;

    return isPrint;
}

void printOperation (const Node* node, const char* operation, FILE* const fileToPrint)
{
    int isPrintL = 0;
    int isPrintR = 0;

    isPrintL = compareOperation (node, node->left);
    treePrint (node->left, isPrintL, fileToPrint);

    fprintf (fileToPrint, " %s ", operation);

    isPrintR = compareOperation (node, node->right);
    treePrint (node->right, isPrintR, fileToPrint);
}

void treePrint (const Node* node, const int isPrint, FILE* const fileToPrint)
{
    assert (fileToPrint != nullptr);
    assert (node        != nullptr);

    int isPrintL = 0;
    int isPrintR = 0;

    if (isPrint)
        fprintf (fileToPrint, "(");


    switch (node->type)
    {
        case OP_t:
            switch (node->opValue)
            {
                case OP_DIV:
                    fprintf (fileToPrint, "\\frac");

                    fprintf (fileToPrint, "{");
                    printOperation (node, "}{", fileToPrint);
                    fprintf (fileToPrint, "}");

                    break;

                case OP_MUL:
                    printOperation (node, "\\cdot", fileToPrint);
                    break;

                case OP_ADD:
                    printOperation (node, "+", fileToPrint);
                    break;

                case OP_SUB:
                    printOperation (node, "-", fileToPrint);
                    break;
                
                case OP_POW:
                    fprintf (fileToPrint, "{");
                    printOperation (node, "}^{", fileToPrint);
                    fprintf (fileToPrint, "}");
                    break;

                case OP_SIN:
                    fprintf (fileToPrint, "\\sin{(");

                    isPrintR = compareOperation (node, node->right);
                    treePrint (node->right, isPrintR, fileToPrint);

                    fprintf (fileToPrint, ")}");
                    break;
                case OP_COS:
                    fprintf (fileToPrint, "\\cos{(");

                    isPrintR = compareOperation (node, node->right);
                    treePrint (node->right, isPrintR, fileToPrint);

                    fprintf (fileToPrint, ")}");
                    break;

                case OP_LN:
                    fprintf (fileToPrint, "\\ln{");

                    isPrintR = compareOperation (node, node->right);
                    treePrint (node->right, isPrintR, fileToPrint);

                    fprintf (fileToPrint, "}");
                    break;

                case OP_LOG:
                    fprintf (fileToPrint, "\\log_{");

                    printOperation (node, "}{", fileToPrint);

                    fprintf (fileToPrint, "}");
                    break;
            }
            break;
        
        case Var_t:
            fprintf (fileToPrint, "%s", node->varValue);
            break;
        
        case Num_t:
            fprintf (fileToPrint, "%lg", node->numValue);
            break;

        case Unknown:
            fprintf (fileToPrint, "Unknown");
    }
            

    if (isPrint)
        fprintf (fileToPrint, ")");

    return;
}

int findInTree (Node* node, const char* dataToFind)
{
    if (node->type == Var_t && strcmp(node->varValue, dataToFind) == 0)
        return 1;

    else if (node->left == nullptr && node->right == nullptr)
        return 0;

    if (findInTree (node->left, dataToFind))
        return 1;

    else if (findInTree (node->right, dataToFind))
        return 1;
    
    else
        return 0;
}

#undef dumpprint

#define createNum(NUM) createNode(Num_t, UnknownOp, NUM, nullptr, nullptr, nullptr)
#define dL diff(node->left)
#define dR diff(node->right)
#define cL treeCpy(node->left)
#define cR treeCpy(node->right)
#define SUB(left, right) createNode(OP_t, OP_SUB, 0, nullptr, left, right)
#define ADD(left, right) createNode(OP_t, OP_ADD, 0, nullptr, left, right)
#define MUL(left, right) createNode(OP_t, OP_MUL, 0, nullptr, left, right)
#define DIV(left, right) createNode(OP_t, OP_DIV, 0, nullptr, left, right)
#define POW(left, right) createNode(OP_t, OP_POW, 0, nullptr, left, right)
#define COS(right) createNode(      OP_t, OP_COS, 0, nullptr, nullptr, right)
#define SIN(right) createNode(      OP_t, OP_SIN, 0, nullptr, nullptr, right)
#define LOG(left, right) createNode(OP_t, OP_LOG, 0, nullptr, left, right)
#define LN(right) createNode(       OP_t, OP_LN,  0, nullptr, nullptr, right)

Node* diff (const Node* node)
{
    fprintf (stderr, "%d\n", node->type);

    switch (node->type)
    {
        case Num_t:
            {
            return createNum (0);
            break;
            }

        case Var_t:
            {
            return createNum (1);
            break;
            }

        case OP_t:
            switch (node->opValue)
            {
                case OP_ADD:
                    return ADD (dL, dR);
                
                case OP_SUB:
                    return SUB (dL, dR);

                case OP_MUL:
                    return ADD (MUL (dL, cR), MUL (cL, dR));

                case OP_DIV:
                    return DIV (SUB (MUL (dL, cR), MUL (cL, dR)), MUL (cR, cR));

                case OP_POW:
                    if (findInTree (node->right, "x") && findInTree(node->left, "x"))
                    {
                        return MUL (POW (cL, cR), diff (MUL (LN (cL), cR)));
                    }
                    else if (findInTree (node->right, "x"))
                    {
                        return MUL (LN (cL), MUL (POW (cL, cR), dR));
                    }
                    else if (findInTree (node->left, "x"))
                    {
                        return MUL (cR, POW (cL, SUB (cR, createNum(1))));
                    }
                    else 
                        return createNum(pow (node->left->numValue, node->right->numValue));

                case OP_COS: 
                    return MUL (dR, MUL (createNum (-1), SIN (cR)));

                case OP_SIN:
                    return MUL (dR, COS (cR));

                case OP_LN:
                    return DIV (createNum (1), cR);

                case OP_LOG:
                    return DIV (dR, MUL (cR, LN (cL)));
            }
            break;

        default:
            assert (0);
    }
}

int nodeEquals (Node* node, double val)
{
    if (node->type != Num_t) return 0;

    if (fabs (node->numValue - val) < EPS)
        return 1;
    else
        return 0;
}

#define OP_is(op) (node->type == OP_t && node->opValue == op)
#define IS_VAL(side, val) (

void makeEasier (Node* node)
{
}

#define laprint(...) fprintf (fileToPrint, __VA_ARGS__)
void latexBegin (FILE* fileToPrint)
{
    laprint ("\\documentclass{article}\n");
    laprint ("\\usepackage[utf8]{inputenc}\n");
    laprint ("\\title{11}\n");
    laprint ("\\author{Витя Тяжелков}\n");
    laprint ("\\date{November 2022}\n");
    laprint ("\\begin{document}\n");
}

void latexEnd (FILE* fileToPrint)
{
    laprint ("\\end{document}\n");
}

void latexCompile ()
{
    char cmd[MAXCMDSIZE] = "";
    sprintf (cmd, "pdflatex %s", LatexFileName);
    printf ("%s\n", cmd);
    system (cmd);
}
#undef laprint

int main (int argc, char* argv[])
{
    Tree tree = {};
    treeCtor (&tree);

    FILE* DBFileptr = fopen ("DBFile.txt", "r");
    tree.root = treeParse (tree.root, DBFileptr);
    tree.root = tree.root->left;

    
    treeDump (&tree, "Graphic dump, called by graphic mode\n");

    Tree tree1 = {}; 
    treeCtor (&tree1); 

    if (argc > 1)
    {
        fprintf (stderr, "sanya daun\n");
        for (size_t index = 0; index < ((char) *(argv[1])) - 48; ++index)
        {
            tree1.root = diff (tree.root);
            tree.root  = tree1.root;
        }
    }
    else
    {
        tree1.root = diff (tree.root);
    }

    treeDump (&tree1, "hey\n");
    fclose (DBFileptr);

    FILE* overleaf = fopen (LatexFileName, "w");
    latexBegin (overleaf);
    assert (overleaf != nullptr);

    fprintf (overleaf, "$");
    treePrint (tree1.root, 0, overleaf);
    fprintf (overleaf, "$\n");

    latexEnd (overleaf);
    fclose (overleaf);

    latexCompile ();
}
