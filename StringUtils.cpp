#include <string>
#include <stdint.h>
#include <vector>
#include <sstream>
#include "Types.h"
#include "MD5Utils.hpp"
#include "StringUtils.hpp"

int ExactPatternMatch(string orig, string pattern) {
    string::size_type pos = orig.find(pattern);
    if (pos == orig.npos) {
        return 0;
    }
    else {
        return 1;
    }
}

void MakeMD5(const char *data, unsigned int dataLength, string &md5Str, int nChars) {

    MD5 md5engine;
    md5engine.update((unsigned char*) data, dataLength);
    md5engine.finalize();

    char *md5c_str = md5engine.hex_digest();
    assert(md5c_str != NULL);
    if (nChars == 0) {
        nChars = 32;
    }
    md5Str.assign(md5c_str, nChars);
    delete[] md5c_str;
}


void MakeMD5(string &data, string &md5Str, int nChars) {
    MakeMD5(data.c_str(), data.size(), md5Str, nChars);
}


int IsWhitespace(char c) {
    return (c == ' ' or c == '\t' or c == '\n' or c == '\r' or c == '\0');
}

int IsSpace(char c) {
    return (c == ' ' or c == '\t');
}

int ToWords(string &orig, vector<string> &words) {
    int curWordStart, curWordEnd;
    curWordStart = 0;
    while(curWordStart < orig.size()) {
        while (curWordStart < orig.size() and IsSpace(orig[curWordStart])) { curWordStart++; }
        curWordEnd = curWordStart;
        while (curWordEnd < orig.size() and !IsSpace(orig[curWordEnd])) { curWordEnd++; }
        string word;
        if (curWordEnd != curWordStart) {
            word.assign(orig, curWordStart, curWordEnd - curWordStart);
            words.push_back(word);
        }
        curWordStart = curWordEnd;
    }
    return words.size();
}

int Tokenize(string orig, string pattern, vector<string> &tokens) {
    VectorIndex tokenStart, tokenEnd;
    int patternLength = pattern.size();
    int origLength    = orig.size();
    if (origLength == 0) {
        return 0;
    }
    bool prevWasToken = false;
    tokenEnd = 0;
    tokenStart = 0;
    //for (tokenEnd = 0; tokenEnd < origLength-patternLength; tokenEnd) {
    while(tokenEnd < origLength - patternLength) {
        while (tokenStart < origLength - patternLength and 
                orig.compare(tokenStart, patternLength, pattern, 0, patternLength) == 0) {
            tokenStart++;
        }
        tokenEnd = tokenStart + 1;

        prevWasToken = false;
        while (tokenEnd < origLength - patternLength) {
            if (orig.compare(tokenEnd, patternLength, pattern, 0, patternLength) == 0) {
                // add this token to the vector of tokens
                if (tokenEnd - tokenStart >= 1) {
                    prevWasToken = true;
                    tokens.push_back(orig.substr(tokenStart, tokenEnd - tokenStart));
                }
                tokenEnd+=patternLength;
                tokenStart = tokenEnd;
                break;
            }
            else {
                prevWasToken = false;
                ++tokenEnd;
            }
        }
    }
    if (tokenEnd - tokenStart > 1) {
        tokens.push_back(orig.substr(tokenStart, tokenEnd - tokenStart+1));
    }
    return tokens.size();
}


void ParseSeparatedList(const string &csl, vector<string> &values, char delim) {
    stringstream cslStrm(csl);
    string valString;
    string next;
    do {
        if (getline(cslStrm, valString, delim)) {
            if (valString.size() > 0) {
                values.push_back(valString);
            }
        }
    }
    while (cslStrm);
}

int AssignUntilFirstSpace(char *orig, int origLength, string &result) {
    int i;
    for (i = 0; i < origLength; i++ ){ 
        if (orig[i] == ' ' or orig[i] == '\t' or orig[i] == '\n' or orig[i] == '\r' or orig[i] == '\0') {
            break;
        }
    }
    result.assign(orig, i);
    return i;
}

string RStrip(string & fileName) {
    // Remove right-ended spaces
    int i = fileName.size();
    if (i == 0) {
        return "";
    }
    while (i >= 1) {
        i--;
        if (not IsWhitespace(fileName[i])) {
            break;
        }
    }
    return fileName.substr(0, i + 1);
}
