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
  //! Read the settings
  void ReadSettings();
  //! Print the settings
  void PrintSettings();

  //! Get the vorbose level
  int VerboseLevel(){return fverbose;}
  //! Get the time of flight offsets for the A/Q
  double TimeOffset(int b){return ftoffset[b];}
  //! XML file with the DALI calibrations
  char* DALIFile(){return (char*)fDALIfile.c_str();}

  //! Get the file with the bad DALI channels
  char* BadChFile(){return (char*)fDALIbadfile.c_str();}
  //! Get the file with the DALI recalibration parameters
  char* ReCalFile(){return (char*)fDALIrecalfile.c_str();}
  //! Do the recalibration
  bool DoReCalibration(){return fdorecal;}

  //! Get the overflow energy
  double Overflow(){return foverflow;}
  //! Get the underflow energy
  double Underflow(){return funderflow;}
  //! Get the txt file witht the positions of the DALI crystals from the simulation
  char* DALIPosFile(){return (char*)fDALIposfile.c_str();}
  //! Get the beta for the Doppler correction
  double Beta(){return fbeta;}
  //! Get the gate on the DALI - beam timing
  double TimingGate(int i){return ftimegate[i];}

  
  //! Get the type of addback to be used, 1 distance, 2 angles
  int AddbackType(){return faddbacktype;}
  //! Get the maximum distance between hits to be considered for addback
  double AddbackDistance(){return faddbackdistance;}
  //! Get the maximum angle between hits and target to be considered for addback
  double AddbackAngle(){return faddbackangle;}
  //! Get the software threshold for addback, global
  double AddbackThresh(){return faddbackthreshold;}
  //! Get the time difference between hits to be considered for addback
  double AddbackTimeDiff(int i){return faddbacktdiff[i];}

  //! Get the target position with respect to nominal focus
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
  
  //! DALI bad channels file
  string fDALIbadfile;
  //! DALI recalibration file
  string fDALIrecalfile;
  //! do recalibration
  bool fdorecal;
  //! Overflow value for gamma energies
  double foverflow;
  //! Underflow value for gamma energies
  double funderflow;

  //! averaged positions from the simulation
  string fDALIposfile;
  //! averge beta for Doppler correction
  double fbeta;
  //! timing gate DALI - beam
  double ftimegate[2];

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
