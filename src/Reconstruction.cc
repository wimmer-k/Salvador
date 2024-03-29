#include "Reconstruction.hh"
using namespace std;

/*!
  Constructor, reads in the settings for the reconstruction
  \param settings the settings file
*/
Reconstruction::Reconstruction(char* settings){
  fset = new Settings(settings);
  fbeta = fset->Beta();

  double pp[3] = {-fset->MINOSlength()/2, 0, fset->MINOSlength()/2};
  double bb[3] = {fset->BetaBefore(), fbeta, fset->BetaAfter()};
  TGraph *gbeta = new TGraph(3,pp,bb);
  if(fset->MINOSlength()>0){
    fminos = new TF1("fminos","pol2",-fset->MINOSlength()/2,fset->MINOSlength()/2);
    gbeta->Fit(fminos,"Rn");
  }
  
  ReadBadChannels(fset->BadChFile());
  ReadPositions(fset->DALIPosFile());
  if(fset->DoReCalibration())
    ReadReCalParams(fset->ReCalFile());
  if(fset->VerboseLevel()>1){
    for(unsigned short i=0;i<fpositions.size();i++){
      cout << i << "\t" << fpositions[i][0]<< "\t" << fpositions[i][1]<< "\t" << fpositions[i][2]<<endl;
    }
    if(fset->DoReCalibration())
      for(unsigned short i=0;i<frecal.size();i++){
	cout << i << "\t" << frecal[i][0]<< "\t" << frecal[i][1]<< "\t" << frecal[i][2]<<endl;
      }
  }
}

/*!
  Reads a list with bad channels
  \param infile the file with the bad channels
*/
void Reconstruction::ReadBadChannels(const char* infile){
  TEnv* bad = new TEnv(infile);
  unsigned short nbad = bad->GetValue("Number.Bad.Channels",0);
  fbad.resize(nbad);
  for(int i=0;i<nbad;i++){
    fbad.at(i) = bad->GetValue(Form("Bad.Channel.%d",i),-1);
  }
}

/*!
  Reads in the parameters for the recalibration
  \param infile the file with the calibration parameters
*/
void Reconstruction::ReadReCalParams(const char* infile){
  TEnv* cal = new TEnv(infile);
  for(int i=0;i<MAXNCRYSTAL;i++){
    vector<double> r;
    r.resize(3);
    r.at(0) = cal->GetValue(Form("DALI.%d.Parameter.0",i),0.0);
    r.at(1) = cal->GetValue(Form("DALI.%d.Parameter.1",i),1.0);
    r.at(2) = cal->GetValue(Form("DALI.%d.Parameter.2",i),0.0);
    frecal.push_back(r);
  }
}

/*!
  Reads in the average positions of first interaction points from the simulation
  \param infile the file with the interaction point averages
*/
void Reconstruction::ReadPositions(const char* infile){
  TEnv* pos = new TEnv(infile);
  for(int i=0;i<MAXNCRYSTAL;i++){
    vector<double> r;
    r.resize(3);
    r.at(0) = pos->GetValue(Form("Average.Position.X.%d",i),0.0);
    r.at(1) = pos->GetValue(Form("Average.Position.Y.%d",i),0.0);
    r.at(2) = pos->GetValue(Form("Average.Position.Z.%d",i),0.0);
    fpositions.push_back(r);
  }
}

/*!
  Sort the hits by their energy, first the one with the lowest energy
  \param hits a vector with the unsorted hits
  \return a vector with the sorted hits
*/
vector<DALIHit*> Reconstruction::Revert(vector<DALIHit*> hits){
  sort(hits.begin(), hits.end(), HitComparer());
  reverse(hits.begin(), hits.end());
  return hits;
}

/*!
  Sort the hits by their energy, first the one with the highest energy
  \param hits a vector with the unsorted hits
  \return a vector with the sorted hits
*/
vector<DALIHit*> Reconstruction::Sort(vector<DALIHit*> hits){
  sort(hits.begin(), hits.end(), HitComparer());
  return hits;
}

/*!
  Recalibrate DALI using second order polynomial
  \param hits a vector with all hits to be recalibrated
*/
void Reconstruction::ReCalibrate(vector<DALIHit*> hits){
  vector<DALIHit*> output;
  for(unsigned short i=0;i<hits.size();i++){
    short id = hits.at(i)->GetID();
    double en = hits.at(i)->GetEnergy();
    en = frecal[id][0] + frecal[id][1]*en + frecal[id][2]*en*en;
    hits.at(i)->SetEnergy(en);
  }
}

/*!
  Filter bad channels, the overflow and underflow hits
  \param hits a vector with all hits
  \return a vector with the filtered hits
*/
vector<DALIHit*> Reconstruction::FilterBadHits(vector<DALIHit*> hits){
  vector<DALIHit*> output;
  for(unsigned short i=0;i<hits.size();i++){
    if(hits.at(i)->GetEnergy()<fset->Overflow() && hits.at(i)->GetEnergy()> fset->Underflow()){
      bool bad = false;
      for(unsigned short j=0;j<fbad.size();j++){
	if(hits.at(i)->GetID()==fbad.at(j))
	  bad = true;
      }
      if(!bad)
	output.push_back(hits.at(i));
    }
  }
  return output;
}

