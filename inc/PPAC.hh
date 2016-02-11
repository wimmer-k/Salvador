#ifndef __PPAC_HH
#define __PPAC_HH
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>

#include "TObject.h"
#include "PPACdefs.h"
using namespace std;

/*!
  Container for the information of a single PPAC
*/
class SinglePPAC : public TObject {
public:
  //! default constructor
  SinglePPAC(){
    Clear();
  };
  //! Clear the ppac information
  void Clear(Option_t *option = ""){
    fID = -1;
    fx = sqrt(-1.);
    fy = sqrt(-1.);
    ftsumx = sqrt(-1.);
    ftsumy = sqrt(-1.);
  }
  //! Set the ID
  void SetID(short id){
    if(id>0 && id <NPPACS+1) // id runs from 1
      fID = id-1; //fID runs from 0
  }
  //! Set the x position
  void SetX(double x){fx = x;}
  //! Set the y position
  void SetY(double y){fy = y;}
  //! Set the timing sum x
  void SetTsumX(double tsumx){ftsumx = tsumx;}
  //! Set the timing sum y
  void SetTsumY(double tsumy){ftsumy = tsumy;}
  //! Set the position
  void SetXY(double x, double y){
    fx = x;
    fy = y;
  }
  //! Set the timing sum
  void SetTsumXY(double tsumx, double tsumy){
    ftsumx = tsumx;
    ftsumy = tsumy;
  }
  //! Set everything
  void Set(short id, double x, double y, double tsumx, double tsumy){
    fID = id;
    fx = x;
    fy = y;
    ftsumx = tsumx;
    ftsumy = tsumy;
  }

  //! Get the ID
  short GetID(){return fID;}
  //! Get the x position
  double GetX(){return fx;}
  //! Get the y position
  double GetY(){return fy;}
  //! Get the timing sum x
  double GetTsumX(){return ftsumx;}
  //! Get the timing sum y
  double GetTsumY(){return ftsumy;}
  
protected:
  //! ID
  short fID;
  //! x position
  double fx;
  //! y position
  double fy;
  //! timing sum x
  double ftsumx;
  //! timing sum y
  double ftsumy;

  /// \cond CLASSIMP
  ClassDef(SinglePPAC,1);
  /// \endcond
};

/*!
  Container for the PPAC information
*/
class PPAC : public TObject {
public:
  //! default constructor
  PPAC(){
    Clear();
  };
  //! Clear all ppac information
  void Clear(Option_t *option = ""){
    fnppacs = 0;
    for(vector<SinglePPAC*>::iterator ppac=fppacs.begin(); ppac!=fppacs.end(); ppac++){
      delete *ppac;
    }
    fppacs.clear();
  }
  //! Add a ppac
  void AddPPAC(SinglePPAC* ppac){
    fppacs.push_back(ppac);
    fnppacs++;
  }
  //! Returns the number of hit ppacs
  unsigned short GetN(){return fnppacs;}
  //! Returns the whole vector of ppacs
  vector<SinglePPAC*> GetPPACS(){return fppacs;}
  //! Returns the ppacs number n
  SinglePPAC* GetPPAC(unsigned short n){return fppacs.at(n);}

protected:
  //! number of ppacs hit
  unsigned short fnppacs;
  //! vector of ppacs
  vector<SinglePPAC*> fppacs;

  /// \cond CLASSIMP
  ClassDef(PPAC,1);
  /// \endcond
};
#endif
