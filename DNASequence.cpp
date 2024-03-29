#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include "Types.h"
#include "DNASequence.hpp"

DNALength DNASequence::size() {
    return length;
}

void DNASequence::TakeOwnership(DNASequence &rhs) {
    if (deleteOnExit) {
        if (seq != NULL) {
            delete[] seq;
        }
    }
    seq = rhs.seq;
    length = rhs.length;
    deleteOnExit = rhs.deleteOnExit;
}

void DNASequence::Append(const DNASequence &rhs, DNALength appendPos) {
    //
    // Simply append rhs to this seuqence, unless appendPos is nonzero
    // in which case rhs is inserted at attendPos, overwriting this
    // sequence from appendPos to the end.
    //
    Nucleotide *newSeq;
    //
    // Handle the two cases (appendPos == 0 and appendPos > 0)
    // separately in order to handle memory deallocation correctly.
    //
    if (appendPos == 0) {
        DNALength  newSeqLength = length + rhs.length;
        newSeq = new Nucleotide[newSeqLength];
        memcpy(newSeq, seq, length);
        memcpy(&newSeq[length], rhs.seq, rhs.length);

        if (length != 0) {
            delete[] seq;
        }
        seq = newSeq;
        length = newSeqLength;
        deleteOnExit = true;
    }
    else {
        if (appendPos + rhs.length < length) {
            memcpy(&seq[appendPos], rhs.seq, rhs.length);
            length = appendPos + rhs.length;
        }
        else {
            DNALength lengthCopy = length;
            length = appendPos;
            DNALength newSeqLength;
            newSeqLength = length + rhs.length;
            newSeq = new Nucleotide[newSeqLength];
            memcpy(newSeq, seq, length);
            memcpy(&newSeq[length], rhs.seq, rhs.length);
            if (deleteOnExit and lengthCopy != 0) {
                delete[] seq;
            }
            seq = newSeq;
            length = newSeqLength;
            deleteOnExit = true;
        }
    }
}

// Copie FROM rhs to this DNASequence. 
DNASequence& DNASequence::Copy(const DNASequence &rhs, DNALength rhsPos, DNALength rhsLength) {
    if (length != 0) {
        if (seq != NULL)
            delete[] seq;
        seq = NULL;
        length = 0;
    }

    //
    // When initializing a vector of DNASequence's, the copy
    // constructor will initialze a list and call this
    // function with a zero-length DNASequence as the rhs to
    // initialize every element in the vector   The check
    // below will fail on zero-length sequences, so add a boundary
    // condition check before that to allow the copy-constructor to
    // work.
    //
    if (rhs.length == 0) {
        return *this;
    }

    //
    // Silently ignoring this case could lead to problems later on,
    // catastrophically assert here if the input is not valid.
    // In case rhsLength + rhsPos > ULONG_MAX (4294967295), check 
    // both rhsLength and rhsPos, fix bug 21794
    //

    if (not (rhsLength <= rhs.length     && 
                rhsPos    <= rhs.length + 1 &&
                rhsLength + rhsPos <= rhs.length + 2 )) {
        std::cout << "ERROR.  The subsequence to copy is out of bounds." 
            << std::endl
            << "        Failed to copy a subsequence starting at " << rhsPos 
            << std::endl
            << "        with length "<< rhsLength 
            << " from a sequence of length " << rhs.length << "." 
            << std::endl;
        exit(1);
    }

    if (rhsLength == 0) {
        rhsLength = rhs.length - rhsPos;
    }
    if (rhsLength == 0) {
        seq = NULL;
    }
    else {
        seq = new Nucleotide [rhsLength];
        memcpy(seq, &rhs.seq[rhsPos], rhsLength);
    }
    length = rhsLength;
    deleteOnExit = true;
    return *this;
}

void DNASequence::ShallowCopy(const DNASequence &rhs) {
    seq = rhs.seq;
    length = rhs.length;
    deleteOnExit = false;
}

int DNASequence::GetStorageSize() {
    return (length * sizeof(Nucleotide));
}

DNASequence &DNASequence::operator=(const DNASequence &rhs){ 
    Copy(rhs);
    return *this;
}

//
// synonym for printseq
//
void DNASequence::Print(std::ostream &out, int lineLength) {
    PrintSeq(out, lineLength);
}

void DNASequence::PrintSeq(std::ostream &out, int lineLength) {
    if (lineLength == 0) {
        std::string line;
        line.assign((char*)seq, length);
        out << line;
    }
    else {
        //
        // Make sure this isn't 
        assert(lineLength > 0);
        DNALength curPos = 0;
        int curLineLength = lineLength;
        while (curPos < length) {
            if (curPos + curLineLength > length) {
                curLineLength = length - curPos;
            }
            std::string line;
            line.assign((char*) &seq[curPos], curLineLength);
            out << line << std::endl;
            curPos += curLineLength;
        }
    }
}

void DNASequence::Allocate(DNALength plength) {
    if (seq != NULL) {
        delete[] seq;
    }
    seq = new Nucleotide [plength];
    length = plength;
    deleteOnExit = true;
}

void DNASequence::ReferenceSubstring(const DNASequence &rhs, DNALength pos, int substrLength) {
    //
    // This makes a reference therefore it should not be deleted.
    //
    assert(pos >= 0 && pos <= rhs.length &&
            substrLength >= 0 && substrLength <= rhs.length);
    if (substrLength == 0) {
        substrLength = rhs.length - pos;
    }
    assert(pos + substrLength <= rhs.length);
    seq = &rhs.seq[pos];
    length = substrLength;
    deleteOnExit = false;
}

