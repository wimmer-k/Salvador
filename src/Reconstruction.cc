#include "Reconstruction.hh"
using namespace std;

/*!
  Constructor, reads in the settings for the reconstruction
  \param settings the settings file
*/
Reconstruction::Reconstruction(char *settings){
  TEnv *set = new TEnv(settings);
  fverbose = set->GetValue("VerboseLevel",0);
  fbeta = set->GetValue("AverageBeta",0.5);
  foverflow = set->GetValue("Overflow.Threshold",8000);
  funderflow = set->GetValue("Underflow.Threshold",0);
  faddbacktype = set->GetValue("Addback.Type",1);
  faddbackdistance = set->GetValue("Addback.Distance",20.);
  faddbackangle = set->GetValue("Addback.Angle",20.);
  faddbackthreshold = set->GetValue("Addback.Threshold",200.);
  faddbacktdiff[0] = set->GetValue("Addback.TimeDiff.Low",-50.);
  faddbacktdiff[1] = set->GetValue("Addback.TimeDiff.High",20.);

  string posfile = set->GetValue("InteractionPoints",(char*)"settings/iponts.dat");
  if(fverbose>0){
    cout << "fverbose " << fverbose << endl;
    cout << "fbeta " << fbeta << endl;
    cout << "faddbacktype " << faddbacktype << endl;
    cout << "faddbackdistance " << faddbackdistance << endl;
    cout << "faddbackangle " << faddbackangle << endl;
    cout << "positions " << posfile << endl;
  }
  ReadPositions((const char*)posfile.c_str());
  if(fverbose>1){
    for(unsigned short i=0;i<fpositions.size();i++){
      cout << i << "\t" << fpositions[i][0]<< "\t" << fpositions[i][1]<< "\t" << fpositions[i][2]<<endl;
    }
  }
}
/*!
  Reads in the average positions of first interaction points from the simulation
  \param infile the file with the interaction point averages
*/
void Reconstruction::ReadPositions(const char *infile){
  TEnv *pos = new TEnv(infile);
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
  Filter the overflow and underflow hits
  \param hits a vector with all hits
  \return a vector with the filtered hits
*/
vector<DALIHit*> Reconstruction::FilterOverUnderflows(vector<DALIHit*> hits){
  vector<DALIHit*> output;
  for(unsigned short i=0;i<hits.size();i++){
    if(hits[i]->GetEnergy()<foverflow && hits[i]->GetEnergy()> funderflow){
      output.push_back(hits[i]);
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
  if(fverbose>4){
    cout << setw(5) << setprecision(3)<< "distance\t" << hit0->GetPos().DeltaR(hit1->GetPos()) << endl;
    cout << "angle\t" << hit0->GetPos().Angle(hit1->GetPos()) << endl;
  }
  double tdiff = hit0->GetTOffset()-hit1->GetTOffset();
  if(tdiff < faddbacktdiff[0] || tdiff > faddbacktdiff[1])
    return false;

  if(faddbacktype==1){
    if(hit0->GetPos().DeltaR(hit1->GetPos())<faddbackdistance)
      return true;
  }
  else if(faddbacktype==2){
    if(hit0->GetPos().Angle(hit1->GetPos())<faddbackangle*TMath::Pi()/180.)
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
    short id = hits[i]->GetID();
    double en = hits[i]->GetEnergy();
    double DCen = hits[i]->GetDCEnergy();
    TVector3 pos = hits[i]->GetPos();
    double time = hits[i]->GetTime();
    double toffset = hits[i]->GetTOffset();
    unsigned short hitsadded = hits[i]->GetHitsAdded();
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
	  if(unusedhits[j]->GetEnergy()<faddbackthreshold)
	    continue;
	  if(Addback(currenthits[k],unusedhits[j])){	    
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
	currenthits.push_back(unusedhits[addme]);
	thishit->AddBackHit(unusedhits[addme]);
	unusedhits.erase(unusedhits.begin() + addme);
      }
      if(hits.size()==0)
	break;
    }while(added>0);//do
    //done with addback for thishit
    if(fverbose>2){
      cout << "current hits" << endl;
      for(unsigned short k=0;k<currenthits.size();k++){
	currenthits[k]->Print();
      }
      cout << "unused hits" << endl;
      for(unsigned short k=0;k<unusedhits.size();k++){
	unusedhits[k]->Print();
      }
      cout << "this hit" <<endl;
      thishit->Print();
    }
    //add the addbacked hit into the vector
    hitsAB.push_back(thishit);
  }//while
  
  if(fverbose>2){
    cout << "after addback " << endl;
    for(unsigned short k=0;k<hitsAB.size();k++){
      hitsAB[k]->Print();
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
    DALIHit * hit = dali->GetHit(g);
    short id = hit->GetID();
    if(id<0||id>MAXNCRYSTAL){
      cout << "invalid ID in DALI" <<endl;
      hit->Print();
      return;
    } 
    if(fverbose>3)
      cout << id << "\t" << fpositions[id][0] << "\t" << fpositions[id][1] << "\t" << fpositions[id][2] << endl;
    hit->SetPos(fpositions[id][0],fpositions[id][1],fpositions[id][2]);
  }
}
/*
  Do the Doppler correction
  \param dali the input DALI object
*/
void Reconstruction::DopplerCorrect(DALI* dali){
  dali->DopplerCorrect(fbeta);
}

// /*!

//  */
// void Reconstruction::ReadAddBackTable(){
  
// }
