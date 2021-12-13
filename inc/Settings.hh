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
  //! Set the vorbose level
  void SetVerboseLevel(int vl){fverbose = vl;}

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

  //! The file containing the time dependent corrections
  const char* TimeCorFile(){return fTimeCorFile.c_str();}
  //! The file containing the events numbers at which each run starts
  const char* EvtNrFile(){return fEvtNrFile.c_str();}
  
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

  //! Get the beta in front of MINOS for the Doppler correction
  double BetaBefore(){return fbetaM[0];}
  //! Get the beta after MINOS for the Doppler correction
  double BetaAfter(){return fbetaM[1];}
  //! Get the lenght of the MINOS target
  double MINOSlength(){return fminoslength;}

  
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

  //! Get the alignment shift (X0) for the PPAC3 at F8 after the target
  double PPAC3PositionX0(){return fppac3align[0];}
  //! Get the alignment shift (Y0) for the PPAC3 at F8 after the target
  double PPAC3PositionY0(){return fppac3align[1];}
  //! Get the alignment shift (X1) for the PPAC3 at F8 after the target
  double PPAC3PositionX1(){return fppac3align[2];}
  //! Get the alignment shift (Y1) for the PPAC3 at F8 after the target
  double PPAC3PositionY1(){return fppac3align[3];}
  //! Get the target position with respect to nominal focus
  double TargetPosition(){return ftargetposition;}
  //! Get the gate on the F5X position
  double F5XGate(int i){return ff5xgate[i];}
  //! Get the gate on the change in delta for charge changes
  double DeltaGate(int i){return fdeltagate[i];}

  double GetAoQCorrection(int sp, int fp, int tr){return faoq_corr[sp][fp][tr];}
  double GetBRAoQCorrection_F3X(){return faoq_corr[0][0][0];}
  double GetBRAoQCorrection_F3A(){return faoq_corr[0][0][1];}
  double GetBRAoQCorrection_F3Q(){return faoq_corr[0][0][2];}
  double GetBRAoQCorrection_F5X(){return faoq_corr[0][1][0];}
  double GetBRAoQCorrection_F5A(){return faoq_corr[0][1][1];}
  double GetBRAoQCorrection_F5Q(){return faoq_corr[0][1][2];}
  double GetBRAoQCorrection_F7X(){return faoq_corr[0][2][0];}
  double GetBRAoQCorrection_F7A(){return faoq_corr[0][2][1];}
  double GetBRAoQCorrection_F7Q(){return faoq_corr[0][2][2];}
  double GetZDAoQCorrection_F8X(){return faoq_corr[1][0][0];}
  double GetZDAoQCorrection_F8A(){return faoq_corr[1][0][1];}
  double GetZDAoQCorrection_F8Q(){return faoq_corr[1][0][2];}
  double GetZDAoQCorrection_F9X(){return faoq_corr[1][1][0];}
  double GetZDAoQCorrection_F9A(){return faoq_corr[1][1][1];}
  double GetZDAoQCorrection_F9Q(){return faoq_corr[1][1][2];}
  double GetZDAoQCorrection_F11X(){return faoq_corr[1][2][0];}
  double GetZDAoQCorrection_F11A(){return faoq_corr[1][2][1];}
  double GetZDAoQCorrection_F11Q(){return faoq_corr[1][2][2];}

  double GetBRAoQCorrection_gain(){return faoq_lin[0][0];}
  double GetBRAoQCorrection_offs(){return faoq_lin[0][1];}
  double GetZDAoQCorrection_gain(){return faoq_lin[1][0];}
  double GetZDAoQCorrection_offs(){return faoq_lin[1][1];}
  
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

  //! Time dependent corrections for IC
  string fTimeCorFile;
  //! Event numbers
  string fEvtNrFile;
  
  
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
  //! beta for Doppler correction with MINOS before and after target
  double fbetaM[2];
  //! length of minos
  double fminoslength;

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

  //! alignment of PPAc at F8
  double fppac3align[4];
  //! target position with respect to nominal focus
  double ftargetposition;
  //! gate on the F5X position
  double ff5xgate[2];
  //! gate on the delta change
  double fdeltagate[4];

  double faoq_corr[2][3][3];  // BR/ZD, focal plane, x,angle,q

  double faoq_lin[2][2]; // BR/ZD, gain,off
};
#endif
