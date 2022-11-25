#pragma once
#include <stdlib.h>

extern const char   GraphFile[20];
extern const size_t MAXDATASIZE;
extern const size_t MAXCMDSIZE;
extern const char SPEAKFILE[40];
extern const char* ShortCMD;
extern const double EPS;
extern const double VarPoison;

extern const size_t VarTableSize;
extern const char* LatexFileName;

extern const size_t numOfFillers;
#define RAND_MAX numOfFillers
extern const char* fillers[];

enum Errors
{
    noErrors     = 0,
    treePtrError = 1 << 0,
};

enum OP
{
    UnknownOp = 0,
    OP_ADD    = 1,
    OP_SUB    = 2,
    OP_MUL    = 4,
    OP_DIV    = 5,
    OP_POW    = 6,
    OP_COS    = 7,
    OP_SIN    = 8,
    OP_LOG    = 9,
    OP_LN     = 10,
    OP_LBR    = 11,
    OP_RBR    = 12,
};

enum Type
{
    Unknown = 0,
    OP_t    = 1,
    Var_t   = 2,
    Num_t   = 3,
};

struct Node 
{
    Type   type;
    OP     opValue;
    double numValue;
    char*  varName;
    Node*  left;
    Node*  right;
};

struct Tree
{
    Node*  root;
    size_t size;
    int    status;
};

struct Var 
{
    char* varName;
    double varValue; 
};

Node* nodeCtor ();
Node* createNode (Type type, OP opValue, double num, char* varValue, Node* left, Node* right);

int treeCtor (Tree* tree);

void  treePrint        (const Node* node, int isPrint,           FILE* const fileToPrint);
void  makeGraph        (Tree* tree);
void  treeGraph        (const Node* node, FILE* GraphFilePtr);

void  inputCleaning();
int   compareOperation (const Node* node, const Node* childNode);
void  printOperation   (const Node* node, const char* operation, FILE* const fileToPrint);
void  parseNodeData    (Node* curNode,    FILE* DBFileptr);
Node* treeCpy          (const Node* node);
Node* diff             (const Node* node);

void  treeDump         (Tree* tree,       const char* str, ...);
void changeVarTable (char* varName, double varValue);
void tableDump ();

int   findInTree       (Node* node,       const char* dataToFind);
void  treePrint        (const Node* node);
Node* treeParse        (Node* node,       FILE* DBFileptr);
void latexBegin        (FILE* fileToPrint);
void latexEnd          (FILE* fileToPrint);

//Node* getN (const char** str);
//Node* getP (const char** str);
//Node* getG (const char** str);
//Node* getE (const char** str);
//Node* getT (const char** str);
//Node* getPW (const char** str);
//Node* getUnOP (const char** str);
int strEqual(const char *l, const char *r);

void getOpOrVarToken (Node*** tokenArray, char** string);
void getTokens (Node** tokenArray, char* string);
void getBOpToken (Node*** tokenArray, char** string);
void skipSpaces (char** string);
