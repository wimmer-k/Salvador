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

  //! Get the vorbose level
  int VerboseLevel(){return fverbose;}
  //! Time of flight offsets for the A/Q
  double TimeOffset(int b){return ftoffset[b];}
  //! XML file with the DALI calibrations
  char* DALIFile(){return (char*)fDALIfile.c_str();}

  //! overflow energy
  double Overflow(){return foverflow;}
  //! underflow energy
  double Underflow(){return funderflow;}
  //! txt file witht the positions of the DALI crystals from the simulation
  char* DALIPosFile(){return (char*)fDALIposfile.c_str();}
  //! beta for the Doppler correction
  double Beta(){return fbeta;}
  
  //! type of addback to be used, 1 distance, 2 angles
  int AddbackType(){return faddbacktype;}
  //! maximum distance between hits to be considered for addback
  double AddbackDistance(){return faddbackdistance;}
  //! maximum angle between hits and target to be considered for addback
  double AddbackAngle(){return faddbackangle;}
  //! software threshold for addback, global
  double AddbackThresh(){return faddbackthreshold;}
  //! time difference between hits to be considered for addback
  double AddbackTimeDiff(int i){return faddbacktdiff[i];}

  //! target position with respect to nominal focus
  double TargetPosition(){return ftargetposition;}

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

  //! target position with respect to nominal focus
  double ftargetposition;
};
#endif
