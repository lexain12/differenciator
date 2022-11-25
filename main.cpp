#include <stdio.h>
#include <stdarg.h>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <math.h>
#include <stdlib.h>
#include <time.h>

extern FILE* LOGFILEPTR;
typedef char* Elem_t;
#include "diff.h"
#include "functions.h"
#include "Analyzator/main.h"

const char   GraphFile[20] = "GraphFile.txt";
const size_t MAXDATASIZE   = 10;
const size_t MAXCMDSIZE    = 100;
const char SPEAKFILE[40]   = "speakFile.txt";
const char* ShortCMD       = "+-k*/^csln()";
const double EPS           = 10e-7;
const double VarPoison     = 10e7;

const size_t VarTableSize  = 10;
const char* LatexFileName  = "latex.tex";

const size_t numOfFillers = 6;

const char* fillers[] = 
{
    "А теперь давайте посмеемся над этим крутым мемом:",
    "Реал жиза:",
    "Вот этот мем мне очень понравился",
    "А вот еще один норм мем",
    "Нетрудно догадаться, что:",
    "Легко видеть, что в примере:",
    "Как известно, производная этой функции равняется:",
    "Поэтому:",
    "Заметим, что:",
    "Доказать, что это верно, оставим как домашнее задание интересующемуся читателю:"
};

Var VarTable[VarTableSize] = {};

void varTablePoison ()
{
    for (size_t index = 0; index < VarTableSize; ++index)
        VarTable[index].varValue = VarPoison;
}

Node* nodeCtor ()
{
    Node* newDataPtr = (Node*) calloc (1, sizeof(*newDataPtr));
    assert (newDataPtr != nullptr);

    newDataPtr->type     = Unknown;
    newDataPtr->opValue  = UnknownOp;
    newDataPtr->numValue = 0;
    newDataPtr->varName  = nullptr;
    newDataPtr->left     = nullptr;
    newDataPtr->right    = nullptr;

    return newDataPtr;
}

void nodeDtor (Node* node)
{
    if (!node) return;

    node->type     = Unknown;
    node->opValue  = UnknownOp;
    node->numValue = 0;
    node->varName  = nullptr;
    node->left     = nullptr;
    node->right    = nullptr;

    free (node);
}

