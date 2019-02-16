#ifndef __WASABI_HH
#define __WASABI_HH
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <algorithm>

#include "TObject.h"
#include "WASABIdefs.h"
using namespace std;
class WASABIHitComparer;

/*!
  Container for the WASABI Raw ADC information
*/
class WASABIRawADC : public TObject {
public:
  //! default constructor
  WASABIRawADC(){
    Clear();
  }
  //! useful constructor
  WASABIRawADC(short adc, short ch, short val){
    fadc = adc;
    fch = ch;
    fval = val;
  }
  //! Clear the ADC information
  void Clear(Option_t *option = ""){
    fadc = sqrt(-1.);
    fch = sqrt(-1.);
    fval = sqrt(-1.);
  }
  //! Set the ADC number
  void SetADC(short adc){ fadc = adc;}
  //! Set the ADC channel
  void SetChan(short ch){ fch = ch;}
  //! Set the ADC value
  void SetVal(short val){ fval = val;}

  //! Get the ADC number
  short GetADC(){ return fadc;}
  //! Get the ADC channel
  short GetChan(){ return fch;}
  //! Get the ADC value
  short GetVal(){ return fval;}
  //! Printing information 
  void Print(Option_t *option = "") const {
    cout << "adc " << fadc;
    cout << "\tch " << fch;
    cout << "\tval " << fval << endl;
    return;
  }
protected:
  //! adc number
  short fadc;
  //! adc channel
  short fch;
  //! adc value
  short fval;
  /// \cond CLASSIMP
  ClassDef(WASABIRawADC,1);
  /// \endcond
};

/*!
  Container for the WASABI Raw TDC information
*/
class WASABIRawTDC : public TObject {
public:
  //! default constructor
  WASABIRawTDC(){
    Clear();
  }
  //! useful constructor
  WASABIRawTDC(short tdc, short ch, short val){
    ftdc = tdc;
    fch = ch;
    fval.push_back(val);
  }
  //! add time to a channel
  void AddRawTDC(short val){
    fval.push_back(val);
  }
  //! Clear the TDC information
  void Clear(Option_t *option = ""){
    ftdc = sqrt(-1.);
    fch = sqrt(-1.);
    fval.clear();
  }
  //! Printing information 
  void Print(Option_t *option = "") const {
    cout << "tdc " << ftdc;
    cout << "\tch " << fch;
    cout << "\tval " << fval.size();
    for(unsigned short i=0; i<fval.size();i++){
      cout << ", " << fval.at(i);
    }
    cout << endl;
    return;
  }
  //! Get the TDC number
  short GetTDC(){ return ftdc;}
  //! Get the TDC channel
  short GetChan(){ return fch;}
  //! Get the TDC values
  vector<short> GetVal(){ return fval;}
  //! Get a TDC value
  short GetVal(short n){ return fval.at(n);}
  
protected:
  //! tdc number
  short ftdc;
  //! tdc channel
  short fch;
  //! tdc value
  vector<short> fval;
  /// \cond CLASSIMP
  ClassDef(WASABIRawTDC,1);
  /// \endcond
};

/*!
  Container for the WASABIRaw information
*/
class WASABIRaw : public TObject {
public:
  //! default constructor
  WASABIRaw(){
    Clear();
  }
  //! Clear the WASABI raw information
  void Clear(Option_t *option = ""){
    fADCmult = 0;
    for(vector<WASABIRawADC*>::iterator adc=fadcs.begin(); adc!=fadcs.end(); adc++){
      delete *adc;
    }
    fadcs.clear();
    fTDCmult = 0;
    for(vector<WASABIRawTDC*>::iterator tdc=ftdcs.begin(); tdc!=ftdcs.end(); tdc++){
      delete *tdc;
    }
    ftdcs.clear();
  }
  //! Add an adc
  void AddADC(WASABIRawADC* adc){
    fadcs.push_back(adc);
    fADCmult++;
  }
  //! Add a tdc
  void AddTDC(WASABIRawTDC* tdc){
    for(vector<WASABIRawTDC*>::iterator stored=ftdcs.begin(); stored!=ftdcs.end(); stored++){
      if(tdc->GetTDC() == (*stored)->GetTDC() && tdc->GetChan() == (*stored)->GetChan()){
	(*stored)->AddRawTDC(tdc->GetVal(0));
	return;
      }
    }
    ftdcs.push_back(tdc);
    fTDCmult++;
  }
  
