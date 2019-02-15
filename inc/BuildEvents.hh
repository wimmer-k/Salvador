#ifndef __BUILDEVENTS_HH
#define __BUILDEVENTS_HH
#include <vector>
#include <algorithm>

#include "TTree.h"
#include "TClonesArray.h"
#include "TArtEventInfo.hh"
#include "TArtGeCluster.hh"

#include "Beam.hh"
#include "FocalPlane.hh"
#include "WASABI.hh"
#include "EURICA.hh"
#include "Globaldefs.h"


/*!
  A container to keep track of the timestamps and corresponding detectors
*/
struct detector{
  //! timestamp of the detector hit
  unsigned long long int TS;
  //! ID for the detector, 0 BigRIPS, 1 WASABI
  int ID;
};

/*!
  A class for building BigRIPS and WASABI combined events
*/
class BuildEvents {
public:
  //! Default constructor
  BuildEvents(){};
  //! Initialize trees
  void Init(TTree* brtr, TTree* watr, TTree* eutr);
  //! Set the window for event building
  void SetWindow(unsigned long long int window){fwindow = window;};
  //! Set coincidence mode
  void SetCoincMode(int mode){fmode = mode;};
  //! Set the verbose level
  void SetVerbose(int verbose){fverbose = verbose;};
  //! Set the last event
  void SetLastEvent(int lastevent){flastevent = lastevent;};
  //! Read one entry from the WASABI tree
  bool ReadWASABI();
  //! Read one entry from the BigRIPS tree
  bool ReadBigRIPS();
  //! Read one entry from the EURICA tree
  bool ReadEURICA();
  //! Read one entry from each tree
  bool ReadEach();
  
  
  //! Merge the data streams
  bool Merge();

  //! Set the last event
  int GetNEvents(){return fWAentries + fBRentries + fEUentries;};
  //! Close the event and write to tree
  void CloseEvent();
  //! Get the merged tree
  TTree* GetTree(){return fmtr;};
  
private:
  //! BigRIPS input tree
  TTree* fBRtr;
  //! WASABI input tree
  TTree* fWAtr;
  //! EURICA input tree
  TTree* fEUtr;
  //! merged output tree
  TTree* fmtr;
  //! verbose level
  int fverbose;
  //! hasBR
  bool fhasBR;
  //! hasWA
  bool fhasWA;
  //! hasEU
  bool fhasEU;
  
  //! list of detector hits
  vector<detector*> fdetectors;

  //! number of events to be read
  int flastevent;

  //! number of bytes read
  int fnbytes;
  //! current timestamp
  unsigned long long int fcurrentts;
  //! last read BigRIPS timestamp
  unsigned long long int flastBRts;
  //! last read WASABI timestamp
  unsigned long long int flastWAts;
  //! last read EURICA timestamp
  unsigned long long int flastEUts;
  //! timestamp jumped in BigRIPS
  bool fBRtsjump;
  //! timestamp jumped in WASABI
  bool fWAtsjump;
  //! timestamp jumped in EURICA
  bool fEUtsjump;

  //! BigRIPS timestamp
  unsigned long long int fBRts;
  //! bigrips data
  Beam* fbeam;
  //! bigrips focal plane information
  FocalPlane* ffp[NFPLANES];
  //! WASABI timestamp
  unsigned long long int fWAts;
  //! wasabi data
  WASABI* fwasabi;
  //! EURICA timestamp
  unsigned long long int fEUts;
  //! eurica data
  EURICA* feurica;

  //! local copy of BigRIPS timestamp
  unsigned long long int flocalBRts;
  //! local copy of bigrips data
  Beam* flocalbeam;
  //! local copy of bigrips focal plane information
  FocalPlane* flocalfp[NFPLANES];
  //! WASABI timestamp
  unsigned long long int flocalWAts;
  //! wasabi data
  WASABI* flocalwasabi;
  //! EURICA timestamp
  unsigned long long int flocalEUts;
  //! eurica data
  EURICA* flocaleurica;

  //! number of bigrips entries
  double fBRentries;
  //! number of wasabi entries
  double fWAentries;
  //! number of eurica entries
  double fEUentries;
  //! current bigrips entry
  unsigned int fBRentry;
  //! current wasabi entry
  unsigned int fWAentry;
  //! current eurica entry
  unsigned int fEUentry;

  //! eurica event info
  TClonesArray *fEUeventinfo;
  //! eurica data
  TClonesArray *fEUcluster;
  //! eurica AB data
  TClonesArray *fEUclusterAB;

  //! time window for eventbuilding
  unsigned long long fwindow;
  //! modus for writing the merged data: 0 all, 1 only isomer (BR and WA)
  int fmode;
  
};

/*!
  A class for sorting the detectors in time
*/
class TSComparer {
public:
  //! comparator
  bool operator() ( detector *lhs, detector *rhs) {
    return (*rhs).TS > (*lhs).TS;
  }
};
#endif
