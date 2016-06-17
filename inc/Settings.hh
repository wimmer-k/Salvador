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

  //! Get if DALI is to be used
  bool WithDALI(){return fwithDALI;}

  //! Get the BigRIPS PPAC xml file
  char *PPACFile(){return (char*)fPPACfile.c_str();}
  //! Get the BigRIPS PPAC Default xml file
  char *PPACDefFile(){return (char*)fPPACdefaultfile.c_str();}
  //! Get the BigRIPS plastic xml file
  char *PlasticFile(){return (char*)fplasticfile.c_str();}
  //! Get the BigRIPS IC xml file
  char *ICFile(){return (char*)fICfile.c_str();}
  //! Get the BigRIPS Focalplane xml file
  char *FocalFile(){return (char*)ffocalfile.c_str();}
  //! Get the BigRIPS/ZeroDegree matrix file
  char *MatrixFile(int i){return (char*)fmatrixfile[i].c_str();}

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
  //! use DALI
  bool fwithDALI;
  //! BigRIPS PPAC xml file
  string fPPACfile;
  //! BigRIPS PPAC default xml file
  string fPPACdefaultfile;
  //! BigRIPS Plastic xml file
  string fplasticfile;
  //! BigRIPS IC xml file
  string fICfile;
  //! BigRIPS Focalplane xml file
  string ffocalfile;
  //! BigRIPS/ZeroDegree matrix files
  string fmatrixfile[4];
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
