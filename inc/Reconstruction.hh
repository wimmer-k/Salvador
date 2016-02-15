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
  //! read the average positions within the crystals
  void ReadPositions(const char *infile);
  //! sort by energy
  vector<DALIHit*> Sort(vector<DALIHit*> dali);
  //! set the positions
  void SetPositions(DALI* dali);


  //void DopplerCorrect();
  
  // //! read in the addback table
  // void ReadAddBackTable();

private:
  //! average positions of first interaction points
  vector<vector<double> > fpositions;
  //! averge beta for Doppler correction
  double fbeta;
  int fverbose;
};
#endif