DNALength DNASequence::MakeRCCoordinate(DNALength forPos ) {
    return length - forPos - 1;
}


void DNASequence::MakeRC(DNASequence &rc, DNALength pos, DNALength rcLength) {
    if (rcLength == 0) {
        rcLength = length - pos;
    }

    rc.Allocate(rcLength);
    DNALength i;
    for (i = 0; i < rcLength; i++) {
        rc.seq[rcLength - i - 1] = ReverseComplementNuc[seq[i+pos]];
    }
    rc.length = rcLength;
    rc.deleteOnExit = true;
}

void DNASequence::ToTwoBit() {
    DNALength i;
    for (i = 0; i < length; i++) {
        seq[i] = TwoBit[seq[i]];
    }
    bitsPerNuc = 2;
}

void DNASequence::ToFourBit() {
    DNALength i;
    if (bitsPerNuc != 4) 
        for (i = 0; i < length; i++) { seq[i] = FourBit[seq[i]]; }
    bitsPerNuc = 4;
}

void DNASequence::ConvertThreeBitToAscii() {
    DNALength i;
    for (i = 0; i < length; i++ ){
        seq[i] = ThreeBitToAscii[seq[i]];
    }
}

void DNASequence::ToAscii() {
    DNALength i;
    if (bitsPerNuc != 8) {
        for (i = 0; i < length; i++ ){ 
            seq[i] = FourBitToAscii[seq[i]];
        }
        bitsPerNuc = 8;
    }
}

void DNASequence::Assign(DNASequence &ref, DNALength start, DNALength plength) {
    if (seq != NULL) {
        delete[] seq;
        seq = NULL;
        length = 0;
    }
    if (plength) {
        length = plength;
        seq = new Nucleotide[length];
        memcpy(seq, &ref.seq[start], length);
    }
    else if (start) {
        length = ref.length - start;
        seq = new Nucleotide[length];
        memcpy(seq, &ref.seq[start], length);
    }
    else {
        this->Copy(ref);
    }
    deleteOnExit = true;
}

void DNASequence::ToLower() {
    DNALength i;
    for (i = 0; i < length; i++) {
        seq[i] = AllToLower[seq[i]];
    }
}

void DNASequence::ToUpper() {
    DNALength i;
    for (i = 0; i < length; i++) {
        seq[i] = AllToUpper[seq[i]];
    }
}

void DNASequence::Concatenate(const Nucleotide *moreSeq, DNALength moreSeqLength) {
    DNALength prevLength = length;
    length += moreSeqLength;
    Nucleotide *prev = seq;
    seq = new Nucleotide[length];
    if (prev != NULL) {
        memcpy(seq, prev, prevLength);
        delete[] prev;
    }
    memcpy((Nucleotide*) &seq[prevLength], moreSeq, moreSeqLength);

}

std::string DNASequence::GetTitle() const {
    return std::string("");
}

void DNASequence::Concatenate(const Nucleotide* moreSeq) {
    DNALength moreSeqLength = strlen((char*) moreSeq);
    Concatenate(moreSeq, moreSeqLength);
}

void DNASequence::Concatenate(DNASequence &seq) {
    Concatenate(seq.seq, seq.length);
}

int DNASequence::Compare(DNALength pos, DNASequence &rhs, DNALength rhsPos, DNALength length) {
    return memcmp(&seq[pos], &rhs.seq[rhsPos], length);
}

int DNASequence::LessThanEqual(DNALength pos, DNASequence &rhs, DNALength rhsPos, DNALength length) {
    int res = Compare(pos, rhs, rhsPos, length);
    if (res <= 0) 
        return 1;
    else
        return 0;
}

int DNASequence::Equals(DNASequence &rhs, DNALength rhsPos, DNALength length, DNALength pos) {
    int res = Compare(pos, rhs, rhsPos, length);
    return res == 0;
}

int DNASequence::LessThan(DNALength pos,  DNASequence &rhs, DNALength rhsPos, DNALength length) {
    int res=  Compare(pos, rhs, rhsPos, length);
    return (res < 0);
}

void DNASequence::CleanupASCII() {
    DNALength i;
    for (i = 0; i < length; i++ ){
        if (ThreeBit[seq[i]] == 255) {
            seq[i] = 'N';
        }
    }
}

Nucleotide DNASequence::GetNuc(DNALength i) {
    return seq[i];
}

DNALength DNASequence::GetRepeatContent() {
    DNALength i;
    DNALength nRepeat = 0;
    for (i =0 ; i < length;i++) {
        if (tolower(seq[i]) == seq[i]) { nRepeat++;}
    }
    return nRepeat;
}

void DNASequence::CleanupOnFree() {
    deleteOnExit = true;
}

void DNASequence::FreeIfControlled() {
    if (deleteOnExit) {
        Free();
    }
}

void DNASequence::Free() {
    if (deleteOnExit == false) { return; }
    if (seq != NULL) {
        delete[] seq;
        seq = NULL;
        length = 0;
    }
}
void DNASequence::Resize(DNALength newLength) {
    if (seq != NULL) {
        delete[] seq;
    }
    seq = new  Nucleotide[newLength];
    length = newLength;
    deleteOnExit = true;
}
DNALength DNASequence::GetSeqStorage() {
    return length;
}