  //! Set all adcs
  void SetADCs(vector<WASABIRawADC*> adcs){
    fADCmult = adcs.size();
    fadcs = adcs;
  }
  //! Returns the ADCmultiplicity of the event
  unsigned short GetADCmult(){return fADCmult;}
  //! Returns the whole vector of adcs
  vector<WASABIRawADC*> GetADCs(){return fadcs;}
  //! Returns the adc number n
  WASABIRawADC* GetADC(unsigned short n){return fadcs.at(n);}
  
  //! Set all tdcs
  void SetTDCs(vector<WASABIRawTDC*> tdcs){
    fTDCmult = tdcs.size();
    ftdcs = tdcs;
  }
  //! Returns the TDCmultiplicity of the event
  unsigned short GetTDCmult(){return fTDCmult;}
  //! Returns the whole vector of tdcs
  vector<WASABIRawTDC*> GetTDCs(){return ftdcs;}
  //! Returns the tdc number n
  // param n the number of the TDC hit
  WASABIRawTDC* GetTDC(unsigned short n){return ftdcs.at(n);}
  //! Returns the hit in TDC module tdc and channel ch
  WASABIRawTDC* GetTDC(short tdc, short ch){
    for(vector<WASABIRawTDC*>::iterator stored=ftdcs.begin(); stored!=ftdcs.end(); stored++){
      if(tdc == (*stored)->GetTDC() && ch == (*stored)->GetChan()){
	return (*stored);
      }
    }
    cout << "warning TDC " << tdc << " channel " << ch << " not found!" << endl;
    return new WASABIRawTDC();
  }
  
  //! Printing information
  void Print(Option_t *option = "") const {
    cout << "ADC multiplicity " << fADCmult << " event" << endl;
    for(unsigned short i=0;i<fadcs.size();i++)
      fadcs.at(i)->Print();
    cout << "TDC multiplicity " << fTDCmult << " event" << endl;
    for(unsigned short i=0;i<ftdcs.size();i++)
      ftdcs.at(i)->Print();
  } 
  
protected:
  //! ADCmultiplicity
  unsigned short fADCmult;
  //! vector with the adcs
  vector<WASABIRawADC*> fadcs;
  //! TDCmultiplicity
  unsigned short fTDCmult;
  //! vector with the tdcs
  vector<WASABIRawTDC*> ftdcs;

  /// \cond CLASSIMP
  ClassDef(WASABIRaw,1);
  /// \endcond
};


/*!
  Container for the WASABI Hit information
*/
class WASABIHit : public TObject {
public:
  //! default constructor
  WASABIHit(){
    Clear();
  }
  //! useful constructor
  WASABIHit(short strip, double en, bool iscal){
    fstrip = strip;
    fen = en;
    fiscal = iscal;
    ftime.clear();
    fhitsadded = 1;
  }
  //! Clear the ADC information
  void Clear(Option_t *option = ""){
    fstrip = sqrt(-1.);
    fen = sqrt(-1.);
    ftime.clear();
    fiscal = false;
    fhitsadded = 0;
  }
  //! Set the time
  void SetTime(double timeval){ ftime.push_back(timeval);}
  
  //! Get the strip number
  short GetStrip(){ return fstrip;}
  //! Get the energy
  double GetEn(){ return fen;}
  //! Get the time values
  vector<double> GetTime(){ return ftime;}
  //! Get the first time value
  double GetTime0(){
    if(ftime.size()>0)
      return ftime.at(0);
    return sqrt(-1);
  }
  //! Get the number of hits that were added back to create one hit
  unsigned short GetHitsAdded(){return fhitsadded;}
  //! Is is calibrated
  bool IsCal(){return fiscal;}
  //! Printing information 
  //! Addback a hit
  void AddBackHit(WASABIHit* hit){
    if(fen<hit->GetEn()){
      cout << " error hits not sorted by energy" << endl;
      return;
    }
    fen+=hit->GetEn();
    fhitsadded+=hit->GetHitsAdded();
  }
  void Print(Option_t *option = "") const {
    cout << "strip = " << fstrip;
    cout << "\ten = " << fen;
    if(fiscal)
      cout << " (calibrated) ";
    else
      cout << " (raw) ";
    cout << "\ttime " << ftime.size() << " values, first one = " << ftime.at(0);
    if(fhitsadded>1)
      cout << "\thits added " << fhitsadded << endl;
    else
      cout << endl;
    return;
  }
protected:
  //! position, strip number
  short fstrip;
  //! energy, calibrated
  double fen;
  //! timing
  vector<double> ftime;
  //! is it calibrated
  bool fiscal;
   //! how many hits were added back to create one hit
  unsigned short fhitsadded;

