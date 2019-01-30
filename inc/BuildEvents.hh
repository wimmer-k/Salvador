#ifndef __BUILDEVENTS_HH
#define __BUILDEVENTS_HH
#include <vector>
#include <algorithm>

#include "TTree.h"

#include "Beam.hh"
#include "WASABI.hh"


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
  void Init(TTree* brtr, TTree* watr);
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
  //! Read one entry from each tree
  bool ReadEach();
  
  
  //! Merge the data streams
  bool Merge();

  //! Set the last event
  int GetNEvents(){return fWAentries + fBRentries;};
  //! Close the event and write to tree
  void CloseEvent();
  //! Get the merged tree
  TTree* GetTree(){return fmtr;};
  
private:
  //! BigRIPS input tree
  TTree* fBRtr;
  //! WASABI input tree
  TTree* fWAtr;
  //! merged output tree
  TTree* fmtr;
  //! verbose level
  int fverbose;

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
  //! timestamp jumped in BigRIPS
  bool fBRtsjump;
  //! timestamp jumped in WASABI
  bool fWAtsjump;

  //! BigRIPS timestamp
  unsigned long long int fBRts;
  //! bigrips data
  Beam* fbeam;
  //! WASABI timestamp
  unsigned long long int fWAts;
  //! wasabi data
  WASABI* fwasabi;

  //! local copy of BigRIPS timestamp
  unsigned long long int flocalBRts;
  //! local copy of bigrips data
  Beam* flocalbeam;
  //! WASABI timestamp
  unsigned long long int flocalWAts;
  //! wasabi data
  WASABI* flocalwasabi;

  //! number of bigrips entries
  double fBRentries;
  //! number of wasabi entries
  double fWAentries;
  //! current bigrips entry
  unsigned int fBRentry;
  //! current wasabi entry
  unsigned int fWAentry;

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
