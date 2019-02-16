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
  //! Read in the TDC mapping
  void ReadTDCMap(char* mapfile);
  //! Read in the ADC thresholds
  void ReadADCThresholds(char* threshfile);
  //! Read in the energy calibration
  void ReadCalibration(char* adccalfile ,char* tdccalfile);
  //! apply mapping and calibration
  WASABI* BuildWASABI(WASABIRaw *raw);
  //! sort the hits by energy, high to low
  vector<WASABIHit*> Sort(vector<WASABIHit*> hits);
  //! sort the hits by energy, low to high
  vector<WASABIHit*> Revert(vector<WASABIHit*> hits);
   
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
  //! gains X strips
  double fgainX[NDSSSD][NXSTRIPS];
  //! offsets X strips
  double foffsetX[NDSSSD][NXSTRIPS];
  //! gains Y strips
  double fgainY[NDSSSD][NYSTRIPS];
  //! offsets Y strips
  double foffsetY[NDSSSD][NYSTRIPS];
  
  //! mapping index to DSSSD number for the TDCs
  short fTDCDSSSD[NTDCS*NTDCCH];
  //! mapping index to Strip number for the TDCs
  short fTDCStrip[NTDCS*NTDCCH];

  //! time offsets X strips
  double fToffsetX[NDSSSD][NXSTRIPS];
  //! time offsets Y strips
  double fToffsetY[NDSSSD][NYSTRIPS];

};
#endif
