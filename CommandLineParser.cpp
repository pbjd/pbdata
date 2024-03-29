#include "CommandLineParser.hpp"

CommandLineParser::
CommandLineParser() {
    lineLength = 80;
    numUnnamedOptions = 0;
    specialVersionFlag = false;
}

void CommandLineParser::
SetProgramSummary(std::string summaryp) {
    programSummary = summaryp;
}
	
void CommandLineParser::
SetHelp(std::string _help) {
    helpString = _help;
}

void CommandLineParser::
SetConciseHelp(std::string _conciseHelp) {
    conciseHelp = _conciseHelp;
}

void CommandLineParser::
SetProgramName(std::string namep) {
    programName = namep;
}

void CommandLineParser::
SetVersion(std::string versionp) {
    specialVersionFlag = true;
    version = versionp;
}
	
void CommandLineParser::
SetVerboseHelp(std::string helpp) {
    verboseHelp = helpp;
}

void CommandLineParser::
SetExamples(std::string examplesp) {
    examples = examplesp;
}

void CommandLineParser::
RegisterPreviousFlagsAsHidden() {
    VectorIndex i;
    for (i = 0; i < named.size(); i++ ){ 
        named[i] = false;
    }
    numUnnamedOptions = named.size();
}

void CommandLineParser::
RegisterVersionFlag(bool *value) {
    specialVersionFlag = true;
    RegisterFlagOption("version", value, "Print version number.");
}


void CommandLineParser::
RegisterFlagOption(std::string option, bool *value, 
    std::string description, bool required) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(Flag);
    optionValueIndexList.push_back(boolValues.size());
    boolValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}

void CommandLineParser::
RegisterIntOption(std::string option, int *value, 
    std::string description, OptionType type, bool required, bool hidden) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(type);
    optionValueIndexList.push_back(intValues.size());
    intValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}

void CommandLineParser::
RegisterFloatOption(std::string option, float *value, 
    std::string description, OptionType type, bool required) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(type);
    optionValueIndexList.push_back(floatValues.size());
    floatValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}

void CommandLineParser::
RegisterStringOption(std::string option, std::string *value,
    std::string description, bool required) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(String);
    optionValueIndexList.push_back(stringValues.size());
    stringValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}

void CommandLineParser::
RegisterStringListOption(std::string option, 
    std::vector<std::string> *value, std::string description, 
    bool required) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(StringList);
    optionValueIndexList.push_back(stringListValues.size());
    stringListValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}


void CommandLineParser::
RegisterIntListOption(std::string option, std::vector<int> *value, 
    std::string description, bool required) {

    named.push_back(true);
    optionList.push_back(option);
    optionTypeList.push_back(IntegerList);
    optionValueIndexList.push_back(intListValues.size());
    intListValues.push_back(value);
    descriptions.push_back(description);
    optionRequired.push_back(required);
    optionUsed.push_back(false);
}


int CommandLineParser::
IsOption(char *str) {
    int len = strlen(str);
    if (len == 0) {
        return 0;
    }
    else {
        return str[0] == '-';
    }
}

