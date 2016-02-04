#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include <signal.h>
#include "TArtStoreManager.hh"
#include "TArtEventStore.hh"
#include "TArtBigRIPSParameters.hh"
#include "TArtDALIParameters.hh"
#include "TArtCalibPID.hh"
#include "TArtCalibDALI.hh"
#include "TArtCalibPPAC.hh"
#include "TArtCalibPlastic.hh"
#include "TArtCalibIC.hh"
#include "TArtCalibFocalPlane.hh"
#include "TArtEventInfo.hh"
#include "TArtPlastic.hh"
#include "TArtIC.hh"
#include "TArtPPAC.hh"
#include "TArtRecoPID.hh"
#include "TArtRecoRIPS.hh"
#include "TArtRecoTOF.hh"
#include "TArtRecoBeam.hh"
#include "TArtFocalPlane.hh"
#include "TArtTOF.hh"
#include "TArtRIPS.hh"
#include "TArtBeam.hh"
#include "TArtDALINaI.hh"

#include "TFile.h"
#include "TTree.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "FocalPlane.hh"
#include "Beam.hh"
using namespace TMath;
using namespace std;
int fpID[6] = {3,5,7,8,9,11};
int fpNr(int id);
bool signal_received = false;
void signalhandler(int sig);
double get_time();
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  char* InputFile;
  char* OutputFile = NULL;
  char* CutFile = NULL;
  int nmax =0;
  int vl =0;

  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "inputfile", &InputFile);
  interface->Add("-o", "outputfile", &OutputFile);
  interface->Add("-n", "nmax", &nmax);
  interface->Add("-v", "verbose", &vl);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least one input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file:"<<InputFile<<endl;
  cout<<"output file: "<<OutputFile<< endl;

  cout << "creating outputfile " << endl;
  TFile* outfile = new TFile(OutputFile,"recreate");
    
  if(outfile->IsZombie()){
    return 4;
  }

  // Create StoreManager both for calibration "TArtCalib..." and treatment "TArtReco..."
  //------------------------------------------------------------------------------------
  TArtStoreManager * sman = TArtStoreManager::Instance();

  // Create EventStore to control the loop and get the EventInfo
  //------------------------------------------------------------
  TArtEventStore *estore = new TArtEventStore();
  estore->SetInterrupt(&signal_received); 
  estore->Open(InputFile);
  std::cout<<"estore ->"<< InputFile <<std::endl;

  // Create BigRIPSParameters to get Plastics, PPACs, ICs and FocalPlanes parameters from ".xml" files
  //--------------------------------------------------------------------------------------------------	
  TArtBigRIPSParameters *para = TArtBigRIPSParameters::Instance();
  para->LoadParameter("/home/wimmer/ribf94/db/BigRIPSPPAC.xml");
  para->LoadParameter("/home/wimmer/ribf94/db/BigRIPSPlastic.xml");
  para->LoadParameter("/home/wimmer/ribf94/db/BigRIPSIC.xml");
  para->LoadParameter("/home/wimmer/ribf94/db/FocalPlane.xml");
  para->SetFocusPosOffset(8,138.5);

  // Create CalibPID to get and calibrate raw data ( CalibPID -> 
  //[CalibPPAC , CalibIC, CalibPlastic , CalibFocalPlane] 
  TArtCalibPID *brcalib          = new TArtCalibPID();
  TArtCalibPPAC *ppaccalib       = brcalib->GetCalibPPAC();
  TArtCalibPlastic *plasticcalib = brcalib->GetCalibPlastic();
  TArtCalibIC *iccalib           = brcalib->GetCalibIC(); 
  TArtCalibFocalPlane *cfpl      = brcalib->GetCalibFocalPlane();

  // Create RecoPID to get calibrated data and to reconstruct TOF, AoQ, Z, ... (RecoPID -> 
  //[ RecoTOF , RecoRIPS , RecoBeam] )
  TArtRecoPID *recopid = new TArtRecoPID();
 
  //para->PrintListOfPPACPara();

  // Definition of observables we want to reconstruct
  std::cout << "Defining bigrips parameters" << std::endl;

  TArtRIPS *recorips[4];
  recorips[0] = recopid->DefineNewRIPS(3,5,"/home/wimmer/ribf94/matrix/mat1.mat","D3"); // F3 - F5
  recorips[1] = recopid->DefineNewRIPS(5,7,"/home/wimmer/ribf94/matrix/mat2.mat","D5"); // F5 - F7
  recorips[2] = recopid->DefineNewRIPS(8,9,"/home/wimmer/ribf94/matrix/F8F9_LargeAccAchr.mat","D7"); // F8 - F9
  recorips[3] = recopid->DefineNewRIPS(9,11,"/home/wimmer/ribf94/matrix/F9F11_LargeAccAchr.mat","D8"); // F9 - F11  

  // Reconstruction of TOF DefineNewTOF(fisrt plane, second plane, time offset)
  TArtTOF *recotof[6];
  recotof[0] = recopid->DefineNewTOF("F3pl","F7pl",297.211,5); // F3 - F5
  recotof[1] = recopid->DefineNewTOF("F3pl","F7pl",314.727,5); // F5 - F7
  recotof[2] = recopid->DefineNewTOF("F3pl","F7pl",306.04,5); // F3 - F7

  recotof[3] = recopid->DefineNewTOF("F8pl","F11pl-1",-162.302,9); // F8 - F9
  recotof[4] = recopid->DefineNewTOF("F8pl","F11pl-1",-161.356,9); // F9 - F11
  recotof[5] = recopid->DefineNewTOF("F8pl","F11pl-1",-161.712,9); // F8 - F11
  
  TArtTOF *tof7to8  = recopid->DefineNewTOF("F7pl","F8pl"); // F7 - F8

  // Reconstruction of IC observables for ID
  TArtBeam *recobeam[6];
  recobeam[0] = recopid->DefineNewBeam(recorips[0],recotof[0],"F7IC");
  recobeam[1] = recopid->DefineNewBeam(recorips[1],recotof[1],"F7IC");   
  recobeam[2] = recopid->DefineNewBeam(recorips[0],recorips[1],recotof[2],"F7IC");   

  recobeam[3] = recopid->DefineNewBeam(recorips[2],recotof[3],"F11IC");
  recobeam[4] = recopid->DefineNewBeam(recorips[3],recotof[4],"F11IC");
  recobeam[5] = recopid->DefineNewBeam(recorips[2],recorips[3],recotof[5],"F11IC");

  // Create DALIParameters to get ".xml"
  TArtDALIParameters *dpara = TArtDALIParameters::Instance();
  dpara->LoadParameter("/home/wimmer/ribf94/db/DALI.xml");
  
  // Create CalibDALI to get and calibrate raw data
  TArtCalibDALI *dalicalib = new TArtCalibDALI();

  std::cout<<"para"<<std::endl;
  TTree *tr = new TTree("tr","Data Tree");

  int trigbit = 0;
  tr->Branch("trigbit",&trigbit,"trigbit/I");
  
  FocalPlane *fp[NFPLANES];
  for(int f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->Branch(Form("fp%d",fpID[f]),&fp[f],320000);
  }
  Beam *beam = new Beam;
  tr->Branch("beam",&beam,320000);

  int ctr =0;
  while(estore->GetNextEvent()){
    //clearing
    trigbit = 0;
    for(int f=0;f<NFPLANES;f++){
      fp[f]->Clear();
    }
    beam->Clear();

    //Making the BigRIPS tree calibration
    brcalib->ClearData();
    brcalib->ReconstructData();
    
    //Reconstructiong the PID
    recopid->ClearData();
    recopid->ReconstructData();


    //trigger bit information
    TArtRawEventObject *rawevent = (TArtRawEventObject *)sman->FindDataContainer("RawEvent");
    for(int i=0;i<rawevent -> GetNumSeg();i++){
      TArtRawSegmentObject *seg = rawevent -> GetSegment(i);
      Int_t fpl = seg -> GetFP();
      Int_t detector = seg -> GetDetector();
      if(fpl==63 && detector==10){
        for(int j=0; j < seg -> GetNumData(); j++){
          TArtRawDataObject *d = seg -> GetData(j);
          trigbit = d -> GetVal();
        }
      }
    }

    //focal plane detector information
    TArtFocalPlane *tfpl;
    TArtPlastic *tpla;
    TArtIC *tic;
    TVectorD *vec;
    PPAC ppac;
    Plastic plastic;
    MUSIC music;
    for(unsigned short f=0;f<NFPLANES;f++){
      ppac.Clear();
      tfpl = cfpl->FindFocalPlane(fpID[f]);
      if(tfpl){
	vec=tfpl->GetOptVector(); 
	ppac.Set((*vec)(0), (*vec)(2), (*vec)(1), (*vec)(3));
      }

      plastic.Clear();
      tpla = plasticcalib->FindPlastic(Form("F%dpl",fpID[f]));
      if(fpID[f]==11)
	tpla = plasticcalib->FindPlastic(Form("F%dpl-1",fpID[f]));
      if(tpla){
	plastic.SetTime(tpla->GetTimeL(), tpla->GetTimeR());
	plastic.SetCharge(tpla->GetQLRaw(), tpla->GetQRRaw());
      }
      
      music.Clear();
      tic = iccalib->FindIC(Form("F%dIC",fpID[f]));
      if(tic){
	music.SetNHits(tic->GetNumHit());
	music.SetEnergy(tic->GetEnergyAvSum(),tic->GetEnergySqSum());
      }

      fp[f]->SetPPAC(ppac);
      fp[f]->SetPlastic(plastic);
      fp[f]->SetMUSIC(music);
    }
    
    //beam parameters and PID

    beam->SetTOFBeta(0,recotof[2]->GetTOF(),recotof[2]->GetBeta());
    beam->SetTOFBeta(1,recotof[5]->GetTOF(),recotof[5]->GetBeta());
    beam->SetTOFBeta(2,tof7to8->GetTOF(),tof7to8->GetBeta());
    
    for(unsigned short b=0;b<6;b++)
      beam->SetAQZ(b,recobeam[b]->GetAoQ(),recobeam[b]->GetZet());

    beam->CorrectAQ(1, +0.00034002 *fp[fpNr(5)]->GetPPAC()->GetA()
		       -6.089e-05  *fp[fpNr(5)]->GetPPAC()->GetX()
		       +0.000413889*fp[fpNr(7)]->GetPPAC()->GetA() 
                       +0.000460512*fp[fpNr(7)]->GetPPAC()->GetX());
    
    beam->CorrectAQ(5, +8.53643e-05*fp[fpNr(5)]->GetPPAC()->GetA()
		       -6.57149e-05*fp[fpNr(5)]->GetPPAC()->GetX()
		       +0.000158604*fp[fpNr(7)]->GetPPAC()->GetA() 
                       +0.000212333*fp[fpNr(7)]->GetPPAC()->GetX()
                       -9.46977e-05*fp[fpNr(9)]->GetPPAC()->GetA()
		       +1.46503e-06*fp[fpNr(9)]->GetPPAC()->GetX()
		       -2.54913e-06*fp[fpNr(11)]->GetPPAC()->GetA() 
                       -0.00010038 *fp[fpNr(11)]->GetPPAC()->GetX());
    for(unsigned short b=0;b<4;b++)
      beam->SetDelta(b ,recorips[b]->GetDelta());

    //dali
    dalicalib->ClearData();
    dalicalib->SetPlTime(beam->GetTOF(2));
    //dalicalib->SetVertex(z_vertex-3.9);
    dalicalib->SetVertex(0.0);
    //Add above to remove F8plastic tof.
    dalicalib->ReconstructData();

    cout << "dalicalib->GetNumNaI() " << dalicalib->GetNumNaI() << endl;
    for(unsigned short g=0; g<dalicalib->GetNumNaI()-2; g++){//last two are junk?
      TArtDALINaI* hit = (TArtDALINaI*)dalicalib->GetNaIArray()->At(g);
      cout << "hit("<<g<<")->GetEnergy() " <<hit->GetEnergy() << endl;
    }

    //fill the tree
    tr->Fill();

    //output
    if(ctr%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (Float_t)ctr/(time_end - time_start) << " events/s \r" << flush;
    }
    ctr++;

    if(ctr>nmax)
      break;
  }
  tr->Write("",TObject::kOverwrite);
  outfile->Close();

  double time_end = get_time();
  cout << "Run time " << time_end - time_start << " s." << endl;
 
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
int fpNr(int id){
  if(id<3 || id>11)
    return -1;
  switch (id){
    case 3:
      return 0;
    case 5:
      return 1;
    case 7:
      return 2;
    case 8:
      return 3;
    case 9:
      return 4;
    case 11:
      return 5;
    default:
      return -1;
  }
}