/*!
  Apply the timing cut
  \param hits a vector with all hits
  \return a vector with the gated hits
*/
vector<DALIHit*> Reconstruction::TimingGate(vector<DALIHit*> hits){
  vector<DALIHit*> output;
  for(unsigned short i=0;i<hits.size();i++){
    if(hits.at(i)->GetTOffset()>fset->TimingGate(0) && hits.at(i)->GetTOffset()< fset->TimingGate(1)){
      output.push_back(hits.at(i));
    }
  }
  return output;
}

/*!
  Filter the overflow and underflow hits
  \param hits a vector with all hits
  \return a vector with the filtered hits
*/
vector<DALIHit*> Reconstruction::FilterOverUnderflows(vector<DALIHit*> hits){
  vector<DALIHit*> output;
  for(unsigned short i=0;i<hits.size();i++){
    if(hits.at(i)->GetEnergy()<fset->Overflow() && hits.at(i)->GetEnergy()> fset->Underflow()){
      output.push_back(hits.at(i));
    }
  }
  return output;
}

/*!
  Checks the distance or angle between two hits
  \param hit0 first hit
  \param hit1 second hit
  \return true if the hits are presumably from one gamma
*/
bool Reconstruction::Addback(DALIHit* hit0, DALIHit* hit1){
  //implement more methods
  if(fset->VerboseLevel()>4){
    cout << setw(5) << setprecision(3)<< "distance\t" << hit0->GetPos().DeltaR(hit1->GetPos()) << endl;
    cout << "angle\t" << hit0->GetPos().Angle(hit1->GetPos()) << endl;
  }
  double tdiff = hit0->GetTOffset()-hit1->GetTOffset();
  if(tdiff < fset->AddbackTimeDiff(0) || tdiff > fset->AddbackTimeDiff(1))
    return false;

  if(fset->AddbackType()==1){
    if(hit0->GetPos().DeltaR(hit1->GetPos())<fset->AddbackDistance())
      return true;
  }
  else if(fset->AddbackType()==2){
    if(hit0->GetPos().Angle(hit1->GetPos())<fset->AddbackAngle()*TMath::Pi()/180.)
      return true;
  }
  return false;
}

/*!
  Addback
  \param hits a vector with the energy sorted hits
  \return a vector with the addback hits
*/
vector<DALIHit*> Reconstruction::Addback(vector<DALIHit*> hits){
  vector<DALIHit*> hitsAB;
  vector<DALIHit*> unusedhits;
  for(unsigned short i=0;i<hits.size();i++){
    short id = hits.at(i)->GetID();
    double en = hits.at(i)->GetEnergy();
    double DCen = hits.at(i)->GetDCEnergy();
    TVector3 pos = hits.at(i)->GetPos();
    double time = hits.at(i)->GetTime();
    double toffset = hits.at(i)->GetTOffset();
    unsigned short hitsadded = hits.at(i)->GetHitsAdded();
    unusedhits.push_back(new DALIHit(id, en, DCen, pos, time, toffset, hitsadded));
  }
  unusedhits = Revert(unusedhits);

  while(unusedhits.size() > 0){
    //first one is automatically good
    vector<DALIHit*> currenthits; //current hits contains all hits that have already been added to thishit, to still know their positions
    currenthits.push_back(unusedhits.back());
    DALIHit* thishit = unusedhits.back();
    unusedhits.pop_back();
    
    // how many hits added
    int added = 0;
    do{
      added =0;
      int addme =-1;
      bool found = false;
      for(unsigned short k=0;k<currenthits.size();k++){
	for(unsigned short j=0;j<unusedhits.size();j++){
	  if(unusedhits.at(j)->GetEnergy()<fset->AddbackThresh())
	    continue;
	  if(Addback(currenthits.at(k),unusedhits.at(j))){	    
	    addme = j;
	    found = true;
	    break;
	  }
	}//loop unused hits
	if(found)
	  break;
      }//loop add back hits
      if(found && addme >-1){
	added++;
	currenthits.push_back(unusedhits.at(addme));
	thishit->AddBackHit(unusedhits.at(addme));
	unusedhits.erase(unusedhits.begin() + addme);
      }
      if(hits.size()==0)
	break;
    }while(added>0);//do
    //done with addback for thishit
    if(fset->VerboseLevel()>2){
      cout << "current hits" << endl;
      for(unsigned short k=0;k<currenthits.size();k++){
	currenthits.at(k)->Print();
      }
      cout << "unused hits" << endl;
      for(unsigned short k=0;k<unusedhits.size();k++){
	unusedhits.at(k)->Print();
      }
      cout << "this hit" <<endl;
      thishit->Print();
    }
    //add the addbacked hit into the vector
    hitsAB.push_back(thishit);
  }//while
  
  if(fset->VerboseLevel()>2){
    cout << "after addback " << endl;
    for(unsigned short k=0;k<hitsAB.size();k++){
      hitsAB.at(k)->Print();
    }
  }

  return hitsAB;
}

