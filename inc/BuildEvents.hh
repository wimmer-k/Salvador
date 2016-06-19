#ifndef __BUILDEVENTS_HH
#define __BUILDEVENTS_HH
#include "TTree.h"
#include "TClonesArray.h"

#include "Beam.hh"
#include "EURICA.hh"
/*!
  A class for building BigRIPS and EURICA combined events
*/
class BuildEvents {
public:
  //! default constructor
  BuildEvents(){};
  //! Initialize trees
  void Init(TTree* brtr, TTree* eutr, TTree* mtr);
private:
  //! BigRIPS input tree
  TTree* fBRtr;
  //! EURICA input tree
  TTree* fEUtr;
  //! merged output tree
  TTree* fmtr;

  //! BigRIPS timestamp
  unsigned long long fBRts;
  //! EURICA timestamp
  unsigned long long fEUts;
  //! bigrips data
  Beam* fbeam;
  //! eurica data
  EURICA* feurica;

  //! eurica event info
  TClonesArray *fEUeventinfo;
};
#endif
