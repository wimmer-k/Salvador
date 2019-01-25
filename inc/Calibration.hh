#ifndef __CALIBRATION_HH
#define __CALIBRATION_HH
#include <iostream>
#include <iomanip>
#include "TRandom.h"

#include "WASABI.hh"
#include "WASABISettings.hh"
#include "WASABIdefs.h"
/*!
  A class for calibration of WASABI data, includes mapping and correlation of time and energy information
*/
class Calibration {
public:
  //! default constructor
  Calibration(){};
  //! constructor
  Calibration(char* settings);
  //! dummy destructor
  ~Calibration(){
  };
  //! access the settings
  WASABISettings* GetSettings(){return fset;}
  //! Read in the ADC mapping
  void ReadADCMap(char* mapfile);
  //! Read in the ADC thresholds
  void ReadADCThresholds(char* threshfile);
  //! Read in the energy calibration
  void ReadCalibration(char* calfile);
  //! apply mapping and calibration
  WASABI* BuildWASABI(WASABIRaw *raw);
  
private:
  //! radom generator for smearing ADC values
  TRandom* fRand;
  //! settings for calibration
  WASABISettings* fset;
  //! mapping index to DSSSD number
  short fDSSSD[NADCS*NADCCH];
  //! mapping index to Strip number
  short fStrip[NADCS*NADCCH];
  //! ADC thresholds
  short fThresh[NADCS*NADCCH];
  //! Gains Xstrips
  double fgainX[NDSSSD][NXSTRIPS];
  //! Offsets Xstrips
  double foffsetX[NDSSSD][NXSTRIPS];
  //! Gains Ystrips
  double fgainY[NDSSSD][NYSTRIPS];
  //! Offsets Ystrips
  double foffsetY[NDSSSD][NYSTRIPS];


  
};
#endif
