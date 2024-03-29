#include <cstdio>
#include <climits>
#include <cmath>
#include "FASTQReader.hpp"

FASTQReader::FASTQReader() : FASTAReader() {
    endOfReadDelim = '\n';
}

long FASTQReader::GetNext(FASTASequence &seq) {
    return ((FASTAReader*)this)->GetNext(seq);
}

unsigned char FASTQReader::phredQVtoPacbioQV(unsigned char phredQV){
    int qual = floor(100.0 * log10(pow(10.0, phredQV/10.0) - 1.0) + 0.5); 
    qual = qual > 250 ? 250 : qual;
    qual = qual < 1   ? 1   : qual;
    return (unsigned char) qual;
}

int FASTQReader::GetNext(FASTQSequence &seq) {
    char c;
    while( curPos < fileSize and ( (c = filePtr[curPos]) == ' ' or c == '\t' or c == '\n' or c == '\r') ) {
        curPos++;
    }

    if (curPos >= fileSize) {
        return false;
    }
    long p = curPos;
    AdvanceToTitleStart(p, '@');
    CheckValidTitleStart(p,'@');
    ReadTitle(p, seq.title, seq.titleLength);
    // Title ends on '\n', consume that;
    p++;
    long p2;
    p2 = p;
    while(p2 < fileSize and filePtr[p2] != '\n') { p2++;}
    if (p2 - p > UINT_MAX) {
        cout << "ERROR! Reading sequences stored in more than 4Gbytes of space is not supported." << endl;
        exit(1);
    }

    seq.length = p2 - p;
    long seqPos;
    if (seq.length > 0) {
        seq.seq = new Nucleotide[seq.length];
        p2 = p;
        seqPos = 0;
        while(p2 < fileSize and filePtr[p2] != '\n') { seq.seq[seqPos] = filePtr[p2]; p2++; seqPos++;}
    }
    else {
        seq.seq = 0;
    }
    p = p2;

    AdvanceToTitleStart(p,'+');
    CheckValidTitleStart(p,'+');
    while(p < fileSize and filePtr[p] != '\n') { p++;}
    p++; // skip '\n'
    p2 = p;
    while(p2 < fileSize and filePtr[p2] != '\n') { p2++;}
    seq.length = p2 - p;
    if (seq.length > 0) {
        seq.qual.Allocate(seq.length);
        p2 = p;
        long seqPos = 0;
        while(p2 < fileSize and filePtr[p2] != '\n') { 
            seq.qual[seqPos] = filePtr[p2] - FASTQSequence::charToQuality; //phredQVtoPacbioQV(filePtr[p2] - charToQuality); 
            p2++; seqPos++;
        }
    }
    else {
        seq.qual.data = NULL;
    }
    curPos = p2;
    seq.deleteOnExit = true;
    return true;
}



int FASTQReader::Advance(int nSteps) {
    // An advance of a FASTQ file is simply twice the number of
    // advances of FASTA, since each nucleotide sequence has a quality
    // sequence. 
    return ((FASTAReader*)this)->Advance(nSteps*2);
}
