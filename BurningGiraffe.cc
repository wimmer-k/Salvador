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
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "FocalPlane.hh"
#include "PPAC.hh"
#include "Beam.hh"
#include "DALI.hh"
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
  cout << "\"The Burning Giraffe\" (1937), Salvator Dali" << endl;
  cout << "Histogramms for DALI" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char* TreeName = (char*)"tr";

  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-tn", "name of the tree", &TreeName);
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
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
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  cout<<"output file: "<<OutFile<< endl;
  TList *hlist = new TList();
  //histograms

  //ppacs
  TH2F* tsumx_id = new TH2F("tsumx_id","tsumx_id",NPPACS,0,NPPACS,2500,0,250);hlist->Add(tsumx_id);
  TH2F* tsumy_id = new TH2F("tsumy_id","tsumy_id",NPPACS,0,NPPACS,2500,0,250);hlist->Add(tsumy_id);
  TH1F* tsumx[NPPACS];
  TH1F* tsumy[NPPACS];
  for(unsigned short p=0;p<NPPACS;p++){
    tsumx[p] = new TH1F(Form("tsumx_%d",p),Form("tsumx_%d",p),1000,-200,800);hlist->Add(tsumx[p]);
    tsumy[p] = new TH1F(Form("tsumy_%d",p),Form("tsumy_%d",p),1000,-200,800);hlist->Add(tsumy[p]);
  }
  //focal planes
  TH2F* dT_vs_logQ[NFPLANES];
  TH2F* logQ_vs_X[NFPLANES];
  TH2F* dT_vs_X[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    dT_vs_logQ[f] = new TH2F(Form("dT_vs_logQ_%d",fpID[f]),Form("dT_vs_logQ_%d",fpID[f]),1000,-5,5,600,-3,3);hlist->Add( dT_vs_logQ[f]);
    logQ_vs_X[f] = new TH2F(Form("logQ_vs_X_%d",fpID[f]),Form("logQ_vs_X_%d",fpID[f]),1000,-50,50,600,-3,3);hlist->Add( logQ_vs_X[f]);
    dT_vs_X[f] = new TH2F(Form("dT_vs_X_%d",fpID[f]),Form("dT_vs_X_%d",fpID[f]),1000,-50,50,1000,-5,5);hlist->Add( dT_vs_X[f]);
  }
  //beam
  TH1F* beta[3];
  for(unsigned short b=0;b<3;b++){
    beta[b] = new TH1F(Form("beta_%d",b),Form("beta_%d",b),1000,0,1);hlist->Add(beta[b]);
  }
  TH1F* delta[4];
  for(unsigned short b=0;b<4;b++){
    delta[b] = new TH1F(Form("delta_%d",b),Form("delta_%d",b),1000,-10,10);hlist->Add(delta[b]);
  }

  //dali
  TH2F* adc_id = new TH2F("adc_id","adc_id",200,0,200,5000,0,5000);hlist->Add(adc_id);
  TH2F* en_id = new TH2F("en_id","en_id",200,0,200,500,0,2000);hlist->Add(en_id);
  TH2F* enDC_id = new TH2F("enDC_id","enDC_id",200,0,200,500,0,2000);hlist->Add(enDC_id);
  TH2F* time_id = new TH2F("time_id","time_id",200,0,200,2000,-2000,0);hlist->Add(time_id);
  TH2F* time_id_g = new TH2F("time_id_g","time_id_g",200,0,200,2000,-2000,0);hlist->Add(time_id_g);
  TH2F* toffset_id = new TH2F("toffset_id","toffset_id",200,0,200,1000,-200,800);hlist->Add(toffset_id);

  //PID
  TH2F* z_vs_aoq[6];
  TH2F* z_vs_aoqc[6];
  for(unsigned short f=0;f<6;f++){
    z_vs_aoq[f] = new TH2F(Form("z_vs_aoq_%d",f),Form("z_vs_aoq_%d",f),1000,1.8,2.2,1000,30,40);hlist->Add(z_vs_aoq[f]);
    z_vs_aoqc[f] = new TH2F(Form("z_vs_aoqc_%d",f),Form("z_vs_aoqc_%d",f),1000,1.8,2.2,1000,30,40);hlist->Add(z_vs_aoqc[f]);
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
  PPAC* ppac = new PPAC;
  tr->SetBranchAddress("ppacs",&ppac);
  FocalPlane *fp[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->SetBranchAddress(Form("fp%d",fpID[f]),&fp[f]);
  }
  Beam* beam = new Beam;
  tr->SetBranchAddress("beam",&beam);
  DALI* dali = new DALI;
  tr->SetBranchAddress("dali",&dali);
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
    ppac->Clear();
    dali->Clear();
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
    
    //start analysis

    //ppacs
    for(unsigned short p=0;p<ppac->GetN();p++){
      SinglePPAC *sp = ppac->GetPPAC(p);
      tsumx_id->Fill(sp->GetID(),sp->GetTsumX());
      tsumy_id->Fill(sp->GetID(),sp->GetTsumY());
      if(sp->GetID()>-1 && sp->GetID()<NPPACS){
	tsumx[sp->GetID()]->Fill(sp->GetTsumX());
	tsumy[sp->GetID()]->Fill(sp->GetTsumY());
      }
    }
    //dali
    for(unsigned short g=0;g<dali->GetMult();g++){
      DALIHit * hit = dali->GetHit(g);
      short id = hit->GetID();
      adc_id->Fill(id,hit->GetADC());
      en_id->Fill(id,hit->GetEnergy());
      enDC_id->Fill(id,hit->GetDCEnergy());
      time_id->Fill(id,hit->GetTime());
      if(hit->GetEnergy()>500){
	time_id_g->Fill(id,hit->GetTime());
      }
      toffset_id->Fill(id,hit->GetTOffset());
    }
    //focal planes
    for(unsigned short f=0;f<NFPLANES;f++){
      Plastic *pl = fp[f]->GetPlastic();
      dT_vs_logQ[f]->Fill(log(pl->GetChargeL()/pl->GetChargeR()), pl->GetTimeL()-pl->GetTimeR());
      logQ_vs_X[f]->Fill(fp[f]->GetTrack()->GetX(), log(pl->GetChargeL()/pl->GetChargeR()));
      dT_vs_X[f]->Fill(fp[f]->GetTrack()->GetX(), pl->GetTimeL()-pl->GetTimeR());
    }

    //beam
    for(unsigned short b=0;b<3;b++)
      beta[b]->Fill(beam->GetBeta(b));
    for(unsigned short b=0;b<4;b++)
      delta[b]->Fill(beam->GetDelta(b));
    for(unsigned short f=0;f<6;f++){
      z_vs_aoq[f]->Fill(beam->GetAQ(f),beam->GetZ(f));
      z_vs_aoqc[f]->Fill(beam->GetCorrAQ(f),beam->GetZ(f));
    }
    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
    }
  }
  cout << endl;
  cout << "creating outputfile " << endl;
  TFile* ofile = new TFile(OutFile,"recreate");
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
