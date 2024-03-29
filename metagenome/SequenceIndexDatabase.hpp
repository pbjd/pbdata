#ifndef _BLASR_SEQUENCE_INDEX_DATABASE_HPP_
#define _BLASR_SEQUENCE_INDEX_DATABASE_HPP_

#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <cstring>
#include <algorithm>
#include "Types.h"
#include "DNASequence.hpp"
#include "StringUtils.hpp"


#define SEQUENCE_INDEX_DATABASE_MAGIC 1233211233

template<typename TSeq>
class SequenceIndexDatabase {
public:
    std::vector<DNALength> growableSeqStartPos;
    std::vector<std::string> growableName;

	DNALength *seqStartPos;
	bool deleteSeqStartPos;
	char **names;
	bool deleteNames;
	int  *nameLengths;
	bool deleteNameLengths;
	int  nSeqPos;
	bool deleteStructures;
    //
    // This is stored after reading in the sequence.
    //
    std::vector<std::string> md5;

    SequenceIndexDatabase(int final=0);

    DNALength GetLengthOfSeq(int seqIndex);

    // Return index of a reference sequence with name "seqName".
    int GetIndexOfSeqName(std::string seqName);

    void GetName(int seqIndex, std::string &name);

    void MakeSAMSQString(std::string &sqString);

    DNALength ChromosomePositionToGenome(int chrom, DNALength chromPos);

    int SearchForIndex(DNALength pos);

    std::string GetSpaceDelimitedName(unsigned int index);

    int SearchForStartBoundary(DNALength pos);

    int SearchForEndBoundary(DNALength pos);

    DNALength SearchForStartAndEnd(DNALength pos, DNALength &start,
        DNALength &end);

    void WriteDatabase(ofstream &out);

   	void ReadDatabase(ifstream &in);

	void SequenceTitleLinesToNames();

   	VectorIndex AddSequence(TSeq &sequence);

   	void Finalize();

	void FreeDatabase();
};


template< typename TSeq >
class SeqBoundaryFtr {
public:
    SequenceIndexDatabase<TSeq> *seqDB;

    SeqBoundaryFtr(SequenceIndexDatabase<TSeq> *_seqDB);

    int GetIndex(DNALength pos);

    int GetStartPos(int index);

    DNALength operator()(DNALength pos);

    // This is misuse of a functor, but easier interface coding for now.
    DNALength Length(DNALength pos);
};

#include "metagenome/SequenceIndexDatabaseImpl.hpp"

#endif
