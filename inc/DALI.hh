#ifndef __DALI_HH
#define __DALI_HH
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>

#include "TObject.h"
#include "TVector3.h"
#include "TMath.h"
#include "DALIdefs.h"
using namespace std;

/*!
  Container for the DALI information
*/
class DALIHit : public TObject {
public:
  //! default constructor
  DALIHit(){
    Clear();
  }
  //! constructor with individual values
  DALIHit(short id, double en, double DCen, TVector3 pos, double time, double toffset, unsigned short hitsadded){
    fID = id;
    fpos = pos;
    fen = en;
    fDCen = DCen;
    ftime = time;
    ftoffset = toffset;
    fhitsadded =hitsadded;
  }
  //! Clear the music information
  void Clear(Option_t *option = ""){
    fID = -1.;
    fpos.SetXYZ(0,0,0);  
    fen = sqrt(-1.);
    fDCen = sqrt(-1.);
    ftime = sqrt(-1.);
    ftoffset = sqrt(-1.);
    fhitsadded =0;
  }
  //! Set the detector ID
  void SetID(short id){
    if(id>0 && id <MAXNCRYSTAL-1)//id is from 1
      fID = id-1;
    fhitsadded++;
  }
  //! Set the position by vector
  void SetPos(TVector3 pos){fpos = pos;}
  //! Set the position by coordinates
  void SetPos(double x, double y, double z){
    fpos.SetXYZ(x,y,z);
  }
  //! Set the position by vector
  void SetPos(vector<double> r){
    if(r.size()==3)
      fpos.SetXYZ(r.at(0),r.at(1),r.at(2));
    else
      cout << "error in DALI.SetPos()" << endl;
  }
  //! Set the energy
  void SetEnergy(double energy){fen = energy;}
  //! Set the Doppler corrected energy
  void SetDCEnergy(double dcen){fDCen = dcen;}
  //! Set the time
  void SetTime(double time){ftime = time;}
  //! Set the time after offset correction
  void SetTOffset(double toffset){ftoffset = toffset;}
  //! Set the raw ADC value
  void SetADC(int adc){fadc = adc;}
  //! Set the raw TDC value
  void SetTDC(int tdc){ftdc = tdc;}

  //! Addback a hit
  void AddBackHit(DALIHit* hit){
    if(fen<hit->GetEnergy()){
      cout << " error hits not sorted by energy" << endl;
      return;
    }
    fen+=hit->GetEnergy();
    fhitsadded+=hit->GetHitsAdded();
  }


  //! Get the ID
  short GetID(){return fID;}
  //! Get the position vector
  TVector3 GetPos(){return fpos;}
  //! Get the energy
  double GetEnergy(){return fen;}
  //! Get the Doppler corrected energy
  double GetDCEnergy(){return fDCen;}
  //! Get the time
  double GetTime(){return ftime;}
  //! Get the time after offset correction
  double GetTOffset(){return ftoffset;}
  //! Get the raw ADC value
  int GetADC(){return fadc;}
  //! Get the raw TDC value
  int GetTDC(){return ftdc;}
  //! Get the number of hits that were added back to create one gamma
  unsigned short GetHitsAdded(){return fhitsadded;}
  

  //! Apply the Doppler correction with the given beta, assuming motion in the +z direction.
  void DopplerCorrect(double beta){
    fDCen = fen/sqrt(1-beta*beta)*(1-beta*cos(fpos.Theta()));
  }
  //! Printing information 
  void Print(Option_t *option = "") const {
    cout << "ID " << fID;
    cout << "\tenergy " << fen;
    cout << "\tDCenergy " << fDCen;
    cout << "\ttheta " << fpos.Theta()*180./TMath::Pi();
    cout << "\tphi " << fpos.Phi()*180./TMath::Pi();
    cout << "\ttime " << ftime;
    cout << "\ttoffset " << ftoffset;
    cout << "\thits added " << fhitsadded << endl;
    return;
  }

protected:
  //! detector number
  short fID;
  //! the position
  TVector3 fpos;
  //! the energy lab system
  double fen;
  //! the Doppler corrected energy
  double fDCen;
  //! the time
  double ftime;
  //! the time after offset correction
  double ftoffset;
  //! the raw adc value
  int fadc;
  //! the raw tdc value
  int ftdc;
  //! how many hits were added back to create one gamma
  unsigned short fhitsadded;

  /// \cond CLASSIMP
  ClassDef(DALIHit,1);
  /// \endcond
};
/*!
  Container for the DALI information
*/
class DALI : public TObject {
public:
  //! default constructor
  DALI(){
    Clear();
  }
  //! Clear the music information
  void Clear(Option_t *option = ""){
    fmult = 0;
    fmultAB = 0;
    for(vector<DALIHit*>::iterator hit=fhits.begin(); hit!=fhits.end(); hit++){
      delete *hit;
    }
    fhits.clear();
    for(vector<DALIHit*>::iterator hitab=fhitsAB.begin(); hitab!=fhitsAB.end(); hitab++){
      delete *hitab;
    }
    fhitsAB.clear();
  }
  //! Add a hit
  void AddHit(DALIHit* hit){
    fhits.push_back(hit);
    fmult++;
  }
  //! Add a hit after addback
  void AddHitAB(DALIHit* hit){
    fhitsAB.push_back(hit);
    fmultAB++;
  }
  //! Set all hits
  void SetHits(vector<DALIHit*> hits){
    fmult = hits.size();
    fhits = hits;
  }
  //! Set all hits after addback
  void SetABHits(vector<DALIHit*> hits){
    fmultAB = hits.size();
    fhitsAB = hits;
  }
  //! Do the Doppler correction
  void DopplerCorrect(double beta){
    for(vector<DALIHit*>::iterator hit=fhits.begin(); hit!=fhits.end(); hit++){
      (*hit)->DopplerCorrect(beta);
    }
    for(vector<DALIHit*>::iterator hit=fhitsAB.begin(); hit!=fhitsAB.end(); hit++){
      (*hit)->DopplerCorrect(beta);
    }
  }
  //! Returns the multiplicity of the event
  unsigned short GetMult(){return fmult;}
  //! Returns the whole vector of hits
  vector<DALIHit*> GetHits(){return fhits;}
  //! Returns the hit number n
  DALIHit* GetHit(unsigned short n){return fhits.at(n);}
  //! Returns the multiplicity of the event after addback
  int GetMultAB(){return fmultAB;}
  //! Returns the whole vector of hits after addback
  vector<DALIHit*> GetHitsAB(){return fhitsAB;}
  //! Returns the hit number n after addback
  DALIHit* GetHitAB(int n){return fhitsAB.at(n);}

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
  vector<DALIHit*> fhits;
  //! multiplicity after addback
  unsigned short fmultAB;
  //! vector with the hits after addback
  vector<DALIHit*> fhitsAB;

  /// \cond CLASSIMP
  ClassDef(DALI,1);
  /// \endcond
};

/*!
  Compare two hits by their energies
*/
class HitComparer {
public:
  //! compares energies of the hits
  bool operator() ( DALIHit *lhs, DALIHit *rhs) {
    return (*lhs).GetEnergy() > (*rhs).GetEnergy();
  }
};
#endif
