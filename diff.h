#include <stdlib.h>

const char   GraphFile[20] = "GraphFile.txt";
const size_t MAXDATASIZE   = 10;
const size_t MAXCMPSIZE    = 100;
const char SPEAKFILE[40]   = "speakFile.txt";
const char* ShortCMD       = "+-k*/^csln";

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
    char*  varValue;
    Node*  left;
    Node*  right;
};

struct Tree
{
    Node*  root;
    size_t size;
    int    status;
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

int   findInTree       (Node* node,       const char* dataToFind);
void  treePrint        (const Node* node);
Node* treeParse        (Node* node,       FILE* DBFileptr);

