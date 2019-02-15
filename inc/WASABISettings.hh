#ifndef __WASABISETTINGS_HH
#define __WASABISETTINGS_HH
#include <iostream>
#include <iomanip>
#include <string>
#include "WASABIdefs.h"

#include "TEnv.h"

using namespace std;

/*!
  A container for the analysis settings
*/
class WASABISettings {
public:
  //! default constructor
  WASABISettings(){};
  //! constructor
  WASABISettings(char* settings);
  //! dummy destructor
  ~WASABISettings(){
  };
  //! Read the settings
  void ReadSettings();
  //! Print the settings
  void PrintSettings();

  //! Get the vorbose level
  int VerboseLevel(){return fverbose;}
  //! Set the verbose level
  void SetVerboseLevel(int vl){fverbose = vl;}
  //! Get the verbose level
  int GetVerboseLevel(){return fverbose;}
  //! Get the verbose level
  int GetVLevel(){return fverbose;}
  //! File containing the mapping ADC channel to detector and strip
  char* ADCMapFile(){return (char*)fADCmapfile.c_str();}
  //! File containing the thresholds for the ADC
  char* ADCThreshFile(){return (char*)fADCthreshfile.c_str();}
  //! File containing energy calibrations
  char* CalFile(){return (char*)fcalfile.c_str();}

  //! File containing the mapping TDC channel to detector and strip
  char* TDCMapFile(){return (char*)fTDCmapfile.c_str();}
  //! Reference channels for the TDC
  short TDCrefchannel(int tdc){
    if(tdc>-1 && tdc<NTDCS)
      return fTDCref[tdc];
    return -1;
  }
  //! File containing time offsets
  char* TOffsetFile(){return (char*)ftoffsetfile.c_str();}

  //! veto X condition on DSSSD
  double VetoX(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return fvetoX[dsssd];
    return -1;
  }
  //! veto Y condition on DSSSD
  double VetoY(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return fvetoY[dsssd];
    return -1;
  }
  //! overflow X condition on DSSSD
  double OverflowX(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return foverflowX[dsssd];
    return -1;
  }
  //! overflow Y condition on DSSSD
  double OverflowY(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return foverflowY[dsssd];
    return -1;
  }
  //! thresh X condition on DSSSD
  double ThreshX(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return fthreshX[dsssd];
    return -1;
  }
  //! thresh Y condition on DSSSD
  double ThreshY(int dsssd){
    if(dsssd>-1 && dsssd<NDSSSD)
      return fthreshY[dsssd];
    return -1;
  }
  
  
private:
  //! filename of the settings file
  string finputfile;
  //! verbose level
  int fverbose;
  //! filename for the ADC mapping
  string fADCmapfile;
  //! filename for the ADC thresholds
  string fADCthreshfile;
  //! filename for the ADC energy calibration
  string fcalfile;
  
  //! filename for the TDC mapping
  string fTDCmapfile;
  //! reference channel of the TDCs
  short fTDCref[NTDCS];
  //! filename for the time offsets
  string ftoffsetfile;
  
  //! veto for X strips
  double fvetoX[NDSSSD];
  //! veto for Y strips
  double fvetoY[NDSSSD];
  
  //! overflow for X strips
  double foverflowX[NDSSSD];
  //! overflow for Y strips
  double foverflowY[NDSSSD];
  
  //! thresh for X strips
  double fthreshX[NDSSSD];
  //! thresh for Y strips
  double fthreshY[NDSSSD];
  

};
#endif
