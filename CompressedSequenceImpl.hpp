#ifndef _BLASR_COMPRESSED_SEQUENCES_IMPL_HPP_
#define _BLASR_COMPRESSED_SEQUENCES_IMPL_HPP_

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::CopyConfiguration(CompressedSequence<T_Sequence> &rhs) {
    hasIndex = rhs.hasIndex;
    hasTitle = rhs.hasTitle;
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::ShallowCopy(CompressedSequence<T_Sequence> &rhs) {
    //
    // Copy over non sequence information
    index.ShallowCopy(rhs.index);
    CopyConfiguration(rhs);

    // Copy sequence information
    ((FASTASequence*)this)->ShallowCopy((FASTASequence&)rhs);
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::MakeRC(CompressedSequence &rc) {
    rc.Allocate(length);
    DNALength i;
    for (i = 0; i < length; i++) {
        rc.seq[length - i - 1] = ReverseComplementNuc[ThreeBit[seq[i] & MaskCount]];
        rc.seq[length - i - 1] += (seq[i] & MaskNuc); 
    }
    if (rc.title != NULL) {
        delete[] rc.title;
    }
    rc.title = new char[titleLength+1];
    memcpy(rc.title, title, titleLength);
    rc.titleLength = titleLength;
    rc.title[titleLength] = '\0';
}

template<typename T_Sequence>
Nucleotide CompressedSequence<T_Sequence>::operator[](DNALength i) {
    return GetNuc(i);
}

template<typename T_Sequence>
Nucleotide CompressedSequence<T_Sequence>::GetNuc(DNALength i) {
    return (seq[i] & MaskCount);
}

template<typename T_Sequence>
unsigned char CompressedSequence<T_Sequence>::GetCount(DNALength i) {
    return seq[i] >> ShiftCount;
}

template<typename T_Sequence>
char* CompressedSequence<T_Sequence>::GetName() {
    return (char*) title;
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::Copy(FASTASequence &rhs) {
    seq = new CompressedNucleotide[rhs.length];
    memcpy(seq, rhs.seq, rhs.length);
    length = rhs.length;
    if (title != NULL) {
        delete[] title;
    }
    title = new char[rhs.titleLength+1];
    memcpy(title, rhs.title, rhs.titleLength);
    titleLength = rhs.titleLength;
    title[titleLength] = '\0';
}

template<typename T_Sequence>
float CompressedSequence<T_Sequence>::GetAverageQuality() {
    return 0.0;
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::SortHomopolymerQualities() {
    std::cout << "qualities are not implemented for compressed sequences." 
    << std::endl;
}

template<typename T_Sequence>
CompressedSequence<T_Sequence>::CompressedSequence() {
    hasIndex = 0;
    hasTitle = 0;
    qual = NULL;
    FASTASequence();
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::SetHasTitle() {
    hasTitle = 1;
}
template<typename T_Sequence>

void CompressedSequence<T_Sequence>::SetHasIndex() {
    hasIndex = 1;
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::Write(std::string outFileName) {
    std::ofstream out;
    CrucialOpen(outFileName,out, std::ios::binary | std::ios::in);
    out.write((char*) &hasTitle, sizeof(int));
    out.write((char*) &hasIndex, sizeof(int));
    if (hasTitle) {
        out.write((char*)&titleLength, sizeof(int));
        out.write((char*)title, titleLength);
    }
    out.write((char*) &length, sizeof(int));
    out.write((char*) seq, sizeof(char) * length);
    if (hasIndex) {
        index.Write(out);
    }
    out.close();
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::Read(std::string inFileName) {
    std::ifstream in;
    CrucialOpen(inFileName, in, std::ios::binary | std::ios::in);
    // step 1, read in the options.
    in.read((char*) &hasTitle, sizeof(int));
    in.read((char*) &hasIndex, sizeof(int));
    if (hasTitle) {
        in.read((char*) &titleLength, sizeof(int));
        title = new char[titleLength+1];
        in.read((char*) title, titleLength);
        title[titleLength] = '\0';
    }
    in.read((char*) &length, sizeof(DNALength));
    seq = new Nucleotide[length];
    in.read((char*) seq, length * sizeof(Nucleotide));
    if (hasIndex) {
        index.Read(in);
    }
}

template<typename T_Sequence>
int CompressedSequence<T_Sequence>::BuildFourBitReverseIndex(int binSize) {
    BuildReverseIndex(15, binSize);
}

template<typename T_Sequence>
int CompressedSequence<T_Sequence>::BuildReverseIndex(int maxRun, int binSize) {
    hasIndex = 1;

    DNALength i;
    DNALength hpi;

    // 
    // Phase 1. Count the number of nucleotide transitions-1
    //

    hpi = 0;
    for (i = 0; i < length; i++) { 
        //		if (hpi % binSize == 0 ){
        //			index.push_back(i);
        //		}
        int run = 1;
        while (i < length - 1 
                and ThreeBit[seq[i]] == ThreeBit[seq[i+1]] and
                (run == 0 or 
                 run < maxRun)) {i++, run++;};
        hpi++;
    }


    //
    // Phase 2. Store the index.
    //
    index.indexLength = hpi/index.binSize + 1;
    index.index = new int[index.indexLength];
    hpi = 0;
    int ii = 0;
    for (i = 0; i < length; i++) { 
        if (hpi % index.binSize == 0 ) {
            assert(ii < index.indexLength);
            index.index[ii] = i;
            ++ii;
        }
        int run = 1;
        while (i < length - 1 
                and ThreeBit[seq[i]] == ThreeBit[seq[i+1]] and
                (run == 0 or 
                 run < maxRun)) {i++, run++;};
        hpi++;
    }

    return index.size();
}

template<typename T_Sequence>
long CompressedSequence<T_Sequence>::Lookup4BitCompressedSequencePos(int cpPos) {
    int      bin = cpPos / index.binSize;
    int  origPos = index.index[bin];
    int cpBinPos = bin * index.binSize;
    int cp;
    for (cp = cpBinPos; cp < cpPos; cp++ ){
        origPos += GetCount(seq[cp]);
    }
    return origPos;
}


template<typename T_Sequence>
int CompressedSequence<T_Sequence>::LookupSequencePos(int hpPos) {
    int origPos = index.index[hpPos % index.binSize];
    int hpi;
    for (hpi = (hpPos / index.binSize) * index.binSize; hpi < hpPos; hpi++, origPos++ ) {
        // 
        // advance orig across all homopolymer stretches.
        //
        while (origPos < length - 1 and
                seq[origPos] == seq[origPos+1])
            ++origPos;
    }
    return origPos;
}

template<typename T_Sequence>
char CompressedSequence<T_Sequence>::GetCount(unsigned char ch) {
    return (ch >> 4);
}

template<typename T_Sequence>
DNALength CompressedSequence<T_Sequence>::FourBitCompressHomopolymers() {
    VectorIndex i, c;
    unsigned char count = 0;
    for (i =0, c = 0; i < length; c++, i++) {
        count = 1;
        while (count < 15 and 
                i < length - 1 
                and ThreeBit[seq[i]] == ThreeBit[seq[i+1]]) {
            i++; count++;
        }
        // store nuc into the lower 4 bits
        seq[c] = ThreeBit[seq[i]];

        // store count into the upper 4 bits.
        count = count << 4;
        seq[c] = seq[c] | count;

    }
    length = c;	
    return length;
}

template<typename T_Sequence>
int CompressedSequence<T_Sequence>::Only4BitACTG(CompressedNucleotide *seq, int seqLength) {
    int i;
    for (i = 0; i < seqLength; i++ ){
        if (ThreeBit[seq[i] & MaskCount] > 3) {
            return 0;
        }
    }
    return 1;
}

template<typename T_Sequence>
int CompressedSequence<T_Sequence>::Only4BitACTG() {
    return Only4BitACTG(seq, length);
}

template<typename T_Sequence>
void CompressedSequence<T_Sequence>::RemoveCompressionCounts() {
    DNALength i;
    unsigned char mask =0xf;
    for (i = 0; i< length; i++) {
        seq[i] = seq[i] & mask;
    }
}

template<typename T_Sequence>
DNALength CompressedSequence<T_Sequence>::FourBitDecompressHomopolymers(int start, int end, 
        T_Sequence &decompSeq) {

    //
    // first compute the lenght of the decoded 
    //
    DNALength i;
    decompSeq.length = 0;
    for (i = start; i < end; i++ ){ 
        unsigned char count;
        count = (unsigned char) seq[i];
        count >>= 4;
        decompSeq.length += count;
    }
    decompSeq.seq = new Nucleotide[decompSeq.length];

    //
    // Now store the actual decompressed seq.
    //
    int d = 0;
    unsigned char mask = 0xf;
    for (i = start; i < end; i++ ){ 
        unsigned char count;
        count = (unsigned char) seq[i];
        count >>= 4;
        int j;
        for (j = 0; j < count; j++ ){ 
            decompSeq.seq[d] = FourBitToAscii[(seq[i] & mask)];
            d++;
        }
    }
    decompSeq.bitsPerNuc = 4;
    return decompSeq.length;
}

template<typename T_Sequence>
DNALength CompressedSequence<T_Sequence>::CondenseHomopolymers() {
    VectorIndex i, c;
    for (i =0, c = 0; i < length; c++, i++) {
        while (i < length - 1 and ThreeBit[seq[i]] == ThreeBit[seq[i+1]]) i++;
        seq[c] = seq[i];
    }
    length = c;
    return length;
}

#endif
