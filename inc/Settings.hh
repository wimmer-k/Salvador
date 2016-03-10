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
  char* DALIFile(){return (char*)fDALIfile.c_str();}

  double Overflow(){return foverflow;}
  double Underflow(){return funderflow;}
  char* DALIPosFile(){return (char*)fDALIposfile.c_str();}
  double Beta(){return fbeta;}
  
  int AddbackType(){return faddbacktype;}
  double AddbackDistance(){return faddbackdistance;}
  double AddbackAngle(){return faddbackangle;}
  double AddbackThresh(){return faddbackthreshold;}
  double AddbackTimeDiff(int i){return faddbacktdiff[i];}

private:
  //! filename of the settings file
  string finputfile;
  //! verbose level
  int fverbose;
  //! time offsets for A/Q calibration
  double ftoffset[6];
  //! DALI calibration file
  string fDALIfile;

  //! Overflow value for gamma energies
  double foverflow;
  //! Underflow value for gamma energies
  double funderflow;

  //! averaged positions from the simulation
  string fDALIposfile;
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

};
#endif
