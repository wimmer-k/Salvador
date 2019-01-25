#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include <signal.h>
#include "TArtStoreManager.hh"
#include "TArtEventStore.hh"
#include "TArtEventInfo.hh"

#include "TFile.h"
#include "TTree.h"
#include "TStopwatch.h"


#include "CommandLineInterface.hh"
#include "Calibration.hh"

#include "WASABI.hh"
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
  cout << "\"The Flames, They Call\" (1942), Salvador Dali" << endl;
  cout << "Unpacker for WAS3Abi" << endl;
  char* InputFile;
  char* OutputFile = NULL;
  char* SetFile = NULL;
  char* CutFile = NULL;
  int nmax = 0;
  int vl = 0;

  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input file", &InputFile);
  interface->Add("-o", "output file", &OutputFile);
  interface->Add("-s", "settings file", &SetFile);
  interface->Add("-n", "nmax", &nmax);
  interface->Add("-v", "verbose", &vl);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least one input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file:"<<InputFile<<endl;
  cout<<"output file: "<<OutputFile<< endl;
  if(SetFile == NULL)
    cerr<<"No settings file! Using standard values"<<endl;
  else
    cout<<"settings file:"<<SetFile<<endl;

  cout << "creating outputfile " << endl;
  TFile* outfile = new TFile(OutputFile,"recreate");
    
  if(outfile->IsZombie()){
    return 4;
  }
  //WASABISettings* set = new WASABISettings(SetFile);
  Calibration* cal = new Calibration(SetFile);
  
  // Create StoreManager both for calibration "TArtCalib..." and treatment "TArtReco..."
  //------------------------------------------------------------------------------------
  TArtStoreManager* sman = TArtStoreManager::Instance();

  // Create EventStore to control the loop and get the EventInfo
  //------------------------------------------------------------
  TArtEventStore* estore = new TArtEventStore();
  estore->SetInterrupt(&signal_received); 
  estore->Open(InputFile);
  std::cout<<"estore ->"<< InputFile <<std::endl;


  // output tree
  TTree* tr = new TTree("tr","Data Tree");
  //branch for original event number
  int eventnumber = 0;
  tr->Branch("eventnumber",&eventnumber,"eventnumber/I");
  //branch for timestamp
  unsigned long long int timestamp = 0;
  tr->Branch("timestamp",&timestamp,"timestamp/l");
  //branch for wasabi RAW, maybe to be removed
  WASABIRaw* wasabiRAW = new WASABIRaw;
  tr->Branch("wasabiRAW",&wasabiRAW,320000);
  //branch for wasabi
  WASABI* wasabi = new WASABI();
  tr->Branch("wasabi",&wasabi,320000);
    
  unsigned long long int last_timestamp = 0;
  int ctr =0;
  while(estore->GetNextEvent() && !signal_received){
    if(vl>0)
      cout << "next event ------------------------------------------------" << endl;
    //clearing
    timestamp = 0;
    eventnumber = 0;
    wasabiRAW->Clear();
    wasabi->Clear();
    
     //timestamp information
    TClonesArray* info_a = (TClonesArray*)sman->FindDataContainer("EventInfo");
    TArtEventInfo* info = (TArtEventInfo*)info_a->At(0);
    timestamp = info->GetTimeStamp();
    eventnumber = info->GetEventNumber();
    if(timestamp<last_timestamp){
      cout << "timestamp was reset, this TS = " << timestamp << ", last one was " << last_timestamp << " difference " << timestamp-last_timestamp << endl;
    }
    if(vl>1)
      cout << eventnumber << "\t" << timestamp << "\t" << timestamp-last_timestamp << endl;
    last_timestamp = timestamp;


    
    //trigger bit information
    TArtRawEventObject* rawevent = (TArtRawEventObject*)sman->FindDataContainer("RawEvent");
    for(int i=0;i<rawevent -> GetNumSeg();i++){
      TArtRawSegmentObject* seg = rawevent -> GetSegment(i);
      Int_t fpl = seg -> GetFP();
      Int_t detector = seg -> GetDetector();
      //cout <<"fpl = " << fpl <<", det = " << detector;
      if(detector == BETAA){
	//ADC data
	for(int j=0; j<seg->GetNumData(); j++){
          TArtRawDataObject* d = seg->GetData(j);
	  int geo = d->GetGeo();
	  int ch  = d->GetCh();
	  int val = d->GetVal();
	  //cout << "geo = " << geo <<", ch = " << ch << ", val = " << val << endl;
	  int adc = geo-5;
	  if(geo==4)
	    adc = 0;
	  if(geo==4 || (geo>5 && geo < 16))
	    wasabiRAW->AddADC(new WASABIRawADC(adc,ch,val));
	}
      }
      else if(detector == BETAT){
	//TDC data
	for(int j=0; j<seg->GetNumData(); j++){
          TArtRawDataObject* d = seg->GetData(j);
	  int geo = d->GetGeo();
	  int ch  = d->GetCh();
	  int val = d->GetVal();
	  //cout << "geo = " << geo <<", ch = " << ch << ", val = " << val << endl;
	  if(geo>24 && geo<28){
	    wasabiRAW->AddTDC(new WASABIRawTDC(geo-25,ch,val));
	  }
	}
      }
    }
    if(vl>1)
      wasabiRAW->Print();

    wasabi = cal->BuildWASABI(wasabiRAW);
    //wasabi->Print();
    
    //fill the tree
    tr->Fill();

    //output
    if(ctr%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << ctr << " events done " << setiosflags(ios::fixed) << setprecision(1) << (Float_t)ctr/(time_end - time_start) << " events/s \r" << flush;
      tr->AutoSave();
    }
    ctr++;

    if(nmax>0 && ctr>nmax-1)
      break;
  }

  double time_end = get_time();
  cout << "Program Run time: " << time_end - time_start << " s." << endl;
  cout << "Total of " << ctr << " events processed, " << ctr/(time_end - time_start) << " events/s." << endl;
  cout << BLUE << tr->GetEntries() << DEFCOLOR << " entries written to tree ("<<BLUE<<tr->GetZipBytes()/(1024*1024)<< DEFCOLOR<<" MB)"<< endl;
  tr->Write("",TObject::kOverwrite);
  outfile->Close();
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