/*!
  Sets the positions of the DALIHits to the average positions determined from the simulation
  \param dali the input DALI object
*/
void Reconstruction::SetPositions(DALI* dali){
  for(unsigned short g=0;g<dali->GetMult();g++){
    DALIHit* hit = dali->GetHit(g);
    short id = hit->GetID();
    if(id<0||id>MAXNCRYSTAL){
      cout << "invalid ID in DALI: " << id <<endl;
      hit->Print();
      return;
    } 
    if(fset->VerboseLevel()>3)
      cout << id << "\t" << fpositions[id][0] << "\t" << fpositions[id][1] << "\t" << fpositions[id][2] << endl;
    hit->SetPos(fpositions[id][0],fpositions[id][1],fpositions[id][2]);
  }
}

/*!
  Do the Doppler correction
  \param dali the input DALI object
*/
void Reconstruction::DopplerCorrect(DALI* dali){
  dali->DopplerCorrect(fbeta);
}

/*!
  Do the Doppler correction including the target positons
  \param dali the input DALI object
  \param zreac the reaction point in the target
  \return event by events beta
*/
double Reconstruction::DopplerCorrect(DALI* dali, double zreac){
  double beta = fminos->Eval(zreac);
  //cout << fbeta <<"\t" << beta << "\t" << zreac << endl;
  dali->DopplerCorrect(beta,zreac);
  return beta;
}

/*!
  Calculate the ppac position as the average of A and B PPACs
  \param pina A PPAC
  \param pinb B PPAC
  \return vector to the position
*/
TVector3 Reconstruction::PPACPosition(SinglePPAC* pina, SinglePPAC* pinb){
  double x = sqrt(-1.);
  double y = sqrt(-1.);
  double z = sqrt(-1.);
  if(pina->Fired() && pinb->Fired()){
    x = (pina->GetX()+pinb->GetX())/2;
    y = (pina->GetY()+pinb->GetY())/2;
    z = (pina->GetZ()+pinb->GetZ())/2;
  }
  else if(pina->Fired() && !pinb->Fired()){
    x = pina->GetX();
    y = pina->GetY();
    z = pina->GetZ();
  }
  else if(!pina->Fired() && pinb->Fired()){
    x = pinb->GetX();
    y = pinb->GetY();
    z = pinb->GetZ();
  }
  return TVector3(x,y,z);
}
/*!
  Align the F8 PPAC3 with respect to the others, calibration parameters from the empty target runs
  \param pin0 uncalibrated PPAC 0 position
  \param pin1 uncalibrated PPAC 1 position
*/
void Reconstruction::AlignPPAC(SinglePPAC* pin0, SinglePPAC* pin1){
  double x = pin0->GetX() + fset->PPAC3PositionX0();
  double y = pin0->GetY() + fset->PPAC3PositionY0();
  pin0->SetXY(x,y);
  x = pin1->GetX() + fset->PPAC3PositionX1();
  y = pin1->GetY() + fset->PPAC3PositionY1();
  pin1->SetXY(x,y);
}
/*!
  Calculate the ppac position as the average of A and B PPACs
  \param inc incoming beam
  \param ppac position from which to extrapolate  
  \return vector to the target position
*/
TVector3 Reconstruction::TargetPosition(TVector3 inc, TVector3 ppac){
  double a = inc.X()/inc.Z();
  double b = inc.Y()/inc.Z();
  
  double x = ppac.X() + a * (fset->TargetPosition()-ppac.Z());
  double y = ppac.Y() + b * (fset->TargetPosition()-ppac.Z());
  
  return TVector3(x,y,fset->TargetPosition());
}

/*!
  Gate on the F5X position
  \return is inside the gate
*/
bool Reconstruction::F5XGate(double f5xpos){
  if(f5xpos < fset->F5XGate(0) || f5xpos > fset->F5XGate(1))
      return false;
  return true;
}

/*!
  Gate on changing charge states in BigRIPS
  \return changes in charge state
*/
bool Reconstruction::ChargeChangeBR(double delta0, double delta1){
  if(fset->DeltaGate(0)<-998)//no gate set
    return false;
  if((delta0-delta1) < fset->DeltaGate(0) || (delta0-delta1) > fset->DeltaGate(1))
      return true;
  return false;
}
/*!
  Gate on changing charge states in ZeroDegree
  \return changes in charge state
*/
bool Reconstruction::ChargeChangeZD(double delta2, double delta3){
  if(fset->DeltaGate(2)<-998)//no gate set
    return false;
  if((delta2-delta3) < fset->DeltaGate(2) || (delta2-delta3) > fset->DeltaGate(3))
      return true;
  return false;
}
