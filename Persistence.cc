#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCutG.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "FocalPlane.hh"
#include "DALI.hh"
#include "Beam.hh"
#include "PPAC.hh"
#include "Reconstruction.hh"
#include "Globaldefs.h"

int DALIIDS = 300;

using namespace TMath;
using namespace std;
bool signal_received = false;
void signalhandler(int sig);
double get_time();
double deg2rad = TMath::Pi()/180.;
double rad2deg = 180./TMath::Pi();
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  cout << "\"La persistencia de la memoria\" (1931), Salvador Dali" << endl;
  cout << "Analyzer for DALI (Add-back and Doppler-correction also for simulated data)" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char *SetFile = NULL;
  char* TreeName = (char*)"tr";
  int writeTree = 1;
  double beta =0;
  int minID = 79;
  int br = 2;
  int zd = 5;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-s", "settings file", &SetFile);    
  interface->Add("-tn", "name of the tree", &TreeName);
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->Add("-wt", "write tree", &writeTree);  
  interface->Add("-b", "beta (overrides settings file)", &beta);
  interface->Add("-id", "minimum DALI ID for additional gamma-gamma spectra", &minID);
  interface->Add("-br", "cut on this BigRIPS PIC", &br);
  interface->Add("-zd", "cut on this ZeroDeg PIC", &zd);

  interface->CheckFlags(argc, argv);
  //Complain about missing mandatory arguments
  if(InputFiles.size() == 0){
    cout << "No input file given " << endl;
    return 1;
  }
  if(OutFile == NULL){
    cout << "No output ROOT file given " << endl;
    OutFile = (char*)"test.root";
  }
  if(SetFile == NULL){
    cout << "No settings file given " << endl;
    return 2;
  }
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  TChain* tr;
  tr = new TChain(TreeName);
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
  }
  if(tr == NULL){
    cout << "could not find tree "<< TreeName <<" in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  int trigbit = 0;
  tr->SetBranchAddress("trigbit",&trigbit);
  PPAC* ppac = new PPAC;
  tr->SetBranchAddress("ppacs",&ppac);
  Beam* beam = new Beam;
  tr->SetBranchAddress("beam",&beam);
  FocalPlane *fp[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->SetBranchAddress(Form("fp%d",fpID[f]),&fp[f]);
  }
  DALI* dali = new DALI;
  tr->SetBranchAddress("dali",&dali);

  TTree* rtr = new TTree("rtr","Reconstructed events");
  rtr->Branch("trigbit",&trigbit,"trigbit/I");
  rtr->Branch("dali",&dali,320000);
  rtr->Branch("beam",&beam,320000);

  cout<<"settings file: " << SetFile <<endl;
  Reconstruction *rec = new Reconstruction(SetFile);
  if(Verbose>0){
    rec->GetSettings()->SetVerboseLevel(Verbose);
    rec->GetSettings()->PrintSettings();
  }
  if(beta>0)
    rec->SetBeta(beta);
  bool recalibrate = rec->DoReCalibration();

  TList *hlist = new TList();

  //histograms
  TH1F* trigger = new TH1F("trigger","trigger",10,0,10);hlist->Add(trigger);
  TH2F* bigrips = new TH2F("bigrips","bigrips",1000,1.8,2.3,1000,20,40);hlist->Add(bigrips);
  TH2F* zerodeg = new TH2F("zerodeg","zerodeg",1000,1.8,2.3,1000,20,40);hlist->Add(zerodeg);
  TH2F* bigrips_tr[10];
  TH2F* zerodeg_tr[10];
  TH1F* f5X_tr[10];
  for(int i=0;i<10;i++){
    bigrips_tr[i] = new TH2F(Form("bigrips_tr%d",i),Form("bigrips_tr%d",i),1000,1.8,2.3,1000,20,40);hlist->Add(bigrips_tr[i]);
    zerodeg_tr[i] = new TH2F(Form("zerodeg_tr%d",i),Form("zerodeg_tr%d",i),1000,1.8,2.3,1000,20,40);hlist->Add(zerodeg_tr[i]);
    f5X_tr[i] = new TH1F(Form("f5X_tr%d",i),Form("f5X_tr%d",i),3000,-150,150);hlist->Add(f5X_tr[i]);
  }

  TH1F* f8ppacX[6];
  TH1F* f8ppacY[6];
  TH2F* f8ppacXY[6];
  for(int p=0;p<6;p++){
    f8ppacX[p] = new TH1F(Form("f8ppacX_%d",p),Form("f8ppacX_%d",p),200,-100,100);hlist->Add(f8ppacX[p]);
    f8ppacY[p] = new TH1F(Form("f8ppacY_%d",p),Form("f8ppacY_%d",p),200,-100,100);hlist->Add(f8ppacY[p]);
    f8ppacXY[p] = new TH2F(Form("f8ppacXY_%d",p),Form("f8ppacXY_%d",p),200,-100,100,200,-100,100);hlist->Add(f8ppacXY[p]);
  }
  TH2F* compareX[2];
  TH2F* compareY[2];
  TH1F* compare1dX[2];
  TH1F* compare1dY[2];
  for(int p=0;p<2;p++){
    compareX[p] = new TH2F(Form("compareX_%d",p),Form("compareX_%d",p),200,-100,100,200,-100,100);hlist->Add(compareX[p]);
    compareY[p] = new TH2F(Form("compareY_%d",p),Form("compareY_%d",p),200,-100,100,200,-100,100);hlist->Add(compareY[p]);
    compare1dX[p] = new TH1F(Form("compare1dX_%d",p),Form("compare1dX_%d",p),1000,-100,100);hlist->Add(compare1dX[p]);
    compare1dY[p] = new TH1F(Form("compare1dY_%d",p),Form("compare1dY_%d",p),1000,-100,100);hlist->Add(compare1dY[p]);
  }
  TH2F* compareA = new TH2F("compareA","compareA",200,-100,100,200,-100,100);hlist->Add(compareA);
  TH2F* compareB = new TH2F("compareB","compareB",200,-100,100,200,-100,100);hlist->Add(compareB);
  TH2F* ppacZpos = new TH2F("ppacZpos","ppacZpos",40,0,40,3000,-1500,1500);hlist->Add(ppacZpos);
  TH2F* incAB = new TH2F("incAB","incAB",300,-30,30,300,-30,30);hlist->Add(incAB);
  TH2F* scaAB = new TH2F("scaAB","scaAB",300,-30,30,300,-30,30);hlist->Add(scaAB);
  TH2F* targetXY = new TH2F("targetXY","targetXY",200,-100,100,200,-100,100);hlist->Add(targetXY);
  TH2F* thetaphi = new TH2F("thetaphi","thetaphi",800,-4,4,1500,0,150);hlist->Add(thetaphi);
  TH2F* thetaphideg = new TH2F("thetaphideg","thetaphideg",800,-180,180,1500,0,5);hlist->Add(thetaphideg);
  TH2F* thetaphiin = new TH2F("thetaphiin","thetaphiin",800,-4,4,1500,0,150);hlist->Add(thetaphiin);
  TH2F* thetaphiout = new TH2F("thetaphiout","thetaphiout",800,-4,4,1500,0,150);hlist->Add(thetaphiout);
  TH2F* thetaphisca = new TH2F("thetaphisca","thetaphisca",800,-4,4,1500,0,150);hlist->Add(thetaphisca);
  TH2F* phiinout = new TH2F("phiinout","phiinout",800,-4,4,800,-4,4);hlist->Add(phiinout);
  TH2F* thetaphi_tr[10];
  TH2F* thetaphideg_tr[10];
  for(int i=0;i<10;i++){
    thetaphi_tr[i] = new TH2F(Form("thetaphi_tr%d",i),Form("thetaphi_tr%d",i),800,-4,4,1500,0,150);hlist->Add(thetaphi_tr[i]);
    thetaphideg_tr[i] = new TH2F(Form("thetaphideg_tr%d",i),Form("thetaphideg_tr%d",i),800,-180,180,1500,0,5);hlist->Add(thetaphideg_tr[i]);
  }
  TH1F* bbeta[3];
  for(unsigned short b=0;b<3;b++){
    bbeta[b] = new TH1F(Form("bbeta_%d",b),Form("bbeta_%d",b),10000,0,1);hlist->Add(bbeta[b]);
  }
  TH1F* delta[4];
  for(unsigned short b=0;b<4;b++){
    delta[b] = new TH1F(Form("delta_%d",b),Form("delta_%d",b),1000,-10,10);hlist->Add(delta[b]);
  }
  TH1F* deltadiff[2];
  for(unsigned short b=0;b<2;b++){
    deltadiff[b] = new TH1F(Form("deltadiff_%d",b),Form("deltadiff_%d",b),1000,-10,10);hlist->Add(deltadiff[b]);
  }
  //background inspection
  TH2F* PL3PPAC3   = new TH2F("PL3PPAC3","PL3PPAC3",    1000,-5,5,1000,-150,150);hlist->Add(PL3PPAC3);
  TH2F* PL7PPAC7   = new TH2F("PL7PPAC7","PL7PPAC7",    1000,-5,5,1000,-150,150);hlist->Add(PL7PPAC7);
  TH2F* PL9PPAC9   = new TH2F("PL9PPAC9","PL9PPAC9",    1000,-5,5,1000,-150,150);hlist->Add(PL9PPAC9);
  TH2F* PL11PPAC11 = new TH2F("PL11PPAC11","PL11PPAC11",1000,-5,5,1000,-150,150);hlist->Add(PL11PPAC11);
  TH2F* PL3PL3     = new TH2F("PL3PL3","PL3PL3",        1000,-5,5,1000,-5,5);hlist->Add(PL3PL3);
  TH2F* PL7PL7     = new TH2F("PL7PL7","PL7PL7",        1000,-5,5,1000,-5,5);hlist->Add(PL7PL7);
  TH2F* PL9PL9     = new TH2F("PL9PL9","PL9PL9",        1000,-5,5,1000,-5,5);hlist->Add(PL9PL9);
  TH2F* PL11PL11   = new TH2F("PL11PL11","PL11PL11",    1000,-5,5,1000,-5,5);hlist->Add(PL11PL11);
  
  
  TH1F* tdiff = new TH1F("tdiff","tdiff",2000,-1000,1000);hlist->Add(tdiff);
  TH1F* rdiff = new TH1F("rdiff","rdiff",2000,0,10);hlist->Add(rdiff);
  TH1F* adiff = new TH1F("adiff","adiff",2000,0,4);hlist->Add(adiff);
  TH2F* radiff = new TH2F("radiff","radiff",200,0,4,200,0,10);hlist->Add(radiff);

  // TH1F* tdiff_coinc = new TH1F("tdiff_coinc","tdiff_coinc",2000,-1000,1000);hlist->Add(tdiff_coinc);
  // TH1F* rdiff_coinc = new TH1F("rdiff_coinc","rdiff_coinc",2000,0,10);hlist->Add(rdiff_coinc);
  // TH1F* adiff_coinc = new TH1F("adiff_coinc","adiff_coinc",2000,0,4);hlist->Add(adiff_coinc);
  // TH2F* radiff_coinc = new TH2F("radiff_coinc","radiff_coinc",200,0,4,200,0,10);hlist->Add(radiff_coinc);

  int bins = 8000;
  
  TH2F* triggertgam = new TH2F("triggertgam","triggertgam",1000,-500,500,10,0,10);hlist->Add(triggertgam);
  TH2F* ID_theta  = new TH2F("ID_theta","ID_theta",250,0,250,180,0,180);hlist->Add(ID_theta);
  TH1F* mult = new TH1F("mult","mult",50,0,50);hlist->Add(mult);
  TH2F* multtrig = new TH2F("multtrig","multtrig",20,0,20,50,0,50);hlist->Add(multtrig);
  TH1F* egam = new TH1F("egam","egam",bins,0,bins);hlist->Add(egam);
  TH1F* egamdc = new TH1F("egamdc","egamdc",bins,0,bins);hlist->Add(egamdc);
  TH1F* egam_IDgate = new TH1F("egam_IDgate","egam_IDgate",bins,0,bins);hlist->Add(egam_IDgate);
  TH1F* egamdc_IDgate = new TH1F("egamdc_IDgate","egamdc_IDgate",bins,0,bins);hlist->Add(egamdc_IDgate);
  TH2F* egamtgam = new TH2F("egamtgam","egamtgam",1000,-500,500,1000,0,bins);hlist->Add(egamtgam);
  TH2F* egamdctgam = new TH2F("egamdctgam","egamdctgam",1000,-500,500,1000,0,bins);hlist->Add(egamdctgam);
  TH2F* egammult = new TH2F("egammult","egammult",20,0,20,bins,0,bins);hlist->Add(egammult);
  TH2F* egamdcmult = new TH2F("egamdcmult","egamdcmult",20,0,20,bins,0,bins);hlist->Add(egamdcmult);
  TH2F* egammult_IDgate = new TH2F("egammult_IDgate","egammult_IDgate",20,0,20,bins,0,bins);hlist->Add(egammult_IDgate);
  TH2F* egamdcmult_IDgate = new TH2F("egamdcmult_IDgate","egamdcmult_IDgate",20,0,20,bins,0,bins);hlist->Add(egamdcmult_IDgate);
  TH2F* egamtrig = new TH2F("egamtrig","egamtrig",10,0,10,bins,0,bins);hlist->Add(egamtrig);
  TH2F* egamID_mult[10];
  TH2F* egamdcID_mult[10];
  TH2F* egamdctrig_mult[10];
  TH2F* egamdctrigmult_theta[10][10];
  TH2F* egamdctheta_mult[10];
  TH2F* egamdcphi_mult[10];
  TH2F* egamegam_mult[10];
  TH2F* egamegamdc_mult[10];
  TH2F* egamegam_IDgate_mult[10];
  TH2F* egamegamdc_IDgate_mult[10];
  TH1F* multAB = new TH1F("multAB","multAB",50,0,50);hlist->Add(multAB);
  TH1F* egamAB = new TH1F("egamAB","egamAB",bins,0,bins);hlist->Add(egamAB);
  TH1F* egamABdc = new TH1F("egamABdc","egamABdc",bins,0,bins);hlist->Add(egamABdc);
  TH2F* egamABmult = new TH2F("egamABmult","egamABmult",20,0,20,bins,0,bins);hlist->Add(egamABmult);
  TH2F* egamABmultAB = new TH2F("egamABmultAB","egamABmultAB",20,0,20,bins,0,bins);hlist->Add(egamABmultAB);
  TH2F* egamABdcmult = new TH2F("egamABdcmult","egamABdcmult",20,0,20,bins,0,bins);hlist->Add(egamABdcmult);
  TH2F* egamABdcmultAB = new TH2F("egamABdcmultAB","egamABdcmultAB",20,0,20,bins,0,bins);hlist->Add(egamABdcmultAB);
  TH1F* egamAB_IDgate = new TH1F("egamAB_IDgate","egamAB_IDgate",bins,0,bins);hlist->Add(egamAB_IDgate);
  TH1F* egamABdc_IDgate = new TH1F("egamABdc_IDgate","egamABdc_IDgate",bins,0,bins);hlist->Add(egamABdc_IDgate);
  TH2F* egamABmult_IDgate = new TH2F("egamABmult_IDgate","egamABmult_IDgate",20,0,20,bins,0,bins);hlist->Add(egamABmult_IDgate);
  TH2F* egamABmultAB_IDgate = new TH2F("egamABmultAB_IDgate","egamABmultAB_IDgate",20,0,20,bins,0,bins);hlist->Add(egamABmultAB_IDgate);
  TH2F* egamABdcmult_IDgate = new TH2F("egamABdcmult_IDgate","egamABdcmult_IDgate",20,0,20,bins,0,bins);hlist->Add(egamABdcmult_IDgate);
  TH2F* egamABdcmultAB_IDgate = new TH2F("egamABdcmultAB_IDgate","egamABdcmultAB_IDgate",20,0,20,bins,0,bins);hlist->Add(egamABdcmultAB_IDgate);
  TH2F* egamABtrig = new TH2F("egamABtrig","egamABtrig",10,0,10,bins,0,bins);hlist->Add(egamABtrig);
  TH2F* egamABID_mult[10];
  TH2F* egamABID_multAB[10];
  TH2F* egamABdcID_mult[10];
  TH2F* egamABdcID_multAB[10];
  TH2F* egamABdctrig_mult[10];
  TH2F* egamABdctrig_multAB[10];
  TH2F* egamABdctrigmult_theta[10][10];
  TH2F* egamABdctrigmultAB_theta[10][10];
 
  TH2F* egamegamAB_mult[10];
  TH2F* egamegamABdc_mult[10];
  TH2F* egamegamAB_multAB[10];
  TH2F* egamegamABdc_multAB[10];
  TH2F* egamegamAB_IDgate_mult[10];
  TH2F* egamegamABdc_IDgate_mult[10];
  TH2F* egamegamAB_IDgate_multAB[10];
  TH2F* egamegamABdc_IDgate_multAB[10];

  egamID_mult[0] = new TH2F("egamID","egamID",DALIIDS,0,DALIIDS,bins,0,bins);hlist->Add(egamID_mult[0]);
  egamdcID_mult[0] = new TH2F("egamdcID","egamdcID",DALIIDS,0,DALIIDS,bins,0,bins);hlist->Add(egamdcID_mult[0]);
  egamdctrig_mult[0] = new TH2F("egamdctrig","egamdctrig",10,0,10,bins,0,bins);hlist->Add(egamdctrig_mult[0]);
  egamdctheta_mult[0] = new TH2F("egamdctheta","egamdctheta",200,0,4,400,0,bins);hlist->Add(egamdctheta_mult[0]);
  egamdcphi_mult[0] = new TH2F("egamdcphi","egamdcphi",200,-4,4,400,0,bins);hlist->Add(egamdcphi_mult[0]);
  egamABID_mult[0] = new TH2F("egamABID","egamABID",DALIIDS,0,DALIIDS,bins,0,bins);hlist->Add(egamABID_mult[0]);
  egamABdcID_mult[0] = new TH2F("egamABdcID","egamABdcID",DALIIDS,0,DALIIDS,bins,0,bins);hlist->Add(egamABdcID_mult[0]);
  egamABdctrig_mult[0] = new TH2F("egamABdctrig","egamABdctrig",10,0,10,bins,0,bins);hlist->Add(egamABdctrig_mult[0]);
  for(int t=0;t<10;t++){
    egamdctrigmult_theta[t][0] = new TH2F(Form("egamdctrig%d_theta",t),Form("egamdctrig%d_theta",t),100,0,5,bins,0,bins);hlist->Add(egamdctrigmult_theta[t][0]);
    egamABdctrigmult_theta[t][0] = new TH2F(Form("egamABdctrig%d_theta",t),Form("egamABdctrig%d_theta",t),100,0,5,bins,0,bins);hlist->Add(egamABdctrigmult_theta[t][0]);
  }
  
  egamegam_mult[0] = new TH2F("egamegam","egamegam",200,0,4000,200,0,4000);hlist->Add(egamegam_mult[0]);
  egamegamdc_mult[0] = new TH2F("egamegamdc","egamegamdc",200,0,4000,200,0,4000);hlist->Add(egamegamdc_mult[0]);
  egamegamAB_mult[0] = new TH2F("egamegamAB","egamegamAB",200,0,4000,200,0,4000);hlist->Add(egamegamAB_mult[0]);
  egamegamABdc_mult[0] = new TH2F("egamegamABdc","egamegamABdc",200,0,4000,200,0,4000);hlist->Add(egamegamABdc_mult[0]);
  egamegam_IDgate_mult[0] = new TH2F("egamegam_IDgate","egamegam_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegam_IDgate_mult[0]);
  egamegamdc_IDgate_mult[0] = new TH2F("egamegamdc_IDgate","egamegamdc_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamdc_IDgate_mult[0]);
  egamegamAB_IDgate_mult[0] = new TH2F("egamegamAB_IDgate","egamegamAB_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamAB_IDgate_mult[0]);
  egamegamABdc_IDgate_mult[0] = new TH2F("egamegamABdc_IDgate","egamegamABdc_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamABdc_IDgate_mult[0]);
  for(int m=1;m<10;m++){
    egamID_mult[m] = new TH2F(Form("egamIDmult%d",m),Form("egamIDmult%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamID_mult[m]);
    egamdcID_mult[m] = new TH2F(Form("egamdcIDmult%d",m),Form("egamdcIDmult%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamdcID_mult[m]);
    egamdctrig_mult[m] = new TH2F(Form("egamdctrigmult%d",m),Form("egamdctrigmult%d",m),10,0,10,400,0,4000);hlist->Add(egamdctrig_mult[m]);
    egamdctheta_mult[m] = new TH2F(Form("egamdcthetamult%d",m),Form("egamdcthetamult%d",m),200,0,4,400,0,4000);hlist->Add(egamdctheta_mult[m]);
    egamdcphi_mult[m] = new TH2F(Form("egamdcphimult%d",m),Form("egamdcphimult%d",m),200,-4,4,400,0,4000);hlist->Add(egamdcphi_mult[m]);
    egamABID_mult[m] = new TH2F(Form("egamABIDmult%d",m),Form("egamABIDmult%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamABID_mult[m]);
    egamABID_multAB[m] = new TH2F(Form("egamABIDmultAB%d",m),Form("egamABIDmultAB%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamABID_multAB[m]);
    egamABdcID_mult[m] = new TH2F(Form("egamABdcIDmult%d",m),Form("egamABdcIDmult%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamABdcID_mult[m]);
    egamABdcID_multAB[m] = new TH2F(Form("egamABdcIDmultAB%d",m),Form("egamABdcIDmultAB%d",m),DALIIDS,0,DALIIDS,400,0,4000);hlist->Add(egamABdcID_multAB[m]);
    egamABdctrig_mult[m] = new TH2F(Form("egamABdctrigmult%d",m),Form("egamABdctrigmult%d",m),10,0,10,400,0,4000);hlist->Add(egamABdctrig_mult[m]);
    egamABdctrig_multAB[m] = new TH2F(Form("egamABdctrigmultAB%d",m),Form("egamABdctrigmultAB%d",m),10,0,10,400,0,4000);hlist->Add(egamABdctrig_multAB[m]);
    for(int t=0;t<10;t++){
      egamdctrigmult_theta[t][m] = new TH2F(Form("egamdctrig%dmult%d_theta",t,m),Form("egamdctrig%dmult%d_theta",t,m),100,0,5,4000,0,4000);hlist->Add(egamdctrigmult_theta[t][m]);
      egamABdctrigmult_theta[t][m] = new TH2F(Form("egamABdctrig%dmult%d_theta",t,m),Form("egamABdctrig%dmult%d_theta",t,m),100,0,5,4000,0,4000);hlist->Add(egamABdctrigmult_theta[t][m]);
      egamABdctrigmultAB_theta[t][m] = new TH2F(Form("egamABdctrig%dmultAB%d_theta",t,m),Form("egamABdctrig%dmultAB%d_theta",t,m),100,0,5,4000,0,4000);hlist->Add(egamABdctrigmultAB_theta[t][m]);
    }

    
    egamegam_mult[m] = new TH2F(Form("egamegammult%d",m),Form("egamegammult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegam_mult[m]);
    egamegamdc_mult[m] = new TH2F(Form("egamegamdcmult%d",m),Form("egamegamdcmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamdc_mult[m]);
    egamegamAB_mult[m] = new TH2F(Form("egamegamABmult%d",m),Form("egamegamABmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_mult[m]);
    egamegamABdc_mult[m] = new TH2F(Form("egamegamABdcmult%d",m),Form("egamegamABdcmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_mult[m]);
    egamegamAB_multAB[m] = new TH2F(Form("egamegamABmultAB%d",m),Form("egamegamABmultAB%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_multAB[m]);
    egamegamABdc_multAB[m] = new TH2F(Form("egamegamABdcmultAB%d",m),Form("egamegamABdcmultAB%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_multAB[m]);

    egamegam_IDgate_mult[m] = new TH2F(Form("egamegammult%d_IDgate",m),Form("egamegammult%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegam_IDgate_mult[m]);
    egamegamdc_IDgate_mult[m] = new TH2F(Form("egamegamdcmult%d_IDgate",m),Form("egamegamdcmult%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegamdc_IDgate_mult[m]);
    egamegamAB_IDgate_mult[m] = new TH2F(Form("egamegamABmult%d_IDgate",m),Form("egamegamABmult%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_IDgate_mult[m]);
    egamegamABdc_IDgate_mult[m] = new TH2F(Form("egamegamABdcmult%d_IDgate",m),Form("egamegamABdcmult%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_IDgate_mult[m]);
    egamegamAB_IDgate_multAB[m] = new TH2F(Form("egamegamABmultAB%d_IDgate",m),Form("egamegamABmultAB%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_IDgate_multAB[m]);
    egamegamABdc_IDgate_multAB[m] = new TH2F(Form("egamegamABdcmultAB%d_IDgate",m),Form("egamegamABdcmultAB%d_IDgate",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_IDgate_multAB[m]);
  }

  TH2F* egamdcmult_trig[10];
  TH2F* egamABdcmult_trig[10];
  TH2F* egamABdcmultAB_trig[10];
  TH2F* egamdcmult_IDgate_trig[10];
  TH2F* egamABdcmult_IDgate_trig[10];
  TH2F* egamABdcmultAB_IDgate_trig[10];
  for(int i=0;i<10;i++){
    egamdcmult_trig[i] = new TH2F(Form("egamdcmult_trig%d",i),Form("egamdcmult_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamdcmult_trig[i]);
    egamABdcmult_trig[i] = new TH2F(Form("egamABdcmult_trig%d",i),Form("egamABdcmult_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamABdcmult_trig[i]);
    egamABdcmultAB_trig[i] = new TH2F(Form("egamABdcmultAB_trig%d",i),Form("egamABdcmultAB_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamABdcmultAB_trig[i]);
    egamdcmult_IDgate_trig[i] = new TH2F(Form("egamdcmult_IDgate_trig%d",i),Form("egamdcmult_IDgate_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamdcmult_IDgate_trig[i]);
    egamABdcmult_IDgate_trig[i] = new TH2F(Form("egamABdcmult_IDgate_trig%d",i),Form("egamABdcmult_IDgate_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamABdcmult_IDgate_trig[i]);
    egamABdcmultAB_IDgate_trig[i] = new TH2F(Form("egamABdcmultAB_IDgate_trig%d",i),Form("egamABdcmultAB_IDgate_trig%d",i),20,0,20,bins,0,bins);hlist->Add(egamABdcmultAB_IDgate_trig[i]);
  }


  // //temp
  // TFile *fc = new TFile("/home/wimmer/ribf94/cuts/coinc.root");
  // TCutG *gc = (TCutG*)fc->Get("cscoinc");
  // fc->Close();

  Double_t nentries = tr->GetEntries();
  cout << nentries << " entries in tree" << endl;
  if(nentries<1)
    return 4;
  if(LastEvent>0)
    nentries = LastEvent;
  
  Int_t nbytes = 0;
  Int_t status;
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    trigbit = 0;
    for(int f=0;f<NFPLANES;f++){
      fp[f]->Clear();
    }
    dali->Clear();
    ppac->Clear();
    beam->Clear();
    if(Verbose>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(Verbose>2)
      cout << "status " << status << endl;
    if(status == -1){
      cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<tr->GetName()<<" in file "<<tr->GetFile()->GetName()<<endl;
      return 5;
    }
    else if(status == 0){
      cerr<<"Error occured, entry "<<i<<" in tree "<<tr->GetName()<<" in file "<<tr->GetFile()->GetName()<<" doesn't exist"<<endl;
      return 6;
    }
    nbytes += status;
    
    //gate on F5X position 
    if(!rec->F5XGate(fp[fpNr(5)]->GetTrack()->GetX()))
      continue;

    //gate on charge changes in ZeroDegree
    if(rec->ChargeChange(beam->GetDelta(2),beam->GetDelta(3)))
      continue;

    //analysis

    // filter over and underflow
    dali->SetHits(rec->FilterBadHits(dali->GetHits()));

    // apply timing gate
    dali->SetHits(rec->TimingGate(dali->GetHits()));

    // recalibration second order
    if(recalibrate)
      rec->ReCalibrate(dali->GetHits());

    // sort by energy
    dali->SetHits(rec->Sort(dali->GetHits()));

    // set the positions
    rec->SetPositions(dali);

    // addback
    dali->SetABHits(rec->Addback(dali->GetHits()));
 
    // sort by energy
    dali->SetABHits(rec->Sort(dali->GetHitsAB()));

    // Doppler correction
    rec->DopplerCorrect(dali);

    if(Verbose>2)
      dali->Print();

    Plastic *pl3 = fp[fpNr(3)]->GetPlastic();
    PL3PPAC3->Fill(pl3->GetTimeL() - pl3->GetTimeR(), fp[fpNr(3)]->GetTrack()->GetX());
    PL3PL3->Fill(log(pl3->GetChargeL()/pl3->GetChargeR()), pl3->GetTimeL() - pl3->GetTimeR());
    Plastic *pl7 = fp[fpNr(7)]->GetPlastic();
    PL7PPAC7->Fill(pl7->GetTimeL() - pl7->GetTimeR(), fp[fpNr(7)]->GetTrack()->GetX());
    PL7PL7->Fill(log(pl7->GetChargeL()/pl7->GetChargeR()), pl7->GetTimeL() - pl7->GetTimeR());
    Plastic *pl9 = fp[fpNr(9)]->GetPlastic();
    PL9PPAC9->Fill(pl9->GetTimeL() - pl9->GetTimeR(), fp[fpNr(9)]->GetTrack()->GetX());
    PL9PL9->Fill(log(pl9->GetChargeL()/pl9->GetChargeR()), pl9->GetTimeL() - pl9->GetTimeR());
    Plastic *pl11 = fp[fpNr(11)]->GetPlastic();
    PL11PPAC11->Fill(pl11->GetTimeL() - pl11->GetTimeR(), fp[fpNr(11)]->GetTrack()->GetX());
    PL11PL11->Fill(log(pl11->GetChargeL()/pl11->GetChargeR()), pl11->GetTimeL() - pl11->GetTimeR());
    
    //beam direction, scattering angle
    //align
    rec->AlignPPAC(ppac->GetPPACID(35),ppac->GetPPACID(36));
    TVector3 ppacpos[3];
    ppacpos[0] = rec->PPACPosition(ppac->GetPPACID(19),ppac->GetPPACID(20));
    ppacpos[1] = rec->PPACPosition(ppac->GetPPACID(21),ppac->GetPPACID(22));
    ppacpos[2] = rec->PPACPosition(ppac->GetPPACID(35),ppac->GetPPACID(36));
    
    // cout << ppac->GetPPACID(19)->GetZ() <<"\t" <<  ppac->GetPPACID(20)->GetZ()<<"\t"<<ppacpos[0].Z() << endl;
    // cout << ppac->GetPPACID(21)->GetZ() <<"\t" <<  ppac->GetPPACID(22)->GetZ()<<"\t"<<ppacpos[1].Z() << endl;
    // cout << ppac->GetPPACID(35)->GetZ() <<"\t" <<  ppac->GetPPACID(36)->GetZ()<<"\t"<<ppacpos[2].Z() << endl;

    
    beam->SetIncomingDirection(ppacpos[1]-ppacpos[0]);
    TVector3 inc = beam->GetIncomingDirection();
    TVector3 targ = rec->TargetPosition(inc,ppacpos[1]);
    //cout << setw(5) << setprecision(5)<< ppacpos[0].X() <<"\t" << ppacpos[0].Y() <<"\t" << ppacpos[0].Z() <<"\t" << ppacpos[1].X() <<"\t" << ppacpos[1].Y() <<"\t" << ppacpos[1].Z() <<"\t" << targ.X() <<"\t" << targ.Y() <<"\t" << targ.Z() <<"\t" << ppacpos[2].X() <<"\t" << ppacpos[2].Y() <<"\t" << ppacpos[2].Z() << endl;
    beam->SetTargetPosition(targ);
    TVector3 out, sca;
    if(trigbit>1){
      beam->SetOutgoingDirection(ppacpos[2]-targ);
      out = beam->GetOutgoingDirection();
      sca = beam->GetScatteredDirection();
    }
    
    //histos
    trigger->Fill(trigbit);
    bigrips->Fill(beam->GetAQ(br),beam->GetZ(br));
    zerodeg->Fill(beam->GetAQ(zd),beam->GetZ(zd));
    if(trigbit>-1 && trigbit<10){
      bigrips_tr[trigbit]->Fill(beam->GetAQ(br),beam->GetZ(br));
      zerodeg_tr[trigbit]->Fill(beam->GetAQ(zd),beam->GetZ(zd));
      f5X_tr[trigbit]->Fill(fp[fpNr(5)]->GetTrack()->GetX());
      thetaphi_tr[trigbit]->Fill(beam->GetPhi(),beam->GetTheta()*1000);
      thetaphideg_tr[trigbit]->Fill(beam->GetPhi()*rad2deg,beam->GetTheta()*rad2deg);
    }
    
    // BEAM
    if(trigbit>1){
      //cout << beam->GetPhi() <<"\t" << sca.Phi() - inc.Phi() << endl;
      thetaphi->Fill(beam->GetPhi(),beam->GetTheta()*1000);
      thetaphideg->Fill(beam->GetPhi()*rad2deg,beam->GetTheta()*rad2deg);
      thetaphiin->Fill(inc.Phi(),inc.Theta()*1000);
      thetaphiout->Fill(out.Phi(),out.Theta()*1000);
      thetaphisca->Fill(sca.Phi(),sca.Theta()*1000);
      phiinout->Fill(inc.Phi(),out.Phi());
    }
    //beam
    for(unsigned short b=0;b<3;b++)
      bbeta[b]->Fill(beam->GetBeta(b));
    for(unsigned short b=0;b<4;b++)
      delta[b]->Fill(beam->GetDelta(b));
    deltadiff[0]->Fill(beam->GetDelta(1) - beam->GetDelta(0));
    deltadiff[1]->Fill(beam->GetDelta(3) - beam->GetDelta(2));
    
    double a = inc.X()/inc.Z();
    double b = inc.Y()/inc.Z();

    compareA->Fill(atan2(inc.X(),inc.Z())*1000, fp[fpNr(8)]->GetTrack()->GetA());
    compareB->Fill(atan2(inc.Y(),inc.Z())*1000, fp[fpNr(8)]->GetTrack()->GetB());
    targetXY->Fill(targ.X(),targ.Y());
    incAB->Fill(atan2(inc.X(),inc.Z())*1000,atan2(inc.Y(),inc.Z())*1000);

    scaAB->Fill(atan2(sca.X(),sca.Z())*1000,atan2(sca.Y(),sca.Z())*1000);
    

    double x = ppacpos[1].X() + a * (ppac->GetPPACID(35)->GetZ()-ppacpos[1].Z());
    double y = ppacpos[1].Y() + b * (ppac->GetPPACID(35)->GetZ()-ppacpos[1].Z());
    compareX[0]->Fill(ppac->GetPPACID(35)->GetX(),x);
    compareY[0]->Fill(ppac->GetPPACID(35)->GetY(),y);
    compare1dX[0]->Fill(ppac->GetPPACID(35)->GetX()-x);
    compare1dY[0]->Fill(ppac->GetPPACID(35)->GetY()-y);

    x = ppacpos[1].X() + a * (ppac->GetPPACID(36)->GetZ()-ppacpos[1].Z());
    y = ppacpos[1].Y() + b * (ppac->GetPPACID(36)->GetZ()-ppacpos[1].Z());
    compareX[1]->Fill(ppac->GetPPACID(36)->GetX(),x);
    compareY[1]->Fill(ppac->GetPPACID(36)->GetY(),y);
    compare1dX[1]->Fill(ppac->GetPPACID(36)->GetX()-x);
    compare1dY[1]->Fill(ppac->GetPPACID(36)->GetY()-y);

    // PPACs
    for(unsigned short p=0;p<ppac->GetN();p++){
      SinglePPAC *sp = ppac->GetPPAC(p);
      ppacZpos->Fill(sp->GetID(),sp->GetZ());
      if(sp->GetID()>=19 && sp->GetID()<=22){
	f8ppacX[sp->GetID()-19]->Fill(sp->GetX());
	f8ppacY[sp->GetID()-19]->Fill(sp->GetY());
	if(sp->Fired())
	  f8ppacXY[sp->GetID()-19]->Fill(sp->GetX(),sp->GetY());
      }
      if(sp->GetID()>=35 && sp->GetID()<=36){
	f8ppacX[sp->GetID()-35+4]->Fill(sp->GetX());
	f8ppacY[sp->GetID()-35+4]->Fill(sp->GetY());
	if(sp->Fired())
	  f8ppacXY[sp->GetID()-35+4]->Fill(sp->GetX(),sp->GetY());
      }
    
    }
    

    // DALI
    mult->Fill(dali->GetMult());
    multtrig->Fill(trigbit,dali->GetMult());
    for(unsigned short k=0;k<dali->GetMult();k++){
      ID_theta->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetPos().Theta()*180/TMath::Pi());
      egam->Fill(dali->GetHit(k)->GetEnergy());
      egamdc->Fill(dali->GetHit(k)->GetDCEnergy()); 
      triggertgam->Fill(dali->GetHit(k)->GetTOffset(),trigbit);
      egamtgam->Fill(dali->GetHit(k)->GetTOffset(),dali->GetHit(k)->GetEnergy());
      egamdctgam->Fill(dali->GetHit(k)->GetTOffset(),dali->GetHit(k)->GetDCEnergy());    
      egamtrig->Fill(trigbit,dali->GetHit(k)->GetEnergy());
      //egamdctrig->Fill(trigbit,dali->GetHit(k)->GetDCEnergy());
      egammult->Fill(dali->GetMult(),dali->GetHit(k)->GetEnergy());
      egamdcmult->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      egamdcmult_trig[trigbit]->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      if(dali->GetHit(k)->GetID()>=minID){
	egamdcmult_IDgate_trig[trigbit]->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      }
      egamID_mult[0]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
      egamdcID_mult[0]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
      egamdctrig_mult[0]->Fill(trigbit,dali->GetHit(k)->GetDCEnergy());
      egamdctrigmult_theta[trigbit][0]->Fill(beam->GetTheta()*rad2deg,dali->GetHit(k)->GetDCEnergy());
	
      egamdctheta_mult[0]->Fill(dali->GetHit(k)->GetPos().Theta(),dali->GetHit(k)->GetDCEnergy());
      egamdcphi_mult[0]->Fill(dali->GetHit(k)->GetPos().Phi(),dali->GetHit(k)->GetDCEnergy());
      if(dali->GetMult()<9){
	egamID_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
	egamdcID_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
	egamdctrig_mult[dali->GetMult()]->Fill(trigbit,dali->GetHit(k)->GetDCEnergy());
	egamdctrigmult_theta[trigbit][dali->GetMult()]->Fill(beam->GetTheta()*rad2deg,dali->GetHit(k)->GetDCEnergy());
	egamdctheta_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetPos().Theta(),dali->GetHit(k)->GetDCEnergy());
	egamdcphi_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetPos().Phi(),dali->GetHit(k)->GetDCEnergy());
      }
      else{
	egamID_mult[9]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
	egamdcID_mult[9]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
	egamdctrig_mult[9]->Fill(trigbit,dali->GetHit(k)->GetDCEnergy());
	egamdctrigmult_theta[trigbit][9]->Fill(beam->GetTheta()*rad2deg,dali->GetHit(k)->GetDCEnergy());
	egamdctheta_mult[9]->Fill(dali->GetHit(k)->GetPos().Theta(),dali->GetHit(k)->GetDCEnergy());
	egamdcphi_mult[9]->Fill(dali->GetHit(k)->GetPos().Phi(),dali->GetHit(k)->GetDCEnergy());
      }
      if(dali->GetHit(k)->GetID()>=minID){
	egam_IDgate->Fill(dali->GetHit(k)->GetEnergy());
	egamdc_IDgate->Fill(dali->GetHit(k)->GetDCEnergy()); 
	egammult_IDgate->Fill(dali->GetMult(),dali->GetHit(k)->GetEnergy());
	egamdcmult_IDgate->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      }
    }

    multAB->Fill(dali->GetMultAB());
    for(unsigned short k=0;k<dali->GetMultAB();k++){
      egamAB->Fill(dali->GetHitAB(k)->GetEnergy());
      egamABdc->Fill(dali->GetHitAB(k)->GetDCEnergy());
      egamABtrig->Fill(trigbit,dali->GetHitAB(k)->GetEnergy());
      //egamABdctrig->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
      egamABmult->Fill(dali->GetMult(),dali->GetHitAB(k)->GetEnergy());    
      egamABmultAB->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetEnergy());    
      egamABID_mult[0]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
      egamABdcmult->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcmultAB->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcmult_trig[trigbit]->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcmultAB_trig[trigbit]->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());    
      if(dali->GetHitAB(k)->GetID()>=minID){
	egamABdcmult_IDgate_trig[trigbit]->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());    
	egamABdcmultAB_IDgate_trig[trigbit]->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());    
      }
      egamABdcID_mult[0]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      egamABdctrig_mult[0]->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
      egamABdctrigmult_theta[trigbit][0]->Fill(beam->GetTheta()*rad2deg,dali->GetHitAB(k)->GetDCEnergy());
      
      if(dali->GetMult()<9){
	egamABID_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrig_mult[dali->GetMult()]->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrigmult_theta[trigbit][dali->GetMult()]->Fill(beam->GetTheta()*rad2deg,dali->GetHitAB(k)->GetDCEnergy());
      }
      else{
	egamABID_mult[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_mult[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrig_mult[9]->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrigmult_theta[trigbit][9]->Fill(beam->GetTheta()*rad2deg,dali->GetHitAB(k)->GetDCEnergy());
      }
      if(dali->GetMultAB()<9){
	egamABID_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
 	egamABdcID_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
 	egamABdctrig_multAB[dali->GetMultAB()]->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrigmultAB_theta[trigbit][dali->GetMultAB()]->Fill(beam->GetTheta()*rad2deg,dali->GetHitAB(k)->GetDCEnergy());
      }
      else{
	egamABID_multAB[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_multAB[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrig_multAB[9]->Fill(trigbit,dali->GetHitAB(k)->GetDCEnergy());
	egamABdctrigmultAB_theta[trigbit][9]->Fill(beam->GetTheta()*rad2deg,dali->GetHitAB(k)->GetDCEnergy());
      }
      if(dali->GetHitAB(k)->GetID()>=minID){
	egamAB_IDgate->Fill(dali->GetHitAB(k)->GetEnergy());
	egamABdc_IDgate->Fill(dali->GetHitAB(k)->GetDCEnergy()); 
	egamABmult_IDgate->Fill(dali->GetMult(),dali->GetHitAB(k)->GetEnergy());
	egamABmultAB_IDgate->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetEnergy());
	egamABdcmult_IDgate->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());
 	egamABdcmultAB_IDgate->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());
      }
    }

    if(dali->GetMult()>1){
      for(unsigned short k=0;k<dali->GetMult();k++){
	for(unsigned short l=k+1;l<dali->GetMult();l++){
	  tdiff->Fill(dali->GetHit(k)->GetTOffset() - dali->GetHit(l)->GetTOffset());
	  rdiff->Fill(dali->GetHit(k)->GetPos().DeltaR(dali->GetHit(l)->GetPos()));
	  adiff->Fill(dali->GetHit(k)->GetPos().Angle(dali->GetHit(l)->GetPos()));
	  radiff->Fill(dali->GetHit(k)->GetPos().Angle(dali->GetHit(l)->GetPos()),dali->GetHit(k)->GetPos().DeltaR(dali->GetHit(l)->GetPos()));
	  // if(gc->IsInside(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy())){
	  //   tdiff_coinc->Fill(dali->GetHit(k)->GetTOffset() - dali->GetHit(l)->GetTOffset());
	  //   rdiff_coinc->Fill(dali->GetHit(k)->GetPos().DeltaR(dali->GetHit(l)->GetPos()));
	  //   adiff_coinc->Fill(dali->GetHit(k)->GetPos().Angle(dali->GetHit(l)->GetPos()));
	  //   radiff_coinc->Fill(dali->GetHit(k)->GetPos().Angle(dali->GetHit(l)->GetPos()),dali->GetHit(k)->GetPos().DeltaR(dali->GetHit(l)->GetPos()));
	  // }
	  double dcen[2];
	  if(dali->GetHit(k)->GetDCEnergy() > dali->GetHit(l)->GetDCEnergy()){
	    dcen[0] = dali->GetHit(k)->GetDCEnergy();
	    dcen[1] = dali->GetHit(l)->GetDCEnergy();
	  }
	  else{
	    dcen[0] = dali->GetHit(l)->GetDCEnergy();
	    dcen[1] = dali->GetHit(k)->GetDCEnergy();
	  }
	  egamegam_mult[0]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	  egamegamdc_mult[0]->Fill(dcen[0],dcen[1]);
	  if(dali->GetMult()<9){
	    egamegam_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	    egamegamdc_mult[dali->GetMult()]->Fill(dcen[0],dcen[1]);
	  }
	  else{
	    egamegam_mult[9]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	    egamegamdc_mult[9]->Fill(dcen[0],dcen[1]);
	  }
	  if(dali->GetHit(k)->GetID()>=minID && dali->GetHit(l)->GetID()>=minID){
	    egamegam_IDgate_mult[0]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	    egamegamdc_IDgate_mult[0]->Fill(dcen[0],dcen[1]);
	    if(dali->GetMult()<9){
	      egamegam_IDgate_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	      egamegamdc_IDgate_mult[dali->GetMult()]->Fill(dcen[0],dcen[1]);
	    }
	    else{
	      egamegam_IDgate_mult[9]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	      egamegamdc_IDgate_mult[9]->Fill(dcen[0],dcen[1]);
	    }
	  }//ID gated
	}
      }
    }
    if(dali->GetMultAB()>1){
      for(unsigned short k=0;k<dali->GetMultAB();k++){
	for(unsigned short l=k+1;l<dali->GetMultAB();l++){
	  double dcen[2];
	  if(dali->GetHitAB(k)->GetDCEnergy() > dali->GetHitAB(l)->GetDCEnergy()){
	    dcen[0] = dali->GetHitAB(k)->GetDCEnergy();
	    dcen[1] = dali->GetHitAB(l)->GetDCEnergy();
	  }
	  else{
	    dcen[0] = dali->GetHitAB(l)->GetDCEnergy();
	    dcen[1] = dali->GetHitAB(k)->GetDCEnergy();
	  }
	  egamegamAB_mult[0]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	  egamegamABdc_mult[0]->Fill(dcen[0],dcen[1]);
	  if(dali->GetMult()<9){
	    egamegamAB_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_mult[dali->GetMult()]->Fill(dcen[0],dcen[1]);
	  }
	  else{
	    egamegamAB_mult[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_mult[9]->Fill(dcen[0],dcen[1]);
	  }
	  if(dali->GetMultAB()<9){
	    egamegamAB_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_multAB[dali->GetMultAB()]->Fill(dcen[0],dcen[1]);
	  }
	  else{
	    egamegamAB_multAB[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_multAB[9]->Fill(dcen[0],dcen[1]);
	  }
	  if(dali->GetHitAB(k)->GetID()>=minID && dali->GetHitAB(l)->GetID()>=minID){
	    egamegamAB_IDgate_mult[0]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_IDgate_mult[0]->Fill(dcen[0],dcen[1]);
	    if(dali->GetMult()<9){
	      egamegamAB_IDgate_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	      egamegamABdc_IDgate_mult[dali->GetMult()]->Fill(dcen[0],dcen[1]);
	    }
	    else{
	      egamegamAB_IDgate_mult[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	      egamegamABdc_IDgate_mult[9]->Fill(dcen[0],dcen[1]);
	    }
	    if(dali->GetMultAB()<9){
	      egamegamAB_IDgate_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	      egamegamABdc_IDgate_multAB[dali->GetMultAB()]->Fill(dcen[0],dcen[1]);
	    }
	    else{
	      egamegamAB_IDgate_multAB[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	      egamegamABdc_IDgate_multAB[9]->Fill(dcen[0],dcen[1]);
	    }
	  }
	}
      }
    }

    // fill tree
    rtr->Fill();
    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
    }
  }


  cout << endl;
  cout << "creating outputfile " << OutFile << endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();
  TH1F* h1;
  TH2F* h2;
  TIter next(hlist);
  while( (h1 = (TH1F*)next()) ){
    if(h1->GetEntries()>0)
      h1->Write("",TObject::kOverwrite);
  }
  while( (h2 = (TH2F*)next()) ){
    if(h2->GetEntries()>0)
      h2->Write("",TObject::kOverwrite);
  }
  if(writeTree>0)
    rtr->Write("",TObject::kOverwrite);
  ofile->Close();
  double time_end = get_time();
  cout << "Program Run time: " << time_end - time_start << " s." << endl;
  timer.Stop();
  cout << "CPU time: " << timer.CpuTime() << "\tReal time: " << timer.RealTime() << endl;
  return 0;
}
void signalhandler(int sig){
  if (sig == SIGINT){
    signal_received = true;
  }
}

double get_time(){  
    struct timeval t;  
    gettimeofday(&t, NULL);  
    double d = t.tv_sec + (double) t.tv_usec/1000000;  
    return d;  
}  
