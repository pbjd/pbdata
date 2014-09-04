
#include "SMRTReadUtils.hpp"

void GetSMRTReadCoordinates(FASTQSequence &seq, int &x, int &y) {
	std::string str(seq.title, seq.titleLength);
	std::vector<std::string> titleTokens;
	Tokenize(str, "_", titleTokens);
	int i;
	x = y = -1;
	int cmp;
	for (i = 0; i < titleTokens.size(); i++ ) {
		if (titleTokens[i].size() > 1 && titleTokens[i][0] == 'x') {
			x = atoi(&titleTokens[i].c_str()[1]);
		}
		if (titleTokens[i].size() > 1 && titleTokens[i][0] == 'y') {
			y = atoi(&titleTokens[i].c_str()[1]);
		}
	}
	assert("Could not parse a title to find an x coordinate" != 0 or x != -1);
	assert("Could not parse a title to find a y coordiante" != 0 or y != -1);
}

void GetSpringfieldHoleNumberFromTitle(FASTQSequence &seq, unsigned int &holeNumber) {
	std::vector<std::string> titleTokens;
	Tokenize(seq.title, "/", titleTokens);
	if (titleTokens.size() < 2) {
		return;
	}
	holeNumber = atoi(titleTokens[1].c_str());
}
			
bool ParsePBIReadName(std::string &readName, std::string &movieName, int &holeNumber) {
  std::vector<std::string> tokens;
  ParseSeparatedList(readName, tokens, '/');
  if (tokens.size() < 2) {
    movieName = "";
    holeNumber = 0;
    return false;
  }
  else {
    movieName = tokens[0];
    holeNumber = atoi(tokens[1].c_str());
    return true;
  }
}
