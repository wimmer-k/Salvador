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
  cout << "\"The Temptation of St. Anthony\" (1946), Salvador Dali" << endl;
  cout << "Analyzer for DALI and MINOS (simulation)" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char *SetFile = NULL;
  int minID = 79;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-s", "settings file", &SetFile);    
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->Add("-id", "minimum DALI ID for additional gamma-gamma spectra", &minID);

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
  tr = new TChain("tr");
  TChain* tobs;
  tobs = new TChain("ObservedEvents");
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
    tobs->Add(InputFiles[i]);
  }
  if(tr == NULL){
    cout << "could not find tree tr in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  if(tobs == NULL){
    cout << "could not find tree ObservedEvents in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  DALI* dali = new DALI;
  tr->SetBranchAddress("dali",&dali);
  float p0[3];
  tobs->SetBranchAddress("ProjectileVertex",&p0);
  
  cout<<"settings file: " << SetFile <<endl;
  Reconstruction *rec = new Reconstruction(SetFile);
  if(Verbose>0){
    rec->GetSettings()->SetVerboseLevel(Verbose);
    rec->GetSettings()->PrintSettings();
  }

  TList *hlist = new TList();

  TH1F* tdiff = new TH1F("tdiff","tdiff",2000,-1000,1000);hlist->Add(tdiff);
  TH1F* rdiff = new TH1F("rdiff","rdiff",2000,0,10);hlist->Add(rdiff);
  TH1F* adiff = new TH1F("adiff","adiff",2000,0,4);hlist->Add(adiff);
  TH2F* radiff = new TH2F("radiff","radiff",200,0,4,200,0,10);hlist->Add(radiff);

  TH2F* beta_z = new TH2F("beta_z","beta_z",500,-5,5,500,0.4,0.6);;hlist->Add(beta_z);
  
  int bins = 8000;
  
  TH1F* mult = new TH1F("mult","mult",50,0,50);hlist->Add(mult);
  TH1F* egam = new TH1F("egam","egam",bins,0,bins);hlist->Add(egam);
  TH1F* egamdc = new TH1F("egamdc","egamdc",bins,0,bins);hlist->Add(egamdc);
  TH1F* egam_IDgate = new TH1F("egam_IDgate","egam_IDgate",bins,0,bins);hlist->Add(egam_IDgate);
  TH1F* egamdc_IDgate = new TH1F("egamdc_IDgate","egamdc_IDgate",bins,0,bins);hlist->Add(egamdc_IDgate);
  TH2F* egamtgam = new TH2F("egamtgam","egamtgam",1000,-500,500,1000,0,bins);hlist->Add(egamtgam);
  TH2F* egamdc_z = new TH2F("egamdc_z","egamdc_z",500,-5,5,1000,0,bins);hlist->Add(egamdc_z);
  TH2F* egamdctgam = new TH2F("egamdctgam","egamdctgam",1000,-500,500,1000,0,bins);hlist->Add(egamdctgam);
  TH2F* egammult = new TH2F("egammult","egammult",20,0,20,bins,0,bins);hlist->Add(egammult);
  TH2F* egamdcmult = new TH2F("egamdcmult","egamdcmult",20,0,20,bins,0,bins);hlist->Add(egamdcmult);
  TH2F* egammult_IDgate = new TH2F("egammult_IDgate","egammult_IDgate",20,0,20,bins,0,bins);hlist->Add(egammult_IDgate);
  TH2F* egamdcmult_IDgate = new TH2F("egamdcmult_IDgate","egamdcmult_IDgate",20,0,20,bins,0,bins);hlist->Add(egamdcmult_IDgate);
  TH2F* egamID_mult[10];
  TH2F* egamdcID_mult[10];
  TH2F* egamdctheta_mult[10];
  TH2F* egamdcphi_mult[10];
  TH2F* egamegam_mult[10];
  TH2F* egamegamdc_mult[10];
  TH2F* egamegam_IDgate_mult[10];
  TH2F* egamegamdc_IDgate_mult[10];
  TH1F* multAB = new TH1F("multAB","multAB",50,0,50);hlist->Add(multAB);
  TH1F* egamAB = new TH1F("egamAB","egamAB",bins,0,bins);hlist->Add(egamAB);
  TH1F* egamABdc = new TH1F("egamABdc","egamABdc",bins,0,bins);hlist->Add(egamABdc);
  TH2F* egamABdc_z = new TH2F("egamABdc_z","egamABdc_z",500,-5,5,bins,0,bins);hlist->Add(egamABdc_z);
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
  TH2F* egamABID_mult[10];
  TH2F* egamABID_multAB[10];
  TH2F* egamABdcID_mult[10];
  TH2F* egamABdcID_multAB[10];
 
  TH2F* egamegamAB_mult[10];
  TH2F* egamegamABdc_mult[10];
  TH2F* egamegamAB_multAB[10];
  TH2F* egamegamABdc_multAB[10];
  TH2F* egamegamAB_IDgate_mult[10];
  TH2F* egamegamABdc_IDgate_mult[10];
  TH2F* egamegamAB_IDgate_multAB[10];
  TH2F* egamegamABdc_IDgate_multAB[10];

  egamID_mult[0] = new TH2F("egamID","egamID",200,0,200,bins,0,bins);hlist->Add(egamID_mult[0]);
  egamdcID_mult[0] = new TH2F("egamdcID","egamdcID",200,0,200,bins,0,bins);hlist->Add(egamdcID_mult[0]);
  egamdctheta_mult[0] = new TH2F("egamdctheta","egamdctheta",200,0,4,400,0,bins);hlist->Add(egamdctheta_mult[0]);
  egamdcphi_mult[0] = new TH2F("egamdcphi","egamdcphi",200,-4,4,400,0,bins);hlist->Add(egamdcphi_mult[0]);
  egamABID_mult[0] = new TH2F("egamABID","egamABID",200,0,200,bins,0,bins);hlist->Add(egamABID_mult[0]);
  egamABdcID_mult[0] = new TH2F("egamABdcID","egamABdcID",200,0,200,bins,0,bins);hlist->Add(egamABdcID_mult[0]);
  
  egamegam_mult[0] = new TH2F("egamegam","egamegam",200,0,4000,200,0,4000);hlist->Add(egamegam_mult[0]);
  egamegamdc_mult[0] = new TH2F("egamegamdc","egamegamdc",200,0,4000,200,0,4000);hlist->Add(egamegamdc_mult[0]);
  egamegamAB_mult[0] = new TH2F("egamegamAB","egamegamAB",200,0,4000,200,0,4000);hlist->Add(egamegamAB_mult[0]);
  egamegamABdc_mult[0] = new TH2F("egamegamABdc","egamegamABdc",200,0,4000,200,0,4000);hlist->Add(egamegamABdc_mult[0]);
  egamegam_IDgate_mult[0] = new TH2F("egamegam_IDgate","egamegam_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegam_IDgate_mult[0]);
  egamegamdc_IDgate_mult[0] = new TH2F("egamegamdc_IDgate","egamegamdc_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamdc_IDgate_mult[0]);
  egamegamAB_IDgate_mult[0] = new TH2F("egamegamAB_IDgate","egamegamAB_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamAB_IDgate_mult[0]);
  egamegamABdc_IDgate_mult[0] = new TH2F("egamegamABdc_IDgate","egamegamABdc_IDgate",200,0,4000,200,0,4000);hlist->Add(egamegamABdc_IDgate_mult[0]);
  for(int m=1;m<10;m++){
    egamID_mult[m] = new TH2F(Form("egamIDmult%d",m),Form("egamIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamID_mult[m]);
    egamdcID_mult[m] = new TH2F(Form("egamdcIDmult%d",m),Form("egamdcIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamdcID_mult[m]);
    egamdctheta_mult[m] = new TH2F(Form("egamdcthetamult%d",m),Form("egamdcthetamult%d",m),200,0,4,400,0,4000);hlist->Add(egamdctheta_mult[m]);
    egamdcphi_mult[m] = new TH2F(Form("egamdcphimult%d",m),Form("egamdcphimult%d",m),200,-4,4,400,0,4000);hlist->Add(egamdcphi_mult[m]);
    egamABID_mult[m] = new TH2F(Form("egamABIDmult%d",m),Form("egamABIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamABID_mult[m]);
    egamABID_multAB[m] = new TH2F(Form("egamABIDmultAB%d",m),Form("egamABIDmultAB%d",m),200,0,200,400,0,4000);hlist->Add(egamABID_multAB[m]);
    egamABdcID_mult[m] = new TH2F(Form("egamABdcIDmult%d",m),Form("egamABdcIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamABdcID_mult[m]);
    egamABdcID_multAB[m] = new TH2F(Form("egamABdcIDmultAB%d",m),Form("egamABdcIDmultAB%d",m),200,0,200,400,0,4000);hlist->Add(egamABdcID_multAB[m]);
    
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
    dali->Clear();
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
    status = tobs->GetEvent(i);

    //analysis

    // filter over and underflow
    dali->SetHits(rec->FilterBadHits(dali->GetHits()));

    // apply timing gate
    dali->SetHits(rec->TimingGate(dali->GetHits()));

     // sort by energy
    dali->SetHits(rec->Sort(dali->GetHits()));

    // set the positions
    rec->SetPositions(dali);

    // addback
    dali->SetABHits(rec->Addback(dali->GetHits()));
 
    // sort by energy
    dali->SetABHits(rec->Sort(dali->GetHitsAB()));

    // Doppler correction
    double recbeta  = rec->DopplerCorrect(dali,p0[2]);
    if(Verbose>2)
      dali->Print();

    beta_z->Fill(p0[2],recbeta);

    // DALI
    mult->Fill(dali->GetMult());
     for(unsigned short k=0;k<dali->GetMult();k++){
      egam->Fill(dali->GetHit(k)->GetEnergy());
      egamdc->Fill(dali->GetHit(k)->GetDCEnergy()); 
      egamdc_z->Fill(p0[2],dali->GetHit(k)->GetDCEnergy()); 
      egamtgam->Fill(dali->GetHit(k)->GetTOffset(),dali->GetHit(k)->GetEnergy());
      egamdctgam->Fill(dali->GetHit(k)->GetTOffset(),dali->GetHit(k)->GetDCEnergy());    
      egammult->Fill(dali->GetMult(),dali->GetHit(k)->GetEnergy());
      egamdcmult->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      egamID_mult[0]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
      egamdcID_mult[0]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
	
      egamdctheta_mult[0]->Fill(dali->GetHit(k)->GetPos().Theta(),dali->GetHit(k)->GetDCEnergy());
      egamdcphi_mult[0]->Fill(dali->GetHit(k)->GetPos().Phi(),dali->GetHit(k)->GetDCEnergy());
      if(dali->GetMult()<9){
	egamID_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
	egamdcID_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
	egamdctheta_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetPos().Theta(),dali->GetHit(k)->GetDCEnergy());
	egamdcphi_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetPos().Phi(),dali->GetHit(k)->GetDCEnergy());
      }
      else{
	egamID_mult[9]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetEnergy());
	egamdcID_mult[9]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
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
      egamABdc_z->Fill(p0[2],dali->GetHitAB(k)->GetDCEnergy());
      egamABmult->Fill(dali->GetMult(),dali->GetHitAB(k)->GetEnergy());    
      egamABmultAB->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetEnergy());    
      egamABID_mult[0]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
      egamABdcmult->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcmultAB->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcID_mult[0]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      
      if(dali->GetMult()<9){
	egamABID_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      }
      else{
	egamABID_mult[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_mult[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      }
      if(dali->GetMultAB()<9){
	egamABID_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
 	egamABdcID_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      }
      else{
	egamABID_multAB[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetEnergy());
	egamABdcID_multAB[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
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