int CommandLineParser::
IsInteger(char *str) {
    int len = strlen(str);
    int i;
    if (len == 0)
        return 0;
    if (!(str[0] == '-' or (str[0] >= '0' and str[0] <= '9')))
        return 0;
    for (i = 1; i < len; i++) {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

int CommandLineParser::
IsFloat(char *str) {
    int len = strlen(str);
    int i;
    if (len == 0) {
        return 0;
    }
    int nDot = 0;
    int nDigit = 0;
    for (i = 0; i < len; i++) {
        if (isdigit(str[i])) nDigit++;
        if (str[i] == '.') nDot++;
    }
    if (nDot > 1)
        return 0;
    if (nDigit == 0)
        return 0;
    if (!isdigit(str[0]) and str[0] != '-' and str[0] != '.')
        return 0;
    //
    // passed all checks, ok!
    //
    return 1;
}

int CommandLineParser::
FindOption(char *option) {
    VectorIndex i;
    for (i = 0; i < optionList.size(); i++ ){ 
        if (optionList[i].compare(option) == 0) {
            return i;
        }
    }
    return -1;
}

void CommandLineParser::
CommandLineToString(int argc, char* argv[], 
    std::string& commandLine) {

    std::stringstream outstrm;
    int i;
    for (i = 0; i< argc; i++) {
        outstrm << argv[i] << " ";
    }
    commandLine = outstrm.str();
}


int CommandLineParser::
ParseCommandLine(int argc, char* argv[]) {
    std::vector<std::string> ufv;
    return ParseCommandLine(argc, argv, ufv);
}

int CommandLineParser::
ParseCommandLine(int argc, char* argv[], 
    std::vector<std::string> &unflaggedValues) {

    VectorIndex argi = 1;
    int curUnnamedOption = 0;
    ErrorValue ev; 
    //
    // Check for a help flag.
    //
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h")==0 or
            strcmp(argv[i], "--help") == 0 and 
            // Check to see if there is non default argument for help
            (IsOption(argv[i]) and !FindOption(&argv[i][1]))) {
            PrintUsage();
            exit(0);
        }
        else if (strcmp(argv[i], "-version") == 0 and specialVersionFlag) {
            //
            // Using -version is an early exit since programs will print the 
            // version and then return.
            //
            assert(IsOption(argv[i]) and FindOption(&argv[argi][1]));
            PrintVersion();
            exit(0);
        }
    }
    if (argc == 1 || argc < numUnnamedOptions) {
        if (conciseHelp != "") {
            std::cout << conciseHelp;
        }
        else {
            PrintUsage();
        }
        exit(0);
    }
    // 
    // Now parse the (probably optional) options.
    //
    while (argi < (VectorIndex) argc){
        if (IsOption(argv[argi])) {
            // 
            // Find which option is specified.
            //
            int optionIndex = FindOption(&argv[argi][1]);
            if (optionIndex == -1) {
                ev = CLBadOption;
            }
            else {
                argi++;

                //
                // Record that this option has been specified.
                //
                optionUsed[optionIndex] = true;
                ev = ParseOption(optionIndex, argi, argc, argv);
            }
            if (ev != CLGood) {
                PrintUsage();
                PrintErrorMessage(ev, &argv[argi][1]);
                exit(1);
            }
        }
        else {
            unflaggedValues.push_back(argv[argi]);
            if (curUnnamedOption < numUnnamedOptions) {
                ev = ParseOption(curUnnamedOption, argi, argc, argv);
                optionUsed[curUnnamedOption] = true;
                curUnnamedOption++;
            }
            else {
                ++argi;
            }
        }
    }

    ev = PrintErrorOnMissingOptions();
    if (ev != CLGood) {
        PrintUsage();
        PrintErrorMessage(ev, &argv[argi][1]);
        exit(1);
    }
    return 1;
}

CommandLineParser::ErrorValue CommandLineParser::
ParseOption(VectorIndex optionIndex, VectorIndex &argi, 
    int argc, char *argv[]) {

    ErrorValue ev;
    // 
    // Extract the value type of this option.
    //
    int optionValueIndex = optionValueIndexList[optionIndex];
    OptionType optionType = optionTypeList[optionIndex];

    switch(optionType) {
        case(Flag):
            ev = ParseFlag(optionValueIndex, argi, argc, argv);
            break;
        case(Integer):
            ev = ParseInteger(optionValueIndex, argi, argc, argv);
            break;
        case(PositiveInteger):
            ev = ParsePositiveInteger(optionValueIndex, argi, argc, argv);
            break;
        case(NonNegativeInteger):
            ev = ParseNonNegativeInteger(optionValueIndex, argi, argc, argv);
            break;
        case(Float):
            ev = ParseFloat(optionValueIndex, argi, argc, argv);
            break;
        case( PositiveFloat):
            ev = ParsePositiveFloat(optionValueIndex, argi, argc, argv);
            break;
        case(NonNegativeFloat):
            ev = ParseNonNegativeFloat(optionValueIndex, argi, argc, argv);
            break;
        case(String):
            ev = ParseString(optionValueIndex, argi, argc, argv);
            break;
        case(StringList):
            ev = ParseStringList(optionValueIndex, argi, argc, argv);
            break;
        case(IntegerList):
            ev = ParseIntList(optionValueIndex, argi, argc, argv);
            break;
    };
    if (ev == CLGood) {
        optionUsed[optionValueIndex] = true;
    }
    return ev;
}

