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
  ReadCalibration(fset->CalFile(), fset->TOffsetFile());
  ReadTDCMap(fset->TDCMapFile());
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
  Read in the TDC mapping, from tdc number and channel to dssd and strip
  \param mapfile the file containing the mapping
*/
void Calibration::ReadTDCMap(char* mapfile){
  TEnv *tdcmap = new TEnv(mapfile);
  for(int a = 0; a<NTDCS; a++){
    for(int c =0; c<NTDCCH; c++){
      fTDCDSSSD[a*NTDCCH+c] = tdcmap->GetValue(Form("WASABI.DSSSD.%d",a*NTDCCH+c),-1);
      fTDCStrip[a*NTDCCH+c] = tdcmap->GetValue(Form("WASABI.Strip.%d",a*NTDCCH+c),-1);
    }
  } 
}
/*!
  Read in the ADC thresholds
  \param threshfile the file containing the thresholds
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
  \param adccalfile the file containing the gains and offsets for the ADC
  \param tdccalfile the file containing the offsets for the TDC
*/
void Calibration::ReadCalibration(char* adccalfile, char* tdccalfile){
  TEnv *adccal = new TEnv(adccalfile);
  TEnv *tdccal = new TEnv(tdccalfile);
  for(int d = 0; d<NDSSSD; d++){
    for(int x =0; x<NXSTRIPS; x++){
      fgainX[d][x] = adccal->GetValue(Form("Gain.DSSSD.%d.XStrip.%d",d,x),-1.0);
      foffsetX[d][x] = adccal->GetValue(Form("Offset.DSSSD.%d.XStrip.%d",d,x),0.0);
      fToffsetX[d][x] = tdccal->GetValue(Form("TOffset.DSSSD.%d.XStrip.%d",d,x),0.0);
      if(fset->GetVLevel()>0)
	cout << "d = " << d << ", xstrip gain = " << fgainX[d][x] << ", offset = " << foffsetX[d][x] << ", Toffset = " << fToffsetX[d][x] << endl;
    }
    for(int y =0; y<NYSTRIPS; y++){
      fgainY[d][y] = adccal->GetValue(Form("Gain.DSSSD.%d.YStrip.%d",d,y),-1.0);
      foffsetY[d][y] = adccal->GetValue(Form("Offset.DSSSD.%d.YStrip.%d",d,y),0.0);
      fToffsetY[d][y] = tdccal->GetValue(Form("TOffset.DSSSD.%d.YStrip.%d",d,y),0.0);
      if(fset->GetVLevel()>0)
	cout << "d = " << d << ", ystrip gain = " << fgainY[d][y] << ", offset = " << foffsetY[d][y] << ", Toffset = " << fToffsetY[d][y] << endl;
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
  WASABI* event = new WASABI();

  //adc mapping and calibration
  vector<WASABIRawADC*> rawadcs = raw->GetADCs();
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
      if(fset->VetoX(dsssd) > 0 && en > fset->VetoX(dsssd))
	event->GetDSSSD(dsssd)->SetVetoX();
      if(fset->ThreshX(dsssd) > 0 && en < fset->ThreshX(dsssd))
	continue;
      event->GetDSSSD(dsssd)->AddHitX(new WASABIHit(strip,en,cal));
      //cout << " added X strip " << strip << " with energy " << en << endl;
    }
    else{
      strip -= NXSTRIPS;
      bool cal = false;
      if(fgainY[dsssd][strip]>0){
	cal = true;
	en = en * fgainY[dsssd][strip] + foffsetY[dsssd][strip];
      }
      if(fset->VetoY(dsssd) > 0 && en > fset->VetoY(dsssd))
	event->GetDSSSD(dsssd)->SetVetoY();
      if(fset->ThreshY(dsssd) > 0 && en < fset->ThreshY(dsssd))
	continue;
      event->GetDSSSD(dsssd)->AddHitY(new WASABIHit(strip,en,cal));
      //cout << " added Y strip " << strip << " with energy " << en << endl;
    }
  }

  //time calibration and mapping
  short references[NTDCS];
  for(int t=0;t<NTDCS;t++){
    references[t] = raw->GetTDC(t,fset->TDCrefchannel(t))->GetVal()[0];
    //cout << "reference TDC " << t << " ch " << fset->TDCrefchannel(t) << ",\t" << raw->GetTDC(t,fset->TDCrefchannel(t))->GetVal().size() << "\t"<< references[t] << endl;
  }
  vector<WASABIRawTDC*> rawtdcs = raw->GetTDCs();
  for(vector<WASABIRawTDC*>::iterator tdc=rawtdcs.begin(); tdc!=rawtdcs.end(); tdc++){
    short tdcnr = (*tdc)->GetTDC();
    short index = tdcnr*NTDCCH + (*tdc)->GetChan();
    short dsssd = fTDCDSSSD[index];
    short strip = fTDCStrip[index];
    if(dsssd<0 || dsssd>NDSSSD-1)
      continue;
    if(strip<0 || strip>NXSTRIPS+NYSTRIPS-1)
      continue;

    /*
    cout << "tdc = " << (*tdc)->GetTDC() << ", ch = " << (*tdc)->GetChan() << ", index = " << index << ", dsssd = " << dsssd << ", strip = " << strip << ", vals = " << (*tdc)->GetVal().size();
    for(unsigned short v = 0; v<(*tdc)->GetVal().size(); v++)
      cout << ", val["<<v<<"] = " << (*tdc)->GetVal()[v];
    cout << endl;
    */
    
    if(strip < NXSTRIPS){
      //cout << "xstrips ";
      for(unsigned short v = 0; v<(*tdc)->GetVal().size(); v++){
	double tval  = (*tdc)->GetVal()[v]+fRand->Uniform(0,1) - references[tdcnr];
	tval -= fToffsetX[dsssd][strip];
	event->GetDSSSD(dsssd)->SetStripTimeX(strip,tval);
	//cout << tval << "\t";
      }
      //cout << endl;
    }
    else{
      strip -= NXSTRIPS;
      //cout << "ystrips ";
       for(unsigned short v = 0; v<(*tdc)->GetVal().size(); v++){
	double tval  = (*tdc)->GetVal()[v]+fRand->Uniform(0,1) - references[tdcnr];
	cout << tval << "\t";
	tval -= fToffsetY[dsssd][strip];
 	event->GetDSSSD(dsssd)->SetStripTimeY(strip,tval);
	cout << tval << endl;
      }
       //cout << endl;
    }
    

  }//loop tdcs

  //find strip with fastest timing for implantation
  for(int i=0; i<NDSSSD; i++){
    DSSSD* dsssd = event->GetDSSSD(i);
    vector<WASABIHit*> hitsX = dsssd->GetHitsX();
    vector<WASABIHit*> hitsY = dsssd->GetHitsY();
    int fasteststrip =-1;
    double fastesttime = 10000;
    for(vector<WASABIHit*>::iterator hit=hitsX.begin(); hit!=hitsX.end(); hit++){
      if((*hit)->GetEn()>fset->OverflowX(i)){
	if((*hit)->GetTime0()<fastesttime){
	  fastesttime = (*hit)->GetTime0();
	  fasteststrip = (*hit)->GetStrip();
	}	
      }//overflow
    }// xhits
    if(fasteststrip>-1)
      dsssd->SetImplantX(fasteststrip);

    fasteststrip =-1;
    fastesttime = 10000;
    for(vector<WASABIHit*>::iterator hit=hitsY.begin(); hit!=hitsY.end(); hit++){
      if((*hit)->GetEn()>fset->OverflowY(i)){
	if((*hit)->GetTime0()<fastesttime){
	  fastesttime = (*hit)->GetTime0();
	  fasteststrip = (*hit)->GetStrip();
	}	
      }//overflow
    }// yhits
    if(fasteststrip>-1)
      dsssd->SetImplantY(fasteststrip);
    
  }//loop DSSSDs


  // for(int i=0; i<NDSSSD; i++)
  //   event->GetDSSSD(i)->Print();
  //event->Print();
  
  return event;
}