  /// \cond CLASSIMP
  ClassDef(WASABIHit,1);
  /// \endcond
};


/*!
  Container for the WASABI DSSSD information
*/
class DSSSD : public TObject {
public:
  //! default constructor
  DSSSD(){
    fdsssd = -1;
    Clear();
  }
  //! constructor
  DSSSD(short dsssdnr){
    fdsssd = dsssdnr;
    Clear();
  }
  //! Clear the DSSSD information
  void Clear(Option_t *option = ""){
    fmultX = 0;
    fvetoX = false;
    for(vector<WASABIHit*>::iterator hit=fhitsX.begin(); hit!=fhitsX.end(); hit++){
      delete *hit;
    }
    fhitsX.clear();
    fimplantX = -1;
    
    fmultY = 0;
    fvetoY = false;
    for(vector<WASABIHit*>::iterator hit=fhitsY.begin(); hit!=fhitsY.end(); hit++){
      delete *hit;
    }
    fhitsY.clear();
    fimplantY = -1;

    fmultABX = 0;
    for(vector<WASABIHit*>::iterator hit=fhitsABX.begin(); hit!=fhitsABX.end(); hit++){
      delete *hit;
    }
    fhitsABX.clear();
    
    fmultABY = 0;
    for(vector<WASABIHit*>::iterator hit=fhitsABY.begin(); hit!=fhitsABY.end(); hit++){
      delete *hit;
    }
    fhitsABY.clear();
  }
  //! Clear the addback  information
  void ClearAddback(Option_t *option = ""){
    fmultABX = 0;
    for(vector<WASABIHit*>::iterator hit=fhitsABX.begin(); hit!=fhitsABX.end(); hit++){
      delete *hit;
    }
    fhitsABX.clear();
    
    fmultABY = 0;
    for(vector<WASABIHit*>::iterator hit=fhitsABY.begin(); hit!=fhitsABY.end(); hit++){
      delete *hit;
    }
    fhitsABY.clear();
  }
  //! setting the dsssd number
  void SetDSSSD(short dsssd){fdsssd = dsssd;}
  //! Add a hit in X
  void AddHitX(WASABIHit* hit){
    fhitsX.push_back(hit);
    fmultX++;
  }
  //! Add a hit in Y
  void AddHitY(WASABIHit* hit){
    fhitsY.push_back(hit);
    fmultY++;
  }
  
  //! Set hits in X
  void SetHitsX(vector<WASABIHit*> hits){
    fhitsX = hits;
    fmultX = hits.size();
  }
  //! Set hits in Y
  void SetHitsY(vector<WASABIHit*> hits){
    fhitsY = hits;
    fmultY = hits.size();
  }
  //! Set a veto on X
  void SetVetoX(){fvetoX = true;}
  //! Set a veto on Y
  void SetVetoY(){fvetoY = true;}
  
  //! Set implantation point X
  void SetImplantX(int strip){fimplantX = strip;}
  //! Set implantation point Y
  void SetImplantY(int strip){fimplantY = strip;}

   //! Add an addback hit in X
  void AddABHitX(WASABIHit* hit){
    fhitsABX.push_back(hit);
    fmultABX++;
  }
  //! Add an addback hit in Y
  void AddABHitY(WASABIHit* hit){
    fhitsABY.push_back(hit);
    fmultABY++;
  }
 //! Returns the DSSSD number
  short GetDSSSD(){return fdsssd;}
  