void CommandLineParser::
PrintErrorMessage(ErrorValue ev, char *option) {
    switch(ev) {
        case(CLBadOption):
            std::cout << "ERROR: " << option 
                      << " is not a valid option." << std::endl;
            break;
        case(CLMissingValue):
            std::cout << "ERROR: " << option 
                      << " requires a value." << std::endl;
            break;
        case(CLInvalidInteger):
            std::cout << "ERROR: " << option  << " requires an "
                      << "integer value (...,-2,-1,0,1,2,...)" 
                      << std::endl;
            break;
        case(CLInvalidPositiveInteger):
            std::cout << "ERROR: " << option 
                      << " requires an integer greater than 0."
                      << std::endl;
            break;
        case(CLInvalidNonNegativeInteger):
            std::cout << "ERROR: " << option 
                      << " requires an interger greater " 
                      << "than or equal to 0." << std::endl;
            break;
        case(CLInvalidFloat):
            std::cout << "ERROR: " << option 
                      << " requires a number as input." << std::endl;
            break;
        case(CLInvalidPositiveFloat):
            std::cout << "ERROR: " << option 
                      << " must be greater than 0 (eg. .0001)." 
                      << std::endl;
            break;
        case (CLInvalidNonNegativeFloat):
            std::cout << "ERROR: " << option 
                      << " must be greater than or equal to 0."
                      << std::endl;
            break;
        default:
            break;
    };
}


CommandLineParser::ErrorValue CommandLineParser::
ParseFlag(VectorIndex optionValueIndex, VectorIndex &argi, 
    int argc, char *argv[]) {

    *boolValues[optionValueIndex] = !(*boolValues[optionValueIndex]);
    return CLGood;
}


CommandLineParser::ErrorValue CommandLineParser::
ParseInteger(VectorIndex optionValueIndex, VectorIndex &argi, 
    int argc, char *argv[]) {

    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsInteger(argv[argi])) {
        *intValues[optionValueIndex] = atoi(argv[argi]);
        ++argi;
        return CLGood;
    }
    else {
        // reset argi to the flag that was broken.
        --argi;
        return CLInvalidInteger;
    }
}


CommandLineParser::ErrorValue CommandLineParser::
ParsePositiveInteger(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    int value;
    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsInteger(argv[argi])) {
        value =  atoi(argv[argi]);
        if (value > 0) {
            *intValues[optionValueIndex] = value;
            ++argi;
            return CLGood;
        }
    }
    // reset argi to the flag that was broken.
    --argi;
    return CLInvalidPositiveInteger;
}

CommandLineParser::ErrorValue CommandLineParser::
ParseNonNegativeInteger(VectorIndex optionValueIndex,
    VectorIndex &argi, int argc, char *argv[]) {

    int value;
    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsInteger(argv[argi])) {
        value =  atoi(argv[argi]);
        if (value >= 0) {
            *intValues[optionValueIndex] = value;
            ++argi;
            return CLGood;
        }
    }
    // reset argi to the flag that was broken.
    --argi;
    return CLInvalidNonNegativeInteger;
}


CommandLineParser::ErrorValue CommandLineParser::
ParseFloat(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsFloat(argv[argi])) {
        *floatValues[optionValueIndex] = atof(argv[argi]);
        ++argi;
        return CLGood;
    }
    else {
        // reset argi to the flag that was broken.
        --argi;
        return CLInvalidFloat;
    }
}


CommandLineParser::ErrorValue CommandLineParser::
ParsePositiveFloat(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    float value;
    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsFloat(argv[argi])) {
        value = atof(argv[argi]);
        if (value > 0) {
            *floatValues[optionValueIndex] = value;
            ++argi;
            return CLGood;
        }
    }
    // reset argi pointer to bad flag
    --argi;
    return CLInvalidPositiveFloat;
}


