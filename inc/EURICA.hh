#ifndef __EURICA_HH
#define __EURICA_HH
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>

#include "TObject.h"
#include "TVector3.h"
#include "TMath.h"
#include "EURICAdefs.h"
using namespace std;

/*!
  Container for the EURICA information
*/
class EURICAHit : public TObject {
public:
  //! default constructor
  EURICAHit(){
    Clear();
  }
  //! constructor with individual values
  EURICAHit(short id, double en, double time, unsigned short hitsadded, unsigned long long int ts){
    fID = id;
    fen = en;
    ftime = time;
    fhitsadded = hitsadded;
    fts = ts;
  }
  //! Clear the music information
  void Clear(Option_t *option = ""){
    fID = -1.;
    fen = sqrt(-1.);
    ftime = sqrt(-1.);
    fhitsadded = 0;
    fts = 0;
  }
  //! Set the detector ID
  void SetID(short id){
    if(id>0 && id <MAXNCRYSTAL-1)//id is from 1
      fID = id-1;
    fhitsadded++;
  }
  //! Set the energy
  void SetEnergy(double energy){fen = energy;}
  //! Set the time
  void SetTime(double time){ftime = time;}
  //! Set the raw ADC value
  void SetADC(int adc){fadc = adc;}
  //! Set the raw TDC value
  void SetTDC(int tdc){ftdc = tdc;}
  //! Set the timestamp
  void SetTimestamp(unsigned long long int ts){fts = ts;}

  //! Addback a hit
  void AddBackHit(EURICAHit* hit){
    if(fen<hit->GetEnergy()){
      cout << " error hits not sorted by energy" << endl;
      return;
    }
    fen+=hit->GetEnergy();
    fhitsadded+=hit->GetHitsAdded();
  }


  //! Get the ID
  short GetID(){return fID;}
  //! Get the energy
  double GetEnergy(){return fen;}
  //! Get the time
  double GetTime(){return ftime;}
  //! Get the timestamp
  unsigned long long int GetTimestamp(){return fts;}
  //! Get the raw ADC value
  int GetADC(){return fadc;}
  //! Get the raw TDC value
  int GetTDC(){return ftdc;}
  //! Get the number of hits that were added back to create one gamma
  unsigned short GetHitsAdded(){return fhitsadded;}
  
  //! Printing information 
  void Print(Option_t *option = "") const {
    cout << "ID " << fID;
    cout << "\tenergy " << fen;
    cout << "\ttime " << ftime;
    cout << "\ttimestamp " << fts;
    cout << "\thits added " << fhitsadded << endl;
    return;
  }

protected:
  //! detector number
  short fID;
  //! the energy lab system
  double fen;
  //! the time
  double ftime;
  //! the timestamp
  unsigned long long fts;
  //! the raw adc value
  int fadc;
  //! the raw tdc value
  int ftdc;
  //! how many hits were added back to create one gamma
  unsigned short fhitsadded;

  /// \cond CLASSIMP
  ClassDef(EURICAHit,1);
  /// \endcond
};
/*!
  Container for the EURICA information
*/
class EURICA : public TObject {
public:
  //! default constructor
  EURICA(){
    Clear();
  }
  //! Clear the music information
  void Clear(Option_t *option = ""){
    fmult = 0;
    fmultAB = 0;
    fhits.clear();
    fhitsAB.clear();
  }
  //! Add a hit
  void AddHit(EURICAHit* hit){
    fhits.push_back(hit);
    fmult++;
  }
  //! Add a hit after addback
  void AddHitAB(EURICAHit* hit){
    fhitsAB.push_back(hit);
    fmultAB++;
  }
  //! Set all hits
  void SetHits(vector<EURICAHit*> hits){
    fmult = hits.size();
    fhits = hits;
  }
  //! Set all hits after addback
  void SetABHits(vector<EURICAHit*> hits){
    fmultAB = hits.size();
    fhitsAB = hits;
  }
  //! Add more hits
  void AddHits(vector<EURICAHit*> hits){
    fmult += hits.size();
    for(vector<EURICAHit*>::iterator hit=hits.begin(); hit!=hits.end(); hit++){
      fhits.push_back(*hit);
    }
  }


  //! Returns the multiplicity of the event
  unsigned short GetMult(){return fmult;}
  //! Returns the whole vector of hits
  vector<EURICAHit*> GetHits(){return fhits;}
  //! Returns the hit number n
  EURICAHit* GetHit(unsigned short n){return fhits.at(n);}
  //! Returns the multiplicity of the event after addback
  int GetMultAB(){return fmultAB;}
  //! Returns the whole vector of hits after addback
  vector<EURICAHit*> GetHitsAB(){return fhitsAB;}
  //! Returns the hit number n after addback
  EURICAHit* GetHitAB(int n){return fhitsAB.at(n);}

  //! Printing information
  void Print(Option_t *option = "") const {
    cout << "multiplicity " << fmult << " event" << endl;
    for(unsigned short i=0;i<fhits.size();i++)
      fhits.at(i)->Print();
    cout << "after addback multiplicity " << fmultAB << endl;
    for(unsigned short i=0;i<fhitsAB.size();i++)
      fhitsAB.at(i)->Print();
  } 

  
protected:
  //! multiplicity
  unsigned short fmult;
  //! vector with the hits
  vector<EURICAHit*> fhits;
  //! multiplicity after addback
  unsigned short fmultAB;
  //! vector with the hits after addback
  vector<EURICAHit*> fhitsAB;

  /// \cond CLASSIMP
  ClassDef(EURICA,1);
  /// \endcond
};

/*!
  Compare two hits by their energies
*/
class HitComparer {
public:
  //! compares energies of the hits
  bool operator() ( EURICAHit *lhs, EURICAHit *rhs) {
    return (*lhs).GetEnergy() > (*rhs).GetEnergy();
  }
};
#endif
