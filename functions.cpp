#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>

#include "functions.h"

size_t fileSize (FILE* file)
{
    fseek(file, 0l, SEEK_END); 
    size_t size = ftell(file); 
    fseek(file, 0l, SEEK_SET);

    return size;
}

int readFile(FILE* openedFile, char** dest)
{
    ASSERT(openedFile != nullptr);

    size_t numberOfChars = fileSize(openedFile);

//    printf("filesize = %lu\n", numberOfChars);

    *dest = (char*) calloc(numberOfChars + 1, sizeof(char));

    size_t charsReaded = fread((void*) *dest, sizeof(char), numberOfChars, openedFile); // 
    if (charsReaded != numberOfChars)
        return readingError;
    
    return 0;
}

int textToStr(char* text, size_t* numberOfLines)
{
    ASSERT(text != nullptr);

    size_t i = 0; 
    for (; text[i] != '\0'; i++) 
    {
        if (text[i] == '\n')
        {
            *numberOfLines += 1;
            text[i] = '\0';
        }
    }
    
    text[i] = '\n';
//    fprintf(stderr, "number of lines = %lu\n", *numberOfLines);

    return 0;
}

int splitIntoLines(InputFile *inputFile)
{
    ASSERT(inputFile != nullptr);
   
    char *text           =  inputFile->text;
    size_t numberOfLines =  inputFile->numberOfLines;
    Line **Lines         = &inputFile->arrayOfLines;

    ASSERT(text != NULL);

    *Lines = (Line*) calloc(numberOfLines, sizeof(Line));
    Line* arrayOfLines = *Lines;
    ASSERT(arrayOfLines != NULL);
    
    size_t line = 0;

//    fprintf(stderr, "%s", ((inputFile->arrayOfLines)[line]).charArray);
//    fprintf(stderr, "%s\n", ((arrayOfLines)[line]).charArray);
    arrayOfLines[line].charArray = text;
//    fprintf(stderr, "%s", ((inputFile->arrayOfLines)[line]).charArray);

    line++;

    size_t i = 0;
    for (; text[i + 1] != '\n'; i++)
    {
        if (text[i] == '\0')
        {
//           fprintf(stderr, "line number is %lu\n", line);
           
           arrayOfLines[line - 1].length    = &(text)[i] - arrayOfLines[line - 1].charArray;
           arrayOfLines[line]    .charArray = &(text)[i + 1];
           
        // fprintf(stderr, "%s\n", ((arrayOfLines)[line]).charArray);           
 //          fprintf(stderr, "length = %lu\n", arrayOfLines[line - 1].length);
           
           line++;
        }
    }

    arrayOfLines[line - 1].length = &text[i] - arrayOfLines[line - 1].charArray;
   
//    fprintf(stderr, "line number is %lu\n", line);
    
    return 0;
}

int readFileToLinesStruct(FILE* openedFile, InputFile* inputFile)
{
    ASSERT(openedFile != nullptr);
    
    readFile(openedFile, &(inputFile->text));
    textToStr(inputFile->text, &(inputFile->numberOfLines));
    splitIntoLines(inputFile);
    
    return 0;
}

//Status printArrayInFile(FILE* fileToPrint, InputFile* inputFile)
//{
//    if (!fileToPrint)
//        return printError;
//    // fseek(fileToPrint, 0l, SEEK_SET);
//
//    for (size_t i = 0; i < inputFile->numberOfLines; ++i)
//        fprintf(fileToPrint, "%s\n", ((inputFile->arrayOfLines)[i]).charArray);
//
//    return noErrors;
//}
//
//Status printOrig(FILE* fileToPrint, char* buf)
//{
//    size_t i = 0;
//    for (;buf[i] != '\n'; i++) 
//    {
//        if (buf[i] == '\0') 
//        {
//            buf[i] = '\n';
//        }
//    }
//
//    buf[i] = '\0';
//    fprintf(fileToPrint, "%s", buf);
//    return noErrors;
//}
//
