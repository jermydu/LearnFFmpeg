#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include "FlvParser.h"

using namespace std;

void Process(fstream &fin, const char *filename);

int main(int argc, char *argv[])
{
    const char* defaultFileName = "../../../../testvideo/testvideo1_960x360.flv";
    const char* defaultOutFile = "../../../../testvideo/testvideo1_960x360_parser.flv";

    fstream fin;
    fin.open(defaultFileName, ios_base::in | ios_base::binary);
    if (!fin)
        return 0;

    Process(fin, defaultOutFile);

    fin.close();

    return 1;
}

void Process(fstream &fin, const char *filename)
{
    CFlvParser parser;

    int nBufSize = 2*1024 * 1024;
    int nFlvPos = 0;
    uint8_t *pBuf, *pBak;
    pBuf = new uint8_t[nBufSize];
    pBak = new uint8_t[nBufSize];

    while (1)
    {
        int nReadNum = 0;
        int nUsedLen = 0;
        fin.read((char *)pBuf + nFlvPos, nBufSize - nFlvPos);
        nReadNum = fin.gcount();
        if (nReadNum == 0)
            break;
        nFlvPos += nReadNum;

        parser.Parse(pBuf, nFlvPos, nUsedLen);
        if (nFlvPos != nUsedLen)
        {
            memcpy(pBak, pBuf + nUsedLen, nFlvPos - nUsedLen);
            memcpy(pBuf, pBak, nFlvPos - nUsedLen);
        }
        nFlvPos -= nUsedLen;
    }
    parser.PrintInfo();
    parser.DumpH264("parser.264");
    parser.DumpAAC("parser.aac");

    //dump into flv
    parser.DumpFlv(filename);

    delete []pBak;
    delete []pBuf;
}
