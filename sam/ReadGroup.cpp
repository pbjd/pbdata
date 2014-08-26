#include "ReadGroup.hpp"

void SAMReadGroup::StoreValues(std::vector<SAMKeywordValuePair> &kvPairs,
                               int lineNumber) {
  int i;
  bool idIsStored = false;
  for (i = 0; i < kvPairs.size(); i++ ){
    if (kvPairs[i].key == "ID") {
      id = kvPairs[i].value;
      idIsStored = true;
    }
  }
  if (idIsStored == false) {
    std::cout << "ReadGroup missing id at " << lineNumber << std::endl;
    exit(1);
  }
}

void SAMFullReadGroup::StoreValues(vector<SAMKeywordValuePair> &kvPairs,
                 int lineNumber) {
  SAMReadGroup::StoreValues(kvPairs, lineNumber);
  std::string kwPair;
  std::string key, valueStr;
  int i;
  for (i = 0; i < kvPairs.size(); i++) {
    if (kvPairs[i].key == "CN") {
      centerName = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "DS") {
      description = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "DT") {
      date = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "FO") {
      flowOrder = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "LB") {
      library = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "PG") {
      processingProgram = kvPairs[i].value;
    }
    else if (kvPairs[i].key == "PI") {
      StoreValue(kvPairs[i].value, insertSize);
    }
    else if (kvPairs[i].key == "SM") {
      sample = kvPairs[i].value;
    }  
  }
}
