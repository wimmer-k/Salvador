#ifndef __RECONSTRUCTION_HH
#define __RECONSTRUCTION_HH
#include <iostream>
#include <iomanip>


//#include "TTree.h"
#include "TEnv.h"

#include "DALIdefs.h"
#include "DALI.hh"

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
  //! manually set the beta
  void SetBeta(double beta){fbeta = beta;}
  //! read the average positions within the crystals
  void ReadPositions(const char *infile);
  //! sort by energy highest first
  vector<DALIHit*> Sort(vector<DALIHit*> dali);
  //! sort by energy lowest first
  vector<DALIHit*> Revert(vector<DALIHit*> dali);
  //! filter over and underflows
  vector<DALIHit*> FilterOverUnderflows(vector<DALIHit*> hits);
  //! set the positions
  void SetPositions(DALI* dali);
  //! apply the Doppler correction
  void DopplerCorrect(DALI* dali);
  //! check the positions of two hits and decide if they are added back
  bool Addback(DALIHit* hit0, DALIHit* hit1);
  //! do the adding back
  vector<DALIHit*> Addback(vector<DALIHit*> dali);
  
  // //! read in the addback table
  // void ReadAddBackTable();

private:
  //! average positions of first interaction points
  vector<vector<double> > fpositions;
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

  //! verbose level
  int fverbose;
  //! Overflow value for gamma energies
  double foverflow;
  //! Underflow value for gamma energies
  double funderflow;

};
#endif
