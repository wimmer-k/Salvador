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
  Container for the PPAC information
*/
class PPAC : public TObject {
public:
  //! default constructor
  PPAC(){
    Clear();
  };
  //! Clear the music information
  void Clear(Option_t *option = ""){
    fx = sqrt(-1.);
    fy = sqrt(-1.);
    ftsumx = sqrt(-1.);
    ftsumy = sqrt(-1.);
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
  void Set(double x, double y, double tsumx, double tsumy){
    fx = x;
    fy = y;
    ftsumx = tsumx;
    ftsumy = tsumy;
  }

  //! Get the x position
  double GetX(){return fx;}
  //! Get the y position
  double GetY(){return fy;}
  //! Get the timing sum x
  double GetTsumX(){return ftsumx;}
  //! Get the timing sum y
  double GetTsumY(){return ftsumy;}
  
protected:
  //! x position
  double fx;
  //! y position
  double fy;
  //! timing sum x
  double ftsumx;
  //! timing sum y
  double ftsumy;

  /// \cond CLASSIMP
  ClassDef(PPAC,1);
  /// \endcond
};
#endif
