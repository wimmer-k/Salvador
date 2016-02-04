#ifndef __BEAM_HH
#define __BEAM_HH
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>

#include "TObject.h"

using namespace std;


/*!
  Container for the full beam, tof, beta and pid information
*/
class Beam : public TObject {
public:
  //! default constructor
  Beam(){
    Clear();
  };
  //! Clear all information
  void Clear(Option_t *option = ""){
    for(unsigned short j=0;j<6;j++){
      faoq[j] = sqrt(-1.);
      faoqc[j] = sqrt(-1.);
      fzet[j] = sqrt(-1.);
      fzetc[j] = sqrt(-1.);
    }    
    for(unsigned short j=0;j<3;j++){
      ftof[j] = sqrt(-1.);
      fbeta[j] = sqrt(-1.);
    }
    for(unsigned short j=0;j<4;j++){
      fdelta[j] = sqrt(-1.);
    }
  }
  //! Set the A/Q ratio
  void SetAQ(unsigned short j, double aoq){
    if(j<0 || j>5) return;
    faoq[j] = aoq;
  }
  //! Set the Z number
  void SetZ(unsigned short j, double zet){    
    if(j<0 || j>5) return;
    fzet[j] = zet;
  }
  //! Set both A/Q and Z
  void SetAQZ(unsigned short j, double aoq, double zet){
    if(j<0 || j>5) return;
    faoq[j] = aoq;
    fzet[j] = zet;
  }
  //! Set the time-of-flight
  void SetTOF(unsigned short j, double tof){    
    if(j<0 || j>2) return;
    ftof[j] = tof;
  }
  //! Set the beta
  void SetBeta(unsigned short j, double beta){    
    if(j<0 || j>2) return;
    fbeta[j] = beta;
  }
  //! Set both time-of-flight and beta
  void SetTOFBeta(unsigned short j, double tof, double beta){    
    if(j<0 || j>2) return;
    ftof[j] = tof;
    fbeta[j] = beta;
  }
  //! Set the delta
  void SetDelta(unsigned short j, double delta){    
    if(j<0 || j>3) return;
    fdelta[j] = delta;
  }

  //! Correct the A/Q ratio based on position
  void CorrectAQ(unsigned short j, double corr){
    if(j<0 || j>5) return;
    faoqc[j] = faoq[j] + corr;
  }

  //! Get the A/Q ratio
  double GetAQ(unsigned short j){
    if(j<0 || j>5) return sqrt(-1.);
    return faoq[j];
  }
  //! Get the corrected A/Q ratio
  double GetCorrAQ(unsigned short j){
    if(j<0 || j>5) return sqrt(-1.);
    return faoqc[j];
  }
  //! Get the Z number
  double GetZ(unsigned short j){
    if(j<0 || j>5) return sqrt(-1.);
    return fzet[j];
  }
  //! Get the time-of-flight
  double GetTOF(unsigned short j){
    if(j<0 || j>2) return sqrt(-1.);
    return ftof[j];
  }
  //! Get beta
  double GetBeta(unsigned short j){
    if(j<0 || j>2) return sqrt(-1.);
    return fbeta[j];
  }
  //! Get Delta
  double GetDelta(unsigned short j){
    if(j<0 || j>2) return sqrt(-1.);
    return fdelta[j];
  }

protected:
  //! A/Q for 3-5, 5-7, 3-7,  8-9, 9-11, 8-11
  double faoq[6];
  //! corrected A/Q
  double faoqc[6];
  //! Z for 3-5, 5-7, 3-7,  8-9, 9-11, 8-11  
  double fzet[6];
  //! corrected Z
  double fzetc[6];

  //! time-of-flight for 3-7, 8-11, 7-8
  double ftof[3];
  //! beta for 3-7, 8-11, 7-8
  double fbeta[3];

  //! delta momentum 3-5, 5-7, 8-9, 9-11
  double fdelta[4];

  /// \cond CLASSIMP
  ClassDef(Beam,1);
  /// \endcond
};


#endif
