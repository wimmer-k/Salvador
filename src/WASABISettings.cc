#include "WASABISettings.hh"
using namespace std;

/*!
  Constructor, setup the settings
  \param settings the settings file
*/
WASABISettings::WASABISettings(char *settings){
  finputfile = settings;
  ReadSettings();
}
/*!
  Read in the settings from the fie
*/
void WASABISettings::ReadSettings(){
  TEnv *set = new TEnv(finputfile.c_str());
  fverbose = set->GetValue("VerboseLevel",0);

  fADCmapfile = set->GetValue("ADCmap.File","/home/wimmer/ribf94/settings/WASABI_ADCmap.dat");
  fTDCmapfile = set->GetValue("TDCmap.File","/home/wimmer/ribf94/settings/WASABI_TDCmap.dat");
  fADCthreshfile = set->GetValue("ADCthresh.File","/home/wimmer/ribf94/settings/WASABI_ADCthresh.dat");

  fcalfile = set->GetValue("Cal.File","/home/wimmer/ribf94/settings/WASABI_cal.dat");
  ftoffsetfile = set->GetValue("TOffset.File","/home/wimmer/ribf94/settings/WASABI_time.dat");
  
  for(int i=0;i<NTDCS;i++)
    fTDCref[i] = set->GetValue(Form("TDC.%d.RefChan",i),0);

  for(int i=0;i<NDSSSD;i++){
    fvetoX[i] = set->GetValue(Form("VetoDSSSD.%d.X",i), -1.0);
    fvetoY[i] = set->GetValue(Form("VetoDSSSD.%d.Y",i), -1.0);
    fthreshX[i] = set->GetValue(Form("ThreshDSSSD.%d.X",i), -1.0);
    fthreshY[i] = set->GetValue(Form("ThreshDSSSD.%d.Y",i), -1.0);
  }
  if(fverbose>0)
    PrintSettings();
}
/*! 
  Print the settings to the screen
*/
void WASABISettings::PrintSettings(){  
  cout << "verbose level\t" << fverbose << endl;
  cout << "ADCmap.File\t" << fADCmapfile << endl;
  cout << "TDCmap.File\t" << fTDCmapfile << endl;
  cout << "ADCthresh.File\t" << fADCthreshfile << endl;
  cout << "Cal.File\t" << fcalfile << endl;
  cout << "TOffset.File\t" << ftoffsetfile << endl;
  for(int i=0;i<NTDCS;i++)
    cout << Form("TDC.%d.RefChan\t",i) << fTDCref[i] << endl;
  for(int i=0;i<NDSSSD;i++){
    cout << Form("VetoDSSSD.%d.X\t",i) << fvetoX[i] << endl;
    cout << Form("VetoDSSSD.%d.Y\t",i) << fvetoY[i] << endl;
    cout << Form("ThreshDSSSD.%d.X\t",i) << fthreshX[i] << endl;
    cout << Form("ThreshDSSSD.%d.Y\t",i) << fthreshY[i] << endl;
  }

}

