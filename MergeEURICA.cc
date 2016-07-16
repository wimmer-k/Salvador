#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TCutG.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "TClonesArray.h"
#include "TArtEventInfo.hh"
#include "CommandLineInterface.hh"
#include "BuildEvents.hh"
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
  cout << "Merger for ERUICA and BigRIPS" << endl;
  int LastEvent = -1;
  int Verbose = 0;
  long long int Window = 10000;
  char* InputBigRIPS = NULL;
  char* InputEURICA = NULL;
  char* OutFile = NULL;
  //char* CutFile = NULL;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-b", "BigRIPS input file", &InputBigRIPS);
  interface->Add("-e", "EURICA input file", &InputEURICA);
  interface->Add("-o", "output file", &OutFile);    
  //interface->Add("-c", "cutfile", &CutFile);
  interface->Add("-w", "event building window", &Window);  
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->CheckFlags(argc, argv);
  //Complain about missing mandatory arguments
  if(InputBigRIPS == NULL){
    cout << "No BigRIPS input file given " << endl;
    return 1;
  }
  if(InputEURICA == NULL){
    cout << "No EURICA input file given " << endl;
    return 1;
  }
  if(OutFile == NULL){
    cout << "No output ROOT file given " << endl;
    return 2;
  }
  // if(CutFile == NULL){
  //   cout << "No cut file given " << endl;
  //   return 3;
  // }
  TFile* inbigrips = new TFile(InputBigRIPS);
  TTree* trbigrips = (TTree*) inbigrips->Get("tr");
  if(trbigrips == NULL){
    cout << "could not find BigRIPS tree in file " << inbigrips->GetName() << endl;
    return 4;
  }

  TFile* ineurica = new TFile(InputEURICA);
  TTree* treurica = (TTree*) ineurica->Get("tree");
  if(treurica == NULL){
    cout << "could not find EURICA tree in file " << ineurica->GetName() << endl;
    return 4;
  }
  cout<<"output file: "<<OutFile<< endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();

  BuildEvents* evts = new BuildEvents();
  evts->SetVerbose(Verbose);
  evts->SetWindow(Window);
  evts->Init(trbigrips,treurica);
  evts->SetLastEvent(LastEvent);

  evts->ReadEach();
  int ctr=0;
  int total = evts->GetNEvents();
  while(evts->Merge()){
    if(ctr%100000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*ctr)/total<<" % done\t" << 
  	(Float_t)ctr/(time_end - time_start) << " events/s " <<
  	(total-ctr)*(time_end - time_start)/(Float_t)ctr << "s to go \r" << flush;
    }
    if(signal_received){
      break;
    }
    ctr++;
  }
  
  
  evts->GetTree()->Write();
  /*
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
    ppac->Clear();
    beam->Clear();
    dali->Clear();
    if(Verbose>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(Verbose>2)
      cout << "status " << status << endl;
    if(status == -1){
      cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<tr->GetName()<<" in file "<<infile->GetName()<<endl;
      return 5;
    }
    else if(status == 0){
      cerr<<"Error occured, entry "<<i<<" in tree "<<tr->GetName()<<" in file "<<infile->GetName()<<" doesn't exist"<<endl;
      return 6;
    }
    nbytes += status;
    
    //start analysis
    for(UShort_t in=0;in<InPartCut.size();in++){ // loop over incoming cuts
      if(InPartCut[in]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	splittree[in][OutPartCut[in].size()]->Fill();
	for(UShort_t ou=0;ou<OutPartCut[in].size();ou++){ // loop over outgoing cuts
	  if(OutPartCut[in][ou]->IsInside(beam->GetAQ(5),beam->GetZ(5))){
	    splittree[in][ou]->Fill();
	  }
	}//outpartcuts
      }//inpartcuts
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
  ofile->cd();
  Long64_t filesize =0;
  for(UShort_t in=0;in<InPartCut.size();in++){ // loop over incoming cuts
    for(UShort_t ou=0;ou<OutPartCut[in].size()+1;ou++){ // loop over outgoing cuts
      splittree[in][ou]->Write("",TObject::kOverwrite);
      filesize += splittree[in][ou]->GetZipBytes();
    }
  }
  cout<<"Size of input tree  "<<setw(7)<<tr->GetZipBytes()/(1024*1024)<<" MB"<<endl
      <<"size of splitted trees "<<setw(7)<<filesize/(1024*1024)<<" MB"<<endl
      <<"=> size of splitted tree(s) is "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<(100.*filesize)/tr->GetZipBytes()<<" % of size of input tree"<<endl;
  */
  ofile->Close();
  inbigrips->Close();
  ineurica->Close();
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