  //! Returns the X multiplicity of the event
  unsigned short GetMultX(){return fmultX;}
  //! Returns the whole vector of hits in X
  vector<WASABIHit*> GetHitsX(){return fhitsX;}
  //! Returns the X hit number n
  WASABIHit* GetHitX(unsigned short n){return fhitsX.at(n);}
  //! Returns the hit at strip number
  WASABIHit* GetStripHitX(short s){
    if(s<0 || s > NXSTRIPS){
      cout << "looking for X strip number " << s <<", not found!" << endl;
      return new WASABIHit();
    }
    for(vector<WASABIHit*>::iterator hit=fhitsX.begin(); hit!=fhitsX.end(); hit++){
      if((*hit)->GetStrip() ==s)
	return (*hit);
    }
    cout << "looking for X strip number " << s <<", not found!" << endl;
    return new WASABIHit();    
  }
  //! Add the timing information to strip s
  void SetStripTimeX(short s, double timeval){
    if(s<0 || s > NXSTRIPS){
      cout << "looking for X strip number " << s <<", not found!" << endl;
      return;
    }
    for(vector<WASABIHit*>::iterator hit=fhitsX.begin(); hit!=fhitsX.end(); hit++){
      if((*hit)->GetStrip() ==s)
	(*hit)->SetTime(timeval);
    }
    //cout << "looking for X strip number " << s <<", not found!" << endl;
    return;    
  }
  
  //! Returns the X multiplicity of the event after addback
  unsigned short GetMultABX(){return fmultABX;}
  //! Returns the whole vector of addback hits in X
  vector<WASABIHit*> GetHitsABX(){return fhitsABX;}
  //! Returns the addback X hit number n
  WASABIHit* GetHitABX(unsigned short n){return fhitsABX.at(n);}
  //! Returns the addback hit at strip number
  WASABIHit* GetStripHitABX(short s){
    if(s<0 || s > NXSTRIPS){
      cout << "looking for addback X strip number " << s <<", not found!" << endl;
      return new WASABIHit();
    }
    for(vector<WASABIHit*>::iterator hit=fhitsABX.begin(); hit!=fhitsABX.end(); hit++){
      if((*hit)->GetStrip() ==s)
	return (*hit);
    }
    cout << "looking for addback X strip number " << s <<", not found!" << endl;
    return new WASABIHit();    
  }
  
  //! Returns the Y multiplicity of the event
  unsigned short GetMultY(){return fmultY;}
  //! Returns the whole vector of hits in Y
  vector<WASABIHit*> GetHitsY(){return fhitsY;}
  //! Returns the Y hit number n
  WASABIHit* GetHitY(unsigned short n){return fhitsY.at(n);}
  //! Returns the hit at strip number
  WASABIHit* GetStripHitY(short s){
    if(s<0 || s > NYSTRIPS){
      cout << "looking for Y strip number " << s <<", not found!" << endl;
      return new WASABIHit();
    }
    for(vector<WASABIHit*>::iterator hit=fhitsY.begin(); hit!=fhitsY.end(); hit++){
      if((*hit)->GetStrip() ==s)
	return (*hit);
    }
    cout << "looking for Y strip number " << s <<", not found!" << endl;
    return new WASABIHit();    
  }
  //! Add the timing information to strip s
  void SetStripTimeY(short s, double timeval){
    if(s<0 || s > NYSTRIPS){
      cout << "looking for Y strip number " << s <<", not found!" << endl;
      return;
    }
    for(vector<WASABIHit*>::iterator hit=fhitsY.begin(); hit!=fhitsY.end(); hit++){
      if((*hit)->GetStrip() ==s)
	(*hit)->SetTime(timeval);
    }
    //cout << "looking for Y strip number " << s <<", not found!" << endl;
    return;    
  }

  //! Returns the Y multiplicity of the event after addback
  unsigned short GetMultABY(){return fmultABY;}
  //! Returns the whole vector of addback hits in Y
  vector<WASABIHit*> GetHitsABY(){return fhitsABY;}
  //! Returns the addback Y hit number n
  WASABIHit* GetHitABY(unsigned short n){return fhitsABY.at(n);}
  //! Returns the addback hit at strip number
  WASABIHit* GetStripHitABY(short s){
    if(s<0 || s > NYSTRIPS){
      cout << "looking for addback Y strip number " << s <<", not found!" << endl;
      return new WASABIHit();
    }
    for(vector<WASABIHit*>::iterator hit=fhitsABY.begin(); hit!=fhitsABY.end(); hit++){
      if((*hit)->GetStrip() ==s)
	return (*hit);
    }
    cout << "looking for addback Y strip number " << s <<", not found!" << endl;
    return new WASABIHit();    
  }
  



