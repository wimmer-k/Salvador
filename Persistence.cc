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
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  cout << "\"La persistencia de la memoria\" (1931), Salvator Dali" << endl;
  cout << "Analyzer for DALI (Add-back and Doppler-correction also for simulated data)" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char *SetFile = NULL;
  char* TreeName = (char*)"tr";
  int writeTree = 1;
  double beta =0;
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
  if(beta>0)
    rec->SetBeta(beta);

  TList *hlist = new TList();

  //histograms
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

  TH1F* tdiff = new TH1F("tdiff","tdiff",2000,-1000,1000);hlist->Add(tdiff);
  TH1F* rdiff = new TH1F("rdiff","rdiff",2000,0,10);hlist->Add(rdiff);
  TH1F* adiff = new TH1F("adiff","adiff",2000,0,4);hlist->Add(adiff);
  TH2F* radiff = new TH2F("radiff","radiff",200,0,4,200,0,10);hlist->Add(radiff);

  // TH1F* tdiff_coinc = new TH1F("tdiff_coinc","tdiff_coinc",2000,-1000,1000);hlist->Add(tdiff_coinc);
  // TH1F* rdiff_coinc = new TH1F("rdiff_coinc","rdiff_coinc",2000,0,10);hlist->Add(rdiff_coinc);
  // TH1F* adiff_coinc = new TH1F("adiff_coinc","adiff_coinc",2000,0,4);hlist->Add(adiff_coinc);
  // TH2F* radiff_coinc = new TH2F("radiff_coinc","radiff_coinc",200,0,4,200,0,10);hlist->Add(radiff_coinc);
  
  TH1F* mult = new TH1F("mult","mult",50,0,50);hlist->Add(mult);
  TH1F* egam = new TH1F("egam","egam",4000,0,4000);hlist->Add(egam);
  TH1F* egamdc = new TH1F("egamdc","egamdc",4000,0,4000);hlist->Add(egamdc);
  TH2F* egamdcmult =new TH2F("egamdcmult","egamdcmult",20,0,20,400,0,4000);hlist->Add(egamdcmult);
  TH2F* egamdcID_mult[10];
  TH2F* egamegam_mult[10];
  TH2F* egamegamdc_mult[10];
  
  TH1F* multAB = new TH1F("multAB","multAB",50,0,50);hlist->Add(multAB);
  TH1F* egamAB = new TH1F("egamAB","egamAB",4000,0,4000);hlist->Add(egamAB);
  TH1F* egamABdc = new TH1F("egamABdc","egamABdc",4000,0,4000);hlist->Add(egamABdc);
  TH2F* egamABdcmult =new TH2F("egamABdcmult","egamABdcmult",20,0,20,400,0,4000);hlist->Add(egamABdcmult);
  TH2F* egamABdcmultAB =new TH2F("egamABdcmultAB","egamABdcmultAB",20,0,20,400,0,4000);hlist->Add(egamABdcmultAB);
  TH2F* egamABdcID_mult[10];
  TH2F* egamABdcID_multAB[10];
  TH2F* egamegamAB_mult[10];
  TH2F* egamegamABdc_mult[10];
  TH2F* egamegamAB_multAB[10];
  TH2F* egamegamABdc_multAB[10];

  egamdcID_mult[0] =new TH2F("egamdcID","egamdcID",200,0,200,400,0,4000);hlist->Add(egamdcID_mult[0]);
  egamABdcID_mult[0] =new TH2F("egamABdcID","egamABdcID",200,0,200,400,0,4000);hlist->Add(egamABdcID_mult[0]);
  egamegam_mult[0] = new TH2F("egamegam","egamegam",200,0,4000,200,0,4000);hlist->Add(egamegam_mult[0]);
  egamegamdc_mult[0] = new TH2F("egamegamdc","egamegamdc",200,0,4000,200,0,4000);hlist->Add(egamegamdc_mult[0]);
  egamegamAB_mult[0] = new TH2F("egamegamAB","egamegamAB",200,0,4000,200,0,4000);hlist->Add(egamegamAB_mult[0]);
  egamegamABdc_mult[0] = new TH2F("egamegamABdc","egamegamABdc",200,0,4000,200,0,4000);hlist->Add(egamegamABdc_mult[0]);
  for(int m=1;m<10;m++){
    egamdcID_mult[m] =new TH2F(Form("egamdcIDmult%d",m),Form("egamdcIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamdcID_mult[m]);
    egamABdcID_mult[m] =new TH2F(Form("egamABdcIDmult%d",m),Form("egamABdcIDmult%d",m),200,0,200,400,0,4000);hlist->Add(egamABdcID_mult[m]);
    egamABdcID_multAB[m] =new TH2F(Form("egamABdcIDmultAB%d",m),Form("egamABdcIDmultAB%d",m),200,0,200,400,0,4000);hlist->Add(egamABdcID_multAB[m]);
    egamegam_mult[m] = new TH2F(Form("egamegammult%d",m),Form("egamegammult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegam_mult[m]);
    egamegamdc_mult[m] = new TH2F(Form("egamegamdcmult%d",m),Form("egamegamdcmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamdc_mult[m]);
    egamegamAB_mult[m] = new TH2F(Form("egamegamABmult%d",m),Form("egamegamABmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_mult[m]);
    egamegamABdc_mult[m] = new TH2F(Form("egamegamABdcmult%d",m),Form("egamegamABdcmult%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_mult[m]);
    egamegamAB_multAB[m] = new TH2F(Form("egamegamABmultAB%d",m),Form("egamegamABmultAB%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamAB_multAB[m]);
    egamegamABdc_multAB[m] = new TH2F(Form("egamegamABdcmultAB%d",m),Form("egamegamABdcmultAB%d",m),200,0,4000,200,0,4000);hlist->Add(egamegamABdc_multAB[m]);
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
    
    //analysis
    //filter over and underflow
    dali->SetHits(rec->FilterOverUnderflows(dali->GetHits()));
    //sort by energy
    dali->SetHits(rec->Sort(dali->GetHits()));
    //set the positions
    rec->SetPositions(dali);
    //addback
    dali->SetABHits(rec->Addback(dali->GetHits()));
    //sort by energy
    dali->SetABHits(rec->Sort(dali->GetHitsAB()));
    //DC
    rec->DopplerCorrect(dali);

    if(Verbose>2)
      dali->Print();

    

    //histos

    //PPACs
    //ppacs
    TVector3 ppacpos[3];
    ppacpos[0] = rec->PPACPosition(ppac->GetPPACID(19),ppac->GetPPACID(20));
    ppacpos[1] = rec->PPACPosition(ppac->GetPPACID(21),ppac->GetPPACID(22));
    ppacpos[2] = rec->PPACPosition(ppac->GetPPACID(35),ppac->GetPPACID(36));
    
    beam->SetIncomingDirection(ppacpos[1]-ppacpos[0]);
    TVector3 inc = beam->GetIncomingDirection();
    compareA->Fill(atan2(inc.X(),inc.Z())*1000, fp[fpNr(8)]->GetTrack()->GetA());
    compareB->Fill(atan2(inc.Y(),inc.Z())*1000, fp[fpNr(8)]->GetTrack()->GetB());
    
    TVector3 targ = rec->TargetPosition(inc,ppacpos[1]);
    beam->SetTargetPosition(targ);

    double a = inc.X()/inc.Z();
    double b = inc.Y()/inc.Z();

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


    //DALI
    mult->Fill(dali->GetMult());
    for(unsigned short k=0;k<dali->GetMult();k++){
      egam->Fill(dali->GetHit(k)->GetEnergy());
      egamdc->Fill(dali->GetHit(k)->GetDCEnergy());    
      egamdcmult->Fill(dali->GetMult(),dali->GetHit(k)->GetDCEnergy());
      egamdcID_mult[0]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
      if(dali->GetMult()<9)
	egamdcID_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
      else
	egamdcID_mult[9]->Fill(dali->GetHit(k)->GetID(),dali->GetHit(k)->GetDCEnergy());
    }



    multAB->Fill(dali->GetMultAB());
    for(unsigned short k=0;k<dali->GetMultAB();k++){
      egamAB->Fill(dali->GetHitAB(k)->GetEnergy());
      egamABdc->Fill(dali->GetHitAB(k)->GetDCEnergy());
      egamABdcmult->Fill(dali->GetMult(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcmultAB->Fill(dali->GetMultAB(),dali->GetHitAB(k)->GetDCEnergy());    
      egamABdcID_mult[0]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      if(dali->GetMult()<9)
	egamABdcID_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      else
	egamABdcID_mult[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      if(dali->GetMultAB()<9)
	egamABdcID_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
      else
	egamABdcID_multAB[9]->Fill(dali->GetHitAB(k)->GetID(),dali->GetHitAB(k)->GetDCEnergy());
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
	  egamegam_mult[0]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	  egamegamdc_mult[0]->Fill(dali->GetHit(k)->GetDCEnergy(),dali->GetHit(l)->GetDCEnergy());
	  if(dali->GetMult()<9){
	    egamegam_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	    egamegamdc_mult[dali->GetMult()]->Fill(dali->GetHit(k)->GetDCEnergy(),dali->GetHit(l)->GetDCEnergy());
	  }
	  else{
	    egamegam_mult[9]->Fill(dali->GetHit(k)->GetEnergy(),dali->GetHit(l)->GetEnergy());
	    egamegamdc_mult[9]->Fill(dali->GetHit(k)->GetDCEnergy(),dali->GetHit(l)->GetDCEnergy());
	  }
	}
      }
    }
    if(dali->GetMultAB()>1){
      for(unsigned short k=0;k<dali->GetMultAB();k++){
	for(unsigned short l=k+1;l<dali->GetMultAB();l++){
	  egamegamAB_mult[0]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	  egamegamABdc_mult[0]->Fill(dali->GetHitAB(k)->GetDCEnergy(),dali->GetHitAB(l)->GetDCEnergy());
	  if(dali->GetMult()<9){
	    egamegamAB_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_mult[dali->GetMult()]->Fill(dali->GetHitAB(k)->GetDCEnergy(),dali->GetHitAB(l)->GetDCEnergy());
	  }
	  else{
	    egamegamAB_mult[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_mult[9]->Fill(dali->GetHitAB(k)->GetDCEnergy(),dali->GetHitAB(l)->GetDCEnergy());
	  }
	  if(dali->GetMultAB()<9){
	    egamegamAB_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_multAB[dali->GetMultAB()]->Fill(dali->GetHitAB(k)->GetDCEnergy(),dali->GetHitAB(l)->GetDCEnergy());
	  }
	  else{
	    egamegamAB_multAB[9]->Fill(dali->GetHitAB(k)->GetEnergy(),dali->GetHitAB(l)->GetEnergy());
	    egamegamABdc_multAB[9]->Fill(dali->GetHitAB(k)->GetDCEnergy(),dali->GetHitAB(l)->GetDCEnergy());
	  }
	}
      }
    }
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
