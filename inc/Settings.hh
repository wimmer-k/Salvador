#ifndef __SETTINGS_HH
#define __SETTINGS_HH
#include <iostream>
#include <iomanip>
#include <string>

#include "TEnv.h"

using namespace std;

/*!
  A container for the analysis settings
*/
class Settings {
public:
  //! default constructor
  Settings(){};
  //! constructor
  Settings(char* settings);
  //! dummy destructor
  ~Settings(){
  };
  //! read the settings
  void ReadSettings();
  //! print the settings
  void PrintSettings();

  int VerboseLevel(){return fverbose;}
  double TimeOffset(int b){return ftoffset[b];}
  const char* DALIFile(){return fDALIFile.c_str();}

private:
  //! filename of the settings file
  string finputfile;
  //! verbose level
  int fverbose;
  //! time offsets for A/Q calibration
  double ftoffset[6];
  //! DALI calibration file
  string fDALIFile;

  //! averge beta for Doppler correction
  double fbeta;
  //! type of addback
  int faddbacktype;
  //! max distance between two hits for addback
  double faddbackdistance;
  //! max angle between two hits for addback
  double faddbackangle;
  //! energy threshold for addback
  double faddbackthreshold;
  //! time difference between two hits for addback
  double faddbacktdiff[2];

  //! Overflow value for gamma energies
  double foverflow;
  //! Underflow value for gamma energies
  double funderflow;

  //! averaged positions from the simulation
  string fposfile;
};
#endif