  //! sort the hits by energy, high to low
  vector<WASABIHit*> Sort(vector<WASABIHit*> hits);
  //! sort the hits by energy, low to high
  vector<WASABIHit*> Revert(vector<WASABIHit*> hits);
  //! addback
  void Addback();
  //! check if addback
  bool Addback(WASABIHit* hit0, WASABIHit* hit1);
  
  //! Is vetoed in X
  bool IsVetoX(){return fvetoX;}
  //! Is vetoed in Y
  bool IsVetoY(){return fvetoY;}
  
  //! Implantation X
  int ImplantX(){return fimplantX;}
  //! Implantation Y
  int ImplantY(){return fimplantY;}

  //! Printing information 
  void Print(Option_t *option = "") const {
    cout << "DSSSD number " << fdsssd << endl;
    if(fvetoX)
      cout << "veto on X strips " << endl; 
    if(fvetoY)
      cout << "veto on Y strips " << endl; 
    cout << "X multiplicity " << fmultX << " event" << endl;
    for(unsigned short i=0;i<fhitsX.size();i++)
      fhitsX.at(i)->Print();
    cout << "X AB multiplicity " << fmultABX << " event" << endl;
    for(unsigned short i=0;i<fhitsABX.size();i++)
      fhitsABX.at(i)->Print();
    cout << "Y multiplicity " << fmultY << " event" << endl;
    for(unsigned short i=0;i<fhitsY.size();i++)
      fhitsY.at(i)->Print();
    cout << "Y AB multiplicity " << fmultABY << " event" << endl;
    for(unsigned short i=0;i<fhitsABY.size();i++)
      fhitsABY.at(i)->Print();
  }

protected:
  //! DSSSD number
  short fdsssd;
  //! HIT multiplicity in X
  unsigned short fmultX;
  //! vector with the X hits
  vector<WASABIHit*> fhitsX;
  //! HIT multiplicity in Y
  unsigned short fmultY;
  //! vector with the Y hits
  vector<WASABIHit*> fhitsY;
  //! HIT multiplicity after addback in X
  unsigned short fmultABX;
  //! vector with the addback X hits
  vector<WASABIHit*> fhitsABX;
  //! HIT multiplicity after addback in Y
  unsigned short fmultABY;
  //! vector with the addback Y hits
  vector<WASABIHit*> fhitsABY;
  //! veto in X
  bool fvetoX;
  //! veto in Y
  bool fvetoY;
  //! implantation point X
  int fimplantX;
  //! implantation point Y
  int fimplantY;

  /// \cond CLASSIMP
  ClassDef(DSSSD,1);
  /// \endcond
};

/*!
  Container for the WASABI information
*/
class WASABI : public TObject {
public:
  //! default constructor
  WASABI(){
    for(int i=0; i<NDSSSD; i++)
      fdsssd[i] = new DSSSD(i);    
    //Clear();
  }
  //! Clear the wasabi information
  void Clear(Option_t *option = ""){
    for(int i=0; i<NDSSSD; i++){
      fdsssd[i]->Clear();
      fdsssd[i]->SetDSSSD(i);
    }
  }
  //! return the DSSSD information
  DSSSD* GetDSSSD(int i){return fdsssd[i];}
  //! printing information
  void Print(Option_t *option = "") const {
    for(int i=0; i<NDSSSD; i++)
      fdsssd[i]->Print();
  }
protected:
  //! sub DSSSD
  DSSSD* fdsssd[NDSSSD];
  /// \cond CLASSIMP
  ClassDef(WASABI,1);
  /// \endcond
};

/*!
  Compare two hits by their energies
*/
class WASABIHitComparer {
public:
  //! compares energies of the hits
  bool operator() ( WASABIHit *lhs, WASABIHit *rhs) {
    return (*lhs).GetEn() > (*rhs).GetEn();
  }
};
  
#endif
