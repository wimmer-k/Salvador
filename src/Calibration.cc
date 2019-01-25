#include "Calibration.hh"

using namespace std;

/*!
  Constructor, reads in the settings for the calibration
  \param settings the settings file
*/
Calibration::Calibration(char* settings){
  fRand = new TRandom();
  fset = new WASABISettings(settings);
  ReadADCMap(fset->ADCMapFile());
  ReadADCThresholds(fset->ADCThreshFile());
  ReadCalibration(fset->CalFile());
}
/*!
  Read in the ADC mapping, from adc number and channel to dssd and strip
  \param mapfile the file containing the mapping
*/
void Calibration::ReadADCMap(char* mapfile){
  TEnv *adcmap = new TEnv(mapfile);
  for(int a = 0; a<NADCS; a++){
    for(int c =0; c<NADCCH; c++){
      fDSSSD[a*NADCCH+c] = adcmap->GetValue(Form("WASABI.DSSSD.%d",a*NADCCH+c),-1);
      fStrip[a*NADCCH+c] = adcmap->GetValue(Form("WASABI.Strip.%d",a*NADCCH+c),-1);
      fDSSSD[a*NADCCH+c] -= 1; //counting starts from 1 in the file
      fStrip[a*NADCCH+c] -= 1;
    }
  } 
}
/*!
  Read in the ADC thresholds
  \param the file containing the thresholds
*/
void Calibration::ReadADCThresholds(char* threshfile){
  TEnv *adcthresh = new TEnv(threshfile);
  for(int a = 0; a<NADCS; a++){
    for(int c =0; c<NADCCH; c++){
      fThresh[a*NADCCH+c] = adcthresh->GetValue(Form("WASABI.Thresh.%d",a*NADCCH+c),0);
    }
  } 
}
/*!
  Read in the energy calibration
  \param the file containing the gains and offsets
*/
void Calibration::ReadCalibration(char* calfile){
  TEnv *adccal = new TEnv(calfile);
  for(int d = 0; d<NDSSSD; d++){
    for(int x =0; x<NXSTRIPS; x++){
      fgainX[d][x] = adccal->GetValue(Form("Gain.DSSSD.%d.XStrip.%d",d,x),-1.0);
      foffsetX[d][x] = adccal->GetValue(Form("Offset.DSSSD.%d.XStrip.%d",d,x),0.0);
      if(fset->GetVLevel()>0)
	cout << "d = " << d << ", xstrip gain = " << fgainX[d][x] << ", offset = " << foffsetX[d][x]<< endl;
    }
    for(int y =0; y<NYSTRIPS; y++){
      fgainY[d][y] = adccal->GetValue(Form("Gain.DSSSD.%d.YStrip.%d",d,y),-1.0);
      foffsetY[d][y] = adccal->GetValue(Form("Offset.DSSSD.%d.YStrip.%d",d,y),0.0);
      if(fset->GetVLevel()>0)
	cout << "d = " << d << ", ystrip gain = " << fgainY[d][y] << ", offset = " << foffsetY[d][y]<< endl;
    }
  } 

}
/*!
  Apply mapping and calibrations, 
  \param raw wasabi data
  \returns calibrated wasabi event
*/
WASABI* Calibration::BuildWASABI(WASABIRaw *raw){
  //cout << __PRETTY_FUNCTION__ << endl;
  vector<WASABIRawADC*> rawadcs = raw->GetADCs();
  WASABI* event = new WASABI();
  for(vector<WASABIRawADC*>::iterator adc=rawadcs.begin(); adc!=rawadcs.end(); adc++){
    short index = (*adc)->GetADC()*NADCCH + (*adc)->GetChan();
    short dsssd = fDSSSD[index];
    short strip = fStrip[index];
    if(dsssd<0 || dsssd>NDSSSD-1)
      continue;
    if(strip<0 || strip>NXSTRIPS+NYSTRIPS-1)
      continue;


    //cout << "adc = " << (*adc)->GetADC() << ", ch = " << (*adc)->GetChan() << ", index = " << index << ", dsssd = " << dsssd << ", strip = " << strip << ", val = " << (*adc)->GetVal() << endl;
    short adcval = (*adc)->GetVal() - fThresh[index];
    if(adcval<0)
      continue;
    
    double en = adcval+fRand->Uniform(0,1);
    if(strip < NXSTRIPS){
      bool cal = false;
      if(fgainX[dsssd][strip]>0){
	cal = true;
	en = en * fgainX[dsssd][strip] + foffsetX[dsssd][strip];
      }
      event->GetDSSSD(dsssd)->AddHitX(new WASABIHit(strip,en,0,cal));
      //cout << " added X strip " << strip << " with energy " << en << endl;
    }
    else{
      strip -= NXSTRIPS;
      bool cal = false;
      if(fgainY[dsssd][strip]>0){
	cal = true;
	en = en * fgainY[dsssd][strip] + foffsetY[dsssd][strip];
      }
      event->GetDSSSD(dsssd)->AddHitY(new WASABIHit(strip,en,0,cal));
      //cout << " added Y strip " << strip << " with energy " << en << endl;
    }
  }
  // for(int i=0; i<NDSSSD; i++)
  //   event->GetDSSSD(i)->Print();
  // event->Print();
  
  return event;
}