Node* createNode (Type type, OP opValue, double numValue, char* varName, Node* left, Node* right)
{
    Node* newNode = nodeCtor ();

    newNode->type         = type;
    newNode->left         = left;
    newNode->right        = right;
    newNode->numValue     = numValue;
    newNode->varName      = varName;
    newNode->opValue      = opValue;
    
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

void tableInsert (char* varName)
{
    for (size_t index = 0; index < VarTableSize; ++index)
    {
        if (VarTable[index].varName != nullptr)
        {
            if (strcmp (VarTable[index].varName, varName) == 0)
                return;
        }
        else 
        {
            VarTable[index].varName = varName;
            return;
        }
    }
}

void parseNodeData (Node* curNode, FILE* DBFileptr)
{
    assert (curNode   != nullptr);
    assert (DBFileptr != nullptr);

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
            free (data);
        }

        else if (strchr (data, '/'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_DIV;
            free (data);
        }

        else if (strchr (data, '+'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_ADD;
            free (data);
        }

        else if (strchr (data, '-'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_SUB;
            free (data);
        }

        else if (strchr (data, '^'))
        {
            curNode->type = OP_t;
            curNode->opValue = OP_POW;
            free (data);
        }

        else if (strcmp (data, "sin") == 0)
        {
            curNode->type = OP_t;
            curNode->opValue = OP_SIN;
            free (data);
        }

        else if (strcmp (data, "cos") ==0)
        {
            curNode->type = OP_t;
            curNode->opValue = OP_COS;
            free (data);
        }

        else 
        {
            curNode->type    = Var_t;
            curNode->varName = data;
            tableInsert (data); 
        }
    }
}

Node* treeParse (Node* node, FILE* DBFileptr)
{
    assert (node      != nullptr);
    assert (DBFileptr != nullptr);

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
    assert (tree         != nullptr);
    assert (tree->root   != nullptr);
    assert (GraphFilePtr != nullptr);

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
                dumpprint ("%s }", tree->root->varName);
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
    assert (node         != nullptr);
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
                dumpprint ("%s }", node->left->varName);
                break;

            case Unknown:
                dumpprint ("Unknown }");
                break;

            default:
                fprintf (stderr, "%d\n", node->left->type);
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
                dumpprint ("%s }", node->right->varName);
                break;

            case Unknown:
                dumpprint ("Unknown }");
                break;

            default:
                fprintf (stderr, "%d\n", node->right->type);
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
    assert (node != nullptr);

    Node* newNode = nodeCtor();
    newNode->type = node->type;
    newNode->opValue = node->opValue;
    newNode->numValue = node->numValue;
    newNode->varName = node->varName;

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
    assert (childNode != nullptr);
    assert (node      != nullptr);

    int isPrint = 0;

    if (childNode->type == OP_t && node->type == OP_t)
    {
        if (node->opValue - childNode->opValue > 1 || fabs (fabs (node->opValue - childNode->opValue) - 1) < EPS)
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
    assert (node        != nullptr);
    assert (operation   != nullptr);
    assert (fileToPrint != nullptr);

    int isPrintL = 0;
    int isPrintR = 0;

//    Node* comfortableRight = node->right;
//    Node* comfortableLeft  = node->left;
//    if (findInTree (node->left, "x") && node->opValue == OP_MUL) 
//    {
//        comfortableRight = node->left;
//        comfortableLeft  = node->right;
//    }

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

                case UnknownOp:
                    fprintf (fileToPrint, "UnknownOP\n");
                    break;

                default:
                    assert (0);
            }
            break;
        
        case Var_t:
            fprintf (fileToPrint, "%s", node->varName);
            break;
        
        case Num_t:
            fprintf (fileToPrint, "%lg", node->numValue);
            break;

        case Unknown:
            fprintf (fileToPrint, "Unknown");
            break;
        
        default:
            assert (0);
    }
            

    if (isPrint)
        fprintf (fileToPrint, ")");

    return;
}

int findInTree (Node* node, const char* dataToFind)
{
    if (!node) return 0;

    assert (dataToFind != nullptr);

    if (node->type == Var_t && strcmp(node->varName, dataToFind) == 0)
        return 1;

    else if (node->left == nullptr && node->right == nullptr)
        return 0;

    if (node->left)
    {
        if (findInTree (node->left, dataToFind))
            return 1;
        
        else 
            return 0;
    }

    if (node->right)
    {
        if (findInTree (node->right, dataToFind))
            return 1;

        else
            return 0;
    }
        
    return 0;
}

#undef dumpprint

#define createNum(NUM) createNode(Num_t, UnknownOp, NUM, nullptr, nullptr, nullptr)
#define dL diff(node->left, varName)
#define dR diff(node->right, varName)
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

Node* diff (const Node* node, char* varName)
{
    assert (node != nullptr);

    switch (node->type)
    {
        case Num_t:
            {
            return createNum (0);
            break;
            }

        case Var_t:
            {
                if (strcmp (varName, node->varName) == 0)
                    return createNum (1);
                else 
                    return createNum (0);

            break;
            }

        case Unknown:
            {
                fprintf (stderr, "Unknown type of node\n");
                return nullptr;
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
                    if (findInTree (node->right, varName) && findInTree(node->left, varName))
                    {
                        return MUL (POW (cL, cR), diff (MUL (LN (cL), cR), varName));
                    }
                    else if (findInTree (node->right, varName))
                    {
                        return MUL (LN (cL), MUL (POW (cL, cR), dR));
                    }
                    else if (findInTree (node->left, varName))
                    {
                        return MUL (MUL (dL, cR), POW (cL, SUB (cR, createNum(1))));
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

                case UnknownOp:
                    fprintf (stderr, "UnknownOP in tree\n");
                    return nullptr;

                default:
                    assert (0);
            }

            break;

        default:
            assert (0);
    }

    return nullptr;
}

int nodeEquals (const Node* node, const double val)
{
    assert (node != nullptr);

    if (node->type != Num_t) return 0;

    if (fabs (node->numValue - val) < EPS)
        return 1;
    else
        return 0;
}

#define OP_is(op) (node->type == OP_t && node->opValue == OP_##op)
#define IS_VAL(side, val) (nodeEquals (node->side, val))

Node* makeEasier (Node* node, int* isChanged)
{
    assert (node != nullptr);

    if (node->right)
        node->right = makeEasier (node->right, isChanged);

    if (node->left)
        node->left  = makeEasier (node->left, isChanged);

    if (OP_is (MUL) && (IS_VAL (right, 0) ||  IS_VAL (left, 0)))
    {
        nodeDtor (node->right);
        nodeDtor (node->left);
        node->left = nullptr;
        node->right = nullptr;
        *isChanged = 1;
        return createNum (0);
    }

    else if ((OP_is (MUL) || OP_is (POW) || OP_is (DIV)) && IS_VAL (right, 1))
    {
        nodeDtor (node->right);
        node->right = nullptr;
        *isChanged = 1;
        return cL; 
    }

    else if ((OP_is (MUL) || OP_is (POW)) && IS_VAL (left, 1))
    {
        nodeDtor (node->left);
        node->left = nullptr;
        *isChanged = 1;
        return cR;
    }

    else if (OP_is (POW) && (IS_VAL (left, 0) || IS_VAL (right, 0)))
    {
        nodeDtor (node->right);
        nodeDtor (node->left);
        node->left = nullptr;
        node->right = nullptr;
        *isChanged = 1;
        return createNum (1);
    }
    
    else if ((OP_is (ADD) || OP_is(SUB)) && IS_VAL (left, 0))
    {
        nodeDtor (node->left);
        node->left = nullptr;
        *isChanged = 1;
        return cR;
    }

    else if ((OP_is (ADD) || OP_is(SUB)) && IS_VAL (right, 0))
    {
        nodeDtor (node->right);
        node->right = nullptr;
        *isChanged = 1;
        return cL;
    }

    return treeCpy (node);
}

#define countOP(sign) (node->left->numValue sign node->right->numValue)

Node* countConstExpr (Node* node, int* isChanged)
{
    assert (node != nullptr);

    if (node->type == OP_t)
    {
        if (node->opValue <= OP_POW) // chtobi bilo dva chlena
        {
            if (node->right->type == Num_t && node->left->type == Num_t)
            {
                switch (node->opValue)
                {
                    case OP_ADD:
                        {
                            Node* rtn = createNum (countOP(+));
                            nodeDtor (node->right);
                            nodeDtor (node->left);
                            node->left = nullptr;
                            node->right = nullptr;

                            return rtn;
                        }

                    case OP_MUL:
                        {
                            Node* rtn = createNum (countOP(*));
                            nodeDtor (node->right);
                            nodeDtor (node->left);
                            node->left = nullptr;
                            node->right = nullptr;

                            return rtn;
                        }

                    case OP_SUB:
                        {
                            Node* rtn = createNum (countOP(-));
                            nodeDtor (node->right);
                            nodeDtor (node->left);
                            node->left = nullptr;
                            node->right = nullptr;
                            
                            return rtn;
                        }

                    case OP_DIV:
                        {
                            Node* rtn = createNum (countOP(/));
                            nodeDtor (node->right);
                            nodeDtor (node->left);
                            node->left = nullptr;
                            node->right = nullptr;

                            return rtn;
                        }

                    case OP_POW:
                        {
                            Node* rtn = createNum (pow(node->left->numValue, node->right->numValue));
                            nodeDtor (node->right);
                            nodeDtor (node->left);
                            node->left = nullptr;
                            node->right = nullptr;

                            return rtn;
                        }
                    
                    case UnknownOp:
                            assert (0);

                    default:
                            assert (0);
                }
                *isChanged = 1;
            }
        }

        else
        {
            if (node->right->type == Num_t)
            {
                switch (node->opValue)
                {
                    case OP_COS:
                        {
                            Node* rtn = createNum (cos (node->right->numValue));
                            nodeDtor (node->right);
                            node->right = nullptr;

                            return rtn;
                        }
                    
                    case OP_SIN:
                        {
                            Node* rtn = createNum (sin (node->right->numValue));
                            nodeDtor (node->right);
                            node->left = nullptr;
                            return rtn;
                        }
                }
                *isChanged = 1;
            }
        }
    }

    if (node->right)
        node->right = countConstExpr (node->right, isChanged);

    if (node->left)
        node->left = countConstExpr (node->left, isChanged);
    
    return treeCpy (node);
}
#undef countOP

Node* optimizeTree (Node* node)
{
    assert (node != nullptr);
    Node* rtnNode = nodeCtor (); 
    rtnNode = node;

    int isChanged        = 0;
    int noChangesCounter = 0;

    while (true)
    {
        isChanged = 0;

        rtnNode = countConstExpr (rtnNode, &isChanged);

        if (isChanged == 0)
            ++noChangesCounter;
        else
            noChangesCounter = 0;

        isChanged = 0;

        rtnNode = makeEasier (rtnNode, &isChanged);

        if (isChanged == 0)
            ++noChangesCounter;
        else
            noChangesCounter = 0;

        if (noChangesCounter > 50)
            return rtnNode;
    }
}



double findVar (const char* varName) {
    assert (varName  != nullptr);

    for (size_t index = 0; index < VarTableSize; ++index)
    {
        if (strcmp (VarTable[index].varName, varName) == 0)
        {
            return VarTable[index].varValue;
        }
    }

    return NAN;
}

void insertVar (Node* node)
{
    if (node->right == nullptr && node->left == nullptr && node->type == Num_t)
        return;

    if (node->type == Var_t)
    {
        node->type = Num_t;
        node->numValue = findVar (node->varName);
    }

    if (node->right)
        insertVar (node->right);

    if (node->left)
        insertVar (node->left);

    return;
}

void fillTable (Var* table, size_t size)
{
    for (size_t index = 0; index < size; ++index)
    {
        if (table[index].varName != nullptr )
        {
            double varValue = 0;
            printf ("Enter value of this var: %s\n", table[index].varName);
            fflush (stdin);
            scanf ("%lg", &varValue);

            table[index].varValue = varValue;
        }
    }
}

Node* countFunction (Node* node)
{
    insertVar (node);  

    return optimizeTree (node);
}

Node* countFunctionInPoint (Node* node)
{
    fillTable (VarTable, VarTableSize);
    countFunction (node);

    return optimizeTree (node);
}


void latexCompile ()
{
    char cmd[MAXCMDSIZE] = "";
    sprintf (cmd, "pdflatex %s", LatexFileName);
    system (cmd);
    sprintf (cmd, "open latex.pdf");
    system (cmd);
}

Node* countDerivative (Node* initialNode, size_t order, char* varName)
{
    Node* tmpNode = nodeCtor ();
    Node* rtnNode = treeCpy (initialNode);

    if (order == 0) 
        return rtnNode;
    else
    {
        rtnNode = diff (initialNode, varName);
        rtnNode = optimizeTree (rtnNode);
        tmpNode = rtnNode;
    }

    for (size_t index = 1; index < order; ++index)
    {
        rtnNode = diff (tmpNode, varName);
        rtnNode = optimizeTree (rtnNode);
        tmpNode = rtnNode;
    }

    return rtnNode;
}

int factorial (int num)
{
    if (num == 0)
        return 1;

    int rtn = 1;

    while (num != 0)
    {
        rtn *= num;
        --num;
    }

    return rtn;
}


void tableDump ()
{
    for (size_t index = 0; index < VarTableSize; ++index)
    {
        printf ("VarName: %s, varValue: %lf\n", VarTable[index].varName, VarTable[index].varValue);
    }
}

void changeVarTable (char* varName, double varValue)
{
    for (size_t index = 0; index < VarTableSize; ++index)
    {
        if (VarTable[index].varName == nullptr)
        {
            VarTable[index].varName = varName;
            VarTable[index].varValue = varValue;
            return;
        }

        if (strcmp (VarTable[index].varName, varName) == 0)
        {
            VarTable[index].varValue = varValue;
            return;
        }
    }
}

void drawPlot (double minX, double maxX, Node* function, char* varName, char* fileName)
{
    FILE* fileptr = fopen (fileName, "w");
    assert (fileptr != nullptr);

    fillTable (VarTable, VarTableSize);
    Node* tmpNode = nodeCtor ();

   for (double index = minX; index <= maxX;)
   {
       tmpNode = treeCpy (function);
       changeVarTable (varName, index); 

       tmpNode = countFunction (tmpNode);
       
       fprintf (fileptr, "%lf %lf\n", index, tmpNode->numValue);

       index += 1e-1;
   }

   fclose (fileptr);
   system ("python3 main.py");
}

void fullError (Node* function)
{
    double fullError         = 0;
    double partialDerivative = 0;
    double valError          = 0;

    Var errorTable[VarTableSize] = {};

    fillTable (VarTable, VarTableSize);

    for (size_t index = 0; index < VarTableSize; ++index)
    {
        errorTable[index].varName = VarTable[index].varName;
    }

    printf ("Enter error for next vars:\n");
    fillTable (errorTable, VarTableSize);

    for (size_t index = 0; index < VarTableSize; ++index)
    {
        if (VarTable[index].varName == nullptr) break;

        valError = errorTable[index].varValue;
        valError = valError * valError;

        partialDerivative = countFunction ( countDerivative (function, 1, VarTable[index].varName))->numValue;
        partialDerivative *= partialDerivative;

        fullError += partialDerivative * valError;
        printf ("%lf\n", valError);
    }
    printf ("fullError %lf\n", fullError);
}
#define laprint(...) fprintf (fileToPrint, __VA_ARGS__)
void latexBegin (FILE* fileToPrint)
{
    laprint ("\\documentclass{article}\n");
    laprint ("\\usepackage[utf8]{inputenc}\n");
    laprint ("\\usepackage{graphicx}\n");
    laprint ("\\title{11}\n");
    laprint ("\\author{Витя Тяжелков}\n");
    laprint ("\\date{November 2022}\n");
    laprint ("\\usepackage[russian]{babel}\n");
    laprint ("\\begin{document}\n");
    laprint ("Одним из основных понятий , изучаемых в курсе математического анализа, является понятие производной. Оно возникает в школьном курсе элементарной албебры. Короче я устал писать, го к делу. Кстати, так как это очень нужное пособие и его прочтет миллионы людей, то здесь могла бы быть ваша реклама.\n\n");
    laprint ("На самом деле понятие производной это очень легко, просто дифференцируем функцию и все, у нас есть производная. Давайте разберем это понятие на примере следующей функции:\n\n");
}

void McLaurenSeries (Node* function, size_t order, char* varName, FILE* fileToPrint)
{
    fprintf (fileToPrint, "McLaurenSeries for this function:\n\n");
    changeVarTable (varName, 0);
    tableDump ();

    fprintf (fileToPrint, "$");
    for (size_t index = 0; index <= order; ++index)
    {
        fprintf (fileToPrint, "x^{%d} \\cdot \\frac{", index);
        treePrint (countFunction (countDerivative (function, index, varName)), 0, fileToPrint);
        fprintf (fileToPrint, "}{");
        fprintf (fileToPrint, "%d} + ", factorial (index));
    }

    fprintf (fileToPrint, "o(x^{%d})", order);
    fprintf (fileToPrint, "$\n\n");

    laprint("\\begin{figure}[h]\n\n");

    laprint("\\centering\n\n");

    laprint ("\\includegraphics[width=0.8\\linewidth]{graph.png}\n\n");

    laprint ("\\caption{График функции}\n\n");

    laprint ("\\label{fig:mpr}\n\n");

    laprint ("\\end{figure}\n\n");
}

void latexEnd (FILE* fileToPrint)
{
    laprint ("\\end{document}\n");
}

void latexDerivative (Node* function, size_t order, FILE* fileToPrint)
{
    int number = 0;

    if (order >= 1)
    {
        laprint ("Давайте посчитаем первую производную:\n\n");
        laprint ("$");

        laprint ("(");
        treePrint (function, 0, fileToPrint);
        laprint (")^{(1)} = ");
        treePrint (countDerivative (function, 1, "x"), 0, fileToPrint);
        laprint ("$\n\n");
    }

    for (size_t index = 2; index <= order; ++index)
    {
        number = rand()%numOfFillers;

        if (number <= 3)
        {
            laprint (fillers[number]);

            laprint("\\begin{figure}[h]\n\n");

            laprint("\\centering\n\n");

            laprint ("\\includegraphics[width=0.2\\linewidth]{meme%d.jpeg}\n\n", number);

            laprint ("\\end{figure}\n\n");
            
            laprint ("АХАХАХАХА, смешно, а вот уже продиффернцировал:\n\n");
        }
        else
        {
            laprint (fillers[number]);
        }
        laprint ("\n\n");
        laprint ("$");
        laprint ("(");
        treePrint (function, 0, fileToPrint);
        laprint (")^{(%lu)} = ", index);

        treePrint (countDerivative (function, index, "x"), 0, fileToPrint);
        laprint ("$\n\n");
    }
     
}

#undef laprint

int main ()
{
    srand (time (NULL));
    varTablePoison();

    Tree tree = {};
    treeCtor (&tree);

    InputFile inputFile = {"DBFile.txt"}; 

    FILE* DBFileptr = fopen ("DBFile.txt", "r");
    readFileToLinesStruct (DBFileptr, &inputFile);

    printf ("%s", inputFile.arrayOfLines[0].charArray);

    Node** tokenArray = (Node**) calloc (inputFile.arrayOfLines[0].length, sizeof(*tokenArray));

    getTokens (tokenArray, inputFile.arrayOfLines[0].charArray);

    for (int index = 0; tokenArray[index]->type != Unknown; ++index)
    {
        tree.root = tokenArray[index];
        treeDump (&tree, "HEY\n");
    }

    return 0;
    tree.root = treeParse (tree.root, DBFileptr);
    tree.root = tree.root->left;

    Tree tree1 = {}; 
    treeCtor (&tree1); 

    printf ("Enter n for differnce the func n times\n");
    size_t numberOfDiff = 0;
    scanf ("%lu", &numberOfDiff);

    //tree1.root = getG ((const char**) &(inputFile.arrayOfLines[0].charArray));
    fprintf (stderr, "hello\n");
    treeDump (&tree1, "Hello\n");
    return 0;

    tree1.root = countDerivative (tree.root, numberOfDiff, "x");

    fclose (DBFileptr);

    fullError (tree.root);

    drawPlot (-5, 5, tree.root, "x", "plot.txt");

    FILE* overleaf = fopen (LatexFileName, "w");
    latexBegin (overleaf);
    fprintf (overleaf, "$f(x) = ");
    treePrint (tree.root, 0, overleaf);
    fprintf (overleaf, "$\n\n");
    assert (overleaf != nullptr);


    fprintf (overleaf, "$");
    treePrint (tree1.root, 0, overleaf);
    fprintf (overleaf, "$\n\n очевидно, что это равняется: \n\n");

    fillTable (VarTable, VarTableSize);
    tree1.root = countFunction (tree1.root);

    fprintf (overleaf, "$");
    treePrint (tree1.root, 0, overleaf);
    fprintf (overleaf, "$\n\n");

    latexDerivative (tree.root, numberOfDiff, overleaf);

    McLaurenSeries (tree.root, numberOfDiff, "x", overleaf);

    fprintf (overleaf, "\n\n");
    

    latexEnd (overleaf);
    fclose (overleaf);

    latexCompile ();
}
