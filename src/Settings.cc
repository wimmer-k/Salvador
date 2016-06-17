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
  
  fwithDALI = set->GetValue("WithDALI",true);

  for(int i=0;i<6;i++)
    ftoffset[i] = set->GetValue(Form("TOF.Offset.%d",i),300.0);
  fDALIfile = set->GetValue("DALI.File","/home/wimmer/ribf94/db/DALI.xml");

  fPPACfile = set->GetValue("BigRIPS.PPAC.File","/home/wimmer/ribf94/db/BigRIPSPPAC.xml");
  fPPACdefaultfile = set->GetValue("BigRIPS.PPAC,Def.File","/home/wimmer/ribf94/db/BigRIPSPPAC.xml");
  fplasticfile = set->GetValue("BigRIPS.Plastic.File","/home/wimmer/ribf94/db/BigRIPSPlastic.xml");
  fICfile = set->GetValue("BigRIPS.IC.File","/home/wimmer/ribf94/db/BigRIPSIC.xml");
  ffocalfile = set->GetValue("BigRIPS.Focal.File","/home/wimmer/ribf94/db/FocalPlane.xml");
  fmatrixfile[0] = set->GetValue("Matrix.35.File","/home/wimmer/ribf94/matrix/mat1.mat");
  fmatrixfile[1] = set->GetValue("Matrix.57.File","/home/wimmer/ribf94/matrix/mat2.mat");
  fmatrixfile[2] = set->GetValue("Matrix.89.File","/home/wimmer/ribf94/matrix/F8F9_LargeAccAchr.mat");
  fmatrixfile[3] = set->GetValue("Matrix.911.File","/home/wimmer/ribf94/matrix/F9F11_LargeAccAchr.mat");

  fdorecal = false;
  if(set->GetValue("Do.ReCalibration",0)>0){
    fdorecal = true;
    fDALIrecalfile = set->GetValue("ReCalibration.File",(char*)"settings/recal.dat");
  }

  fbeta = set->GetValue("AverageBeta",0.5);
  foverflow = set->GetValue("Overflow.Threshold",8000);
  funderflow = set->GetValue("Underflow.Threshold",0);
  faddbacktype = set->GetValue("Addback.Type",1);
  faddbackdistance = set->GetValue("Addback.Distance",20.);
  faddbackangle = set->GetValue("Addback.Angle",20.);
  faddbackthreshold = set->GetValue("Addback.Threshold",200.);
  faddbacktdiff[0] = set->GetValue("Addback.TimeDiff.Low",-50.);
  faddbacktdiff[1] = set->GetValue("Addback.TimeDiff.High",20.);

  ftimegate[0] = set->GetValue("Timing.Gate.Low", 20.);
  ftimegate[1] = set->GetValue("Timing.Gate.High",30.);

  fDALIposfile = set->GetValue("InteractionPoints",(char*)"settings/iponts.dat");
  fDALIbadfile = set->GetValue("Bad.Channels",(char*)"settings/baddali.dat");

  ftargetposition = set->GetValue("Target.Position",129.5);
  if(fverbose>0)
    PrintSettings();
}
/*! 
  Print the settings to the screen
*/
void Settings::PrintSettings(){  
  cout << "verbose level\t" << fverbose << endl;
  if(fwithDALI)
    cout << "using DALI" << endl;
  else
    cout << "without DALI" << endl;

  for(int i=0;i<6;i++)
    cout << Form("TOF offset.%d\t",i) << ftoffset[i] << endl;

  cout << "BigRIPS.PPAC.File\t"	<< fPPACfile << endl;
  cout << "BigRIPS.PPAC.Def.File\t"	<< fPPACdefaultfile << endl;
  cout << "BigRIPS.Plastic.File\t" << fplasticfile << endl;
  cout << "BigRIPS.IC.File\t" << fICfile << endl;
  cout << "BigRIPS.Focal.File\t" << ffocalfile	<< endl;
  cout << "Matrix.35.File\t" << fmatrixfile[0] << endl;
  cout << "Matrix.57.File\t" << fmatrixfile[1] << endl;
  cout << "Matrix.89.File\t" << fmatrixfile[2] << endl;
  cout << "Matrix.911.File\t" << fmatrixfile[3] << endl;
  	
  cout << "bad channels file\t" << fDALIbadfile << endl;  
  cout << "DALI calibration file\t" << fDALIfile << endl;
  if(fdorecal){
    cout << "performing re-calibration with second order" << endl;
    cout << "DALI second order calibration file\t" << fDALIrecalfile << endl;
  }
  cout << "position file\t" << fDALIposfile << endl;  
  cout << "timing gate\t" <<ftimegate[0] << " to " << ftimegate[1] << endl;

  cout << "addback type\t" << faddbacktype << endl;
  cout << "addback distance\t" << faddbackdistance << endl;
  cout << "addback angle\t" << faddbackangle << endl;
  cout << "addback time difference\t" <<faddbacktdiff[0] << " to " << faddbacktdiff[1] << endl;

  cout << "beta\t" << fbeta << endl;  
  cout << "target position\t" << ftargetposition << endl;
}

