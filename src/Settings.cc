#include "Settings.hh"
using namespace std;

/*!
  Constructor, setup the settings
  \param settings the settings file
*/
Settings::Settings(char *settings){
  finputfile = settings;
}
/*!
  Read in the settings from the fie
*/
void Settings::ReadSettings(){
  TEnv *set = new TEnv(finputfile.c_str());
  fverbose = set->GetValue("VerboseLevel",0);

  for(int i=0;i<6;i++)
    ftoffset[i] = set->GetValue(Form("TOF.Offset.%d",i),300.0);
  fDALIFile = set->GetValue("DALI.File","/home/wimmer/ribf94/db/DALI.xml");



  fbeta = set->GetValue("AverageBeta",0.5);
  foverflow = set->GetValue("Overflow.Threshold",8000);
  funderflow = set->GetValue("Underflow.Threshold",0);
  faddbacktype = set->GetValue("Addback.Type",1);
  faddbackdistance = set->GetValue("Addback.Distance",20.);
  faddbackangle = set->GetValue("Addback.Angle",20.);
  faddbackthreshold = set->GetValue("Addback.Threshold",200.);
  faddbacktdiff[0] = set->GetValue("Addback.TimeDiff.Low",-50.);
  faddbacktdiff[1] = set->GetValue("Addback.TimeDiff.High",20.);

  fposfile = set->GetValue("InteractionPoints",(char*)"settings/iponts.dat");


  if(fverbose>0)
    PrintSettings();
}
/*! 
  Print the settings to the screen
*/
void Settings::PrintSettings(){  
  cout << "fverbose " << fverbose << endl;
  cout << "fbeta " << fbeta << endl;
  cout << "faddbacktype " << faddbacktype << endl;
  cout << "faddbackdistance " << faddbackdistance << endl;
  cout << "faddbackangle " << faddbackangle << endl;
  cout << "fposition file " << fposfile << endl;  
}
