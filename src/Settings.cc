#include "Settings.hh"
using namespace std;

/*!
  Constructor, setup the settings
  \param settings the settings file
*/
Settings::Settings(char *settings){
  finputfile = settings;
  ReadSettings();
}
/*!
  Read in the settings from the fie
*/
void Settings::ReadSettings(){
  TEnv *set = new TEnv(finputfile.c_str());
  fverbose = set->GetValue("VerboseLevel",0);

  for(int i=0;i<6;i++)
    ftoffset[i] = set->GetValue(Form("TOF.Offset.%d",i),300.0);
  fDALIfile = set->GetValue("DALI.File","/home/wimmer/ribf94/db/DALI.xml");

  fbeta = set->GetValue("AverageBeta",0.5);
  foverflow = set->GetValue("Overflow.Threshold",8000);
  funderflow = set->GetValue("Underflow.Threshold",0);
  faddbacktype = set->GetValue("Addback.Type",1);
  faddbackdistance = set->GetValue("Addback.Distance",20.);
  faddbackangle = set->GetValue("Addback.Angle",20.);
  faddbackthreshold = set->GetValue("Addback.Threshold",200.);
  faddbacktdiff[0] = set->GetValue("Addback.TimeDiff.Low",-50.);
  faddbacktdiff[1] = set->GetValue("Addback.TimeDiff.High",20.);

  fDALIposfile = set->GetValue("InteractionPoints",(char*)"settings/iponts.dat");


  if(fverbose>0)
    PrintSettings();
}
/*! 
  Print the settings to the screen
*/
void Settings::PrintSettings(){  
  for(int i=0;i<6;i++)
    cout << Form("TOF.Offset.%d\t",i) << ftoffset[i] << endl;
  cout << "DALI.File\t" << fDALIfile << endl;

  cout << "fverbose\t" << fverbose << endl;
  cout << "fbeta\t" << fbeta << endl;
  cout << "faddbacktype\t" << faddbacktype << endl;
  cout << "faddbackdistance\t" << faddbackdistance << endl;
  cout << "faddbackangle\t" << faddbackangle << endl;
  cout << "fposition file\t" << fDALIposfile << endl;  
}
