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
  cout << "\"Galatea of the Spheres\" (1952), Salvador Dali" << endl;
  cout << "Merger for WASABI, EURICA, and BigRIPS" << endl;
  int LastEvent = -1;
  int Verbose = 0;
  long long int Window = 10000;
  int Mode = 0;
  char* InputBigRIPS = NULL;
  char* InputWASABI = NULL;
  char* InputEURICA = NULL;
  char* OutFile = NULL;
  //char* CutFile = NULL;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-b", "BigRIPS input file", &InputBigRIPS);
  interface->Add("-e", "EURICA input file", &InputEURICA);
  interface->Add("-w", "WASABI input file", &InputWASABI);
  interface->Add("-o", "output file", &OutFile);    
  //interface->Add("-c", "cutfile", &CutFile);
  interface->Add("-w", "event building window", &Window);  
  // interface->Add("-m", "event building mode: 0 everything, 1 isomerdata", &Mode);  
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->CheckFlags(argc, argv);
  //Complain about missing mandatory arguments
  TTree* trbigrips = NULL;
  TTree* treurica = NULL;
  TTree* trwasabi = NULL;
  TFile* inbigrips = NULL;
  TFile* ineurica = NULL;
  TFile* inwasabi = NULL;
  
  if(InputBigRIPS == NULL){
    cout << "No BigRIPS input file given " << endl;
  }
  else{
    inbigrips = new TFile(InputBigRIPS);
    trbigrips = (TTree*) inbigrips->Get("tr");
    if(trbigrips == NULL){
      cout << "could not find BigRIPS tree in file " << inbigrips->GetName() << endl;
    }
  }
  if(InputEURICA == NULL){
    cout << "No EURICA input file given " << endl;
  }
  else{
    ineurica = new TFile(InputEURICA);
    treurica = (TTree*) ineurica->Get("tree");
    if(treurica == NULL){
      cout << "could not find EURICA tree in file " << ineurica->GetName() << endl;
    }
  }
  if(InputWASABI == NULL){
    cout << "No WASABI input file given " << endl;
  }
  else{
    inwasabi = new TFile(InputWASABI);
    trwasabi = (TTree*) inwasabi->Get("tr");
    if(trwasabi == NULL){
      cout << "could not find WASABI tree in file " << inwasabi->GetName() << endl;
    }
  }
  if(OutFile == NULL){
    cout << "No output ROOT file given " << endl;
    return 2;
  }

  cout<<"output file: "<<OutFile<< endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();
 
  BuildEvents* evts = new BuildEvents();
  evts->SetVerbose(Verbose);
  evts->SetWindow(Window);
  evts->SetCoincMode(Mode);  
  evts->Init(trbigrips,trwasabi,treurica);
  evts->SetLastEvent(LastEvent);

  double time_last = get_time();
  evts->ReadEach();
  int ctr=0;
  int total = evts->GetNEvents();
  while(evts->Merge()){
    if(ctr%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*ctr)/total<<" % done\t" << 
    	(Float_t)ctr/(time_end - time_start) << " events/s (average) " <<
    	10000./(time_end - time_last) << " events/s (current) " <<
    	(total-ctr)*(time_end - time_start)/(Float_t)ctr << "s to go \r" << flush;
      time_last = time_end;
    }
    if(signal_received){
      break;
    }
    ctr++;
  }
  
  evts->CloseEvent();
  evts->GetTree()->Write("",TObject::kOverwrite);
  ofile->Close();
  if(inbigrips!=NULL)
    inbigrips->Close();
  if(inwasabi!=NULL)
    inwasabi->Close();
  if(ineurica!=NULL)
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