CommandLineParser::ErrorValue CommandLineParser::
ParseNonNegativeFloat(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    float value;
    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (IsFloat(argv[argi])) {
        value = atof(argv[argi]);
        if (value >= 0) {
            *floatValues[optionValueIndex] = value;
            ++argi;
            return CLGood;
        }
    }			
    // reset argi to the flag that was broken.
    --argi;
    return CLInvalidNonNegativeFloat;
}


CommandLineParser::ErrorValue CommandLineParser::
ParseString(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    if (argi < (unsigned int) argc) {
        *stringValues[optionValueIndex] = argv[argi];
        ++argi;
        return CLGood;
    }
    else {
        // reset argi to the flag that was broken.
        --argi;
        return CLMissingValue;
    }
}

bool CommandLineParser::
IsValuedOption(OptionType optType) {
    if (optType == Integer or 
            optType == PositiveInteger or
            optType == NonNegativeInteger or
            optType == Float or 
            optType == PositiveFloat or 
            optType == NonNegativeFloat or
            optType == String or
            optType == StringList) {
        return true;
    }
    else {
        return false;
    }
}


CommandLineParser::ErrorValue CommandLineParser::
ParseIntList(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {
    
    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    ErrorValue ev;
    ev = CLMissingValue;
    while (argi < (unsigned int) argc and !IsOption(argv[argi])) {
        if (IsInteger(argv[argi])) {
            intListValues[optionValueIndex]->push_back(atoi(argv[argi]));
            ++argi;
            ev = CLGood;
        }
        else {
            ev = CLInvalidInteger;
            --argi;
            break;
        }
    }
    if (ev == CLMissingValue) {
        // reset argi to the flag that was broken.
        --argi;
    }
    return ev;
}

CommandLineParser::ErrorValue CommandLineParser::
ParseStringList(VectorIndex optionValueIndex, 
    VectorIndex &argi, int argc, char *argv[]) {

    if (argi >= argc) {
        --argi;
        return CLMissingValue;
    }
    ErrorValue ev;
    ev = CLMissingValue;
    while (argi < (unsigned int) argc and !IsOption(argv[argi])) {
        stringListValues[optionValueIndex]->push_back(argv[argi]);
        ++argi;
        ev = CLGood;
    }
    if (ev == CLMissingValue) {
        // reset argi to the flag that was broken.
        --argi;
    }
    return ev;
}


void CommandLineParser::
PrintVersion() {
    std::cout << programName << "\t" << version << std::endl;
}


void CommandLineParser::
PrintUsage() {
    std::ios::fmtflags f = std::cout.flags();
    if (helpString != "") {
        std::cout << helpString << std::endl;
        return;
    }
    else {
        if (programSummary.size() > 0) {
            std::cout << programName << " ";

            PrintIndentedText(std::cout, programSummary, 
                programName.size(), lineLength);

            std::cout << std::endl;
        }
        std::cout << std::endl << "usage: " << programName;
        VectorIndex i = 0;
        int maxOptionLength = GetMaxOptionLength();
        while (i < optionList.size() and named[i] == false) {
            std::cout << " ";
            if (optionRequired[i] == false) {
                std::cout << "[";
            }
            std::cout << optionList[i];
            if (optionRequired[i] == false) {
                std::cout << "]";
            }
            i++;
        }
        if (i < optionList.size()) {
            std::cout << " [options] ";
        }
        std::cout << std::endl << std::endl;
        i = 0;
        while (i < optionList.size() and named[i] == false) { 
            if (!named[i]) {
                std::cout << "   " << std::setw(maxOptionLength) 
                          << std::left << optionList[i] << std::endl;

                PrintIndentedText(cout, descriptions[i], 15, 
                    (int)lineLength, 15);

                std::cout << std::endl;
            }
            i++;
        }
        for (; i < optionList.size(); i++) {
            std::string wholeName = "-";
            wholeName += optionList[i];
            if (IsValuedOption(optionTypeList[i])) {
                wholeName += " value ";
            }
            std::cout << "  "   << std::setw(maxOptionLength) 
                      << std::left <<  wholeName << std::endl;

            PrintIndentedText(cout, descriptions[i], 15, 
                (int)lineLength, 15);
            
            std::cout << std::endl;
        }
    }
    if (examples.size() > 0) {
        std::cout << std::endl << std::endl;
        PrintIndentedText(cout, examples, 5, (int) lineLength, 5);
        std::cout << std::endl;
    }
    std::cout.flags(f);
}

int CommandLineParser::
GetNextWordLength(std::string &text, int pos) {
    int startPos = pos;
    int textLength = text.size();
    while (pos < textLength and (!IsWhitespace(text[pos]))) {
        pos++;
    }
    return pos - startPos;
}

void CommandLineParser::
PrintIndentedText(std::ostream &out, std::string &text, 
    int allLineIndent, int lineLength, int firstLineIndent) {

    int textPos = 0;
    std::vector<std::string> words;
    ToWords(text, words);
    int w = 0;
    int wordsOnLine;
    int curLineLength;
    int curLineIndent = 0;
    int i;
    if (firstLineIndent == 0) {
        curLineIndent = 0;
        curLineLength = allLineIndent;
    }
    else {
        for (i = 0; i < firstLineIndent; i++) {
            out << " ";
        }
        curLineIndent = allLineIndent;
        curLineLength = allLineIndent;
    }
    std::string indentation;
    indentation.insert(0, allLineIndent, ' ');
    int textLength = text.size();
    while (textPos < textLength) {
        // Print some whitespace
        while (textPos < textLength and 
               curLineLength < lineLength and 
               IsWhitespace(text[textPos])) {
            out << text[textPos];
            // Some extra logic in case 
            if (text[textPos] == '\n') {
                // an extra line was printed, so skip to the next line.
                curLineLength = lineLength + 1; 
                // done printing this line.
                curLineLength = 0;
                if (textPos < textLength) {
                    out << indentation;
                    curLineLength = allLineIndent;
                }
            }
            else {
                curLineLength++;
                if (curLineLength == lineLength) {
                    std::cout << std::endl;
                    curLineLength = 0;
                    if (textPos < textLength) {
                        out << indentation;
                        curLineLength = allLineIndent;
                    }
                }
            }
            textPos++;
        }
        // Possibly print a word.
        if (!IsWhitespace(text[textPos])) {
            int nextWordLength = GetNextWordLength(text, textPos);
            if (nextWordLength + curLineLength >= lineLength) {
                //
                // The next word runs past the end of this line, 
                // print it on a newline, or print part of it on 
                // this line if the whole word wraps.
                //
                if (nextWordLength > lineLength) {
                    // This word will never fit on a line, print part of it.
                    int substringLength = lineLength - curLineLength;
                    for (; curLineLength < lineLength; 
                         curLineLength++, textPos++) {
                        out << text[textPos];
                    }
                }
                out << std::endl;
                out << indentation;
                curLineLength = allLineIndent;
            }
            else {
                int i;
                for (i = 0; i < nextWordLength; 
                     i++, textPos++, curLineLength++) {
                    out << text[textPos];
                }
            }
        }
    }
}

unsigned int  CommandLineParser::
GetMaxOptionLength() {
    VectorIndex i;
    VectorIndex maxLength = 0;
    for (i = 0; i < optionList.size(); i++ ){
        if (optionList[i].size() > maxLength)
            maxLength = optionList[i].size();
    }
    return maxLength;
}


CommandLineParser::ErrorValue  CommandLineParser::
PrintErrorOnMissingOptions() {
    VectorIndex i;
    ErrorValue ev = CLGood;
    for (i = 0; i < optionList.size(); i++ ){ 
        if (optionRequired[i] and !optionUsed[i]) {
            std::cout << "ERROR, the option " << optionList[i]
                      << " must be specified." << std::endl;
            ev = CLMissingOption;
        }
    }
    return ev;
}
