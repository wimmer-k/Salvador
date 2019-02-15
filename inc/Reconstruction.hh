#ifndef __RECONSTRUCTION_HH
#define __RECONSTRUCTION_HH
#include <iostream>
#include <iomanip>

#include "TGraph.h"
#include "TF1.h"
#include "Settings.hh"
#include "DALIdefs.h"
#include "DALI.hh"
#include "PPAC.hh"
/*!
  A class for reconstruction of DALI data, includes Doppler correction and add-back
*/
class Reconstruction {
public:
  //! default constructor
  Reconstruction(){};
  //! constructor
  Reconstruction(char* settings);
  //! dummy destructor
  ~Reconstruction(){
  };
  //! acces the settings
  Settings* GetSettings(){return fset;}
  //! manually set the beta
  void SetBeta(double beta){fbeta = beta;}
  //! read a list with bad channels
  void ReadBadChannels(const char *infile);
  //! read recalibration parameters
  void ReadReCalParams(const char *infile);
  //! read the average positions within the crystals
  void ReadPositions(const char *infile);
  //! recalibrate dali
  void ReCalibrate(vector<DALIHit*> dali);
  //! sort by energy highest first
  vector<DALIHit*> Sort(vector<DALIHit*> dali);
  //! sort by energy lowest first
  vector<DALIHit*> Revert(vector<DALIHit*> dali);
  //! filter bad channels, over and underflows
  vector<DALIHit*> FilterBadHits(vector<DALIHit*> hits);
  //! filter over and underflows
  vector<DALIHit*> FilterOverUnderflows(vector<DALIHit*> hits);
  //! apply the time cut
  vector<DALIHit*> TimingGate(vector<DALIHit*> hits);
  //! set the positions
  void SetPositions(DALI* dali);
  //! apply the Doppler correction
  void DopplerCorrect(DALI* dali);
  //! apply the Doppler correction with a certain reaction point
  double DopplerCorrect(DALI* dali, double zreac);
  //! check the positions of two hits and decide if they are added back
  bool Addback(DALIHit* hit0, DALIHit* hit1);
  //! do the adding back
  vector<DALIHit*> Addback(vector<DALIHit*> dali);
  //! calculate the PPAC position
  TVector3 PPACPosition(SinglePPAC* pina, SinglePPAC* pinb);
  //! Align the PPAC3 after the target
  void AlignPPAC(SinglePPAC* pin0, SinglePPAC* pin1);
  //! calculate the target position
  TVector3 TargetPosition(TVector3 inc, TVector3 ppac);
  //! recalibration to be done
  bool DoReCalibration(){return fset->DoReCalibration();}
  //!  gate on the F5X position
  bool F5XGate(double f5xpos);
  //!  gate on changing charge state in ZeroDegree
  bool ChargeChange(double delta2, double delta3){return ChargeChangeZD(delta2, delta3);}
  //!  gate on changing charge state in ZeroDegree
  bool ChargeChangeZD(double delta2, double delta3);
  //!  gate on changing charge state in BigRIPS
  bool ChargeChangeBR(double delta0, double delta1);

private:
  //! settings for reconstruction
  Settings* fset;
  //! average beta for Doppler correction
  double fbeta;
  //! average positions of first interaction points
  vector<vector<double> > fpositions;
  //! which detectors are bad and should be excluded
  vector<unsigned short> fbad;
  //! recalibration parameters
  vector<vector<double> > frecal;
  //! function to reconstruct beta from MINOS position
  TF1* fminos;
};
#endif
