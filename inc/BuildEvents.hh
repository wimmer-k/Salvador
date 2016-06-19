#ifndef __BUILDEVENTS_HH
#define __BUILDEVENTS_HH
#include "TTree.h"
#include "TClonesArray.h"
#include "TArtEventInfo.hh"
#include "TArtGeCluster.hh"

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
  //! Set the window for event building
  void SetWindow(unsigned long long int window){fwindow = window;};
  //! Set the verbose level
  void SetVerbose(int verbose){fverbose = verbose;};
  //! Get the merged tree
  TTree* GetTree(){return fmtr;};
  //! Close the event and write to tree
  void CloseEvent();
  //! Read one entry from the EURICA tree
  bool ReadEURICA();
  //! Read one entry from the BigRIPS tree
  bool ReadBigRIPS();
private:
  //! BigRIPS input tree
  TTree* fBRtr;
  //! EURICA input tree
  TTree* fEUtr;
  //! merged output tree
  TTree* fmtr;
  //! verbose level
  int fverbose;

  //! number of bytes read
  int fnbytes;
  //! current timestamp
  unsigned long long fcurrentts;
  //! BigRIPS timestamp
  unsigned long long fBRts;
  //! EURICA timestamp
  unsigned long long fEUts;
  //! bigrips data
  Beam* fbeam;
  //! eurica data
  EURICA* feurica;

  //! number of bigrips entries
  double fBRentries;
  //! number of eurica entries
  double fEUentries;
  //! current bigrips entry
  unsigned int fBRentry;
  //! current eurica entry
  unsigned int fEUentry;

  //! eurica event info
  TClonesArray *fEUeventinfo;
  //! eurica data
  TClonesArray *fEUcluster;

  //! time window for eventbuilding
  unsigned long long fwindow;
};
#endif
