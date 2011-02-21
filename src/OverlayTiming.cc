#include "OverlayTiming.h"
#include <EVENT/LCCollection.h>
#include <EVENT/SimTrackerHit.h>
#include <EVENT/MCParticle.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

#include <IMPL/SimCalorimeterHitImpl.h>
#include <IMPL/SimTrackerHitImpl.h>
#include <IMPL/LCFlagImpl.h>
#include <IMPL/MCParticleImpl.h>
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include <EVENT/ReconstructedParticle.h>

#include <algorithm>


// ----- include for verbosity dependend logging ---------

using namespace lcio ;
using namespace marlin ;


OverlayTiming aOverlayTiming ;

OverlayTiming::OverlayTiming() : Processor("OverlayTiming") {
  
  // modify processor description
  _description = "Processeor to overlay events from the background taking the timing of the subdetectors into account" ;

 StringVec files ;


  registerProcessorParameter("Delta_t",
			     "Time difference between bunches in the bunch train in ns"  ,
			     _T_diff,
			     float(0.5)) ;

  registerProcessorParameter("NBunchtrain",
			     "Number of bunches in a bunch train"  ,
			     _nBunchTrain,
			     int(1));

  registerProcessorParameter( "BackgroundFileNames" , 
			      "Name of the lcio input file(s) with background - assume one file per bunch crossing."  ,
			      _inputFileNames ,
			      files) ;

  registerProcessorParameter("PhysicsBX",
			     "Number of the Bunch crossing of the physics event"  ,
			     _BX_phys,
			     int(1)) ;

  registerProcessorParameter( "TPCDriftvelocity" , 
			      "[mm/ns] (float) - default 5.0e-2 (5cm/us)" ,
			      _tpcVdrift_mm_ns ,
			      float(5.0e-2) ) ;

  registerProcessorParameter( "RandomBx" , 
			      "Place the physics event at an random position in the train -- overrides PhysicsBX" ,
			      _randomBX ,
			      bool(false) ) ;

  registerProcessorParameter( "NumberBackground" , 
			      "Number of Background events to overlay - either fixed or Poisson mean" ,
			      _NOverlay,
			      float(1) ) ;

  registerProcessorParameter( "Poisson_random_NOverlay" , 
			      "Draw random number of Events to overlay from Poisson distribution with  mean value NumberBackground" ,
			      _Poisson ,
			      bool(false) ) ;

  registerProcessorParameter( "RandomSeed" , 
			      "random seed - default 42" ,
			      _ranSeed ,
			      int(42) ) ;


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Collections with Integration Times


  registerProcessorParameter("BeamCalCollection_IntegrationTime",
			     "Integration time for the BeamCalCollection"  ,
			     _BeamCal_int,
			     float(10)) ;

  registerProcessorParameter("ETDCollection_Integration_Time",
			     "Integration time for the ETDCollection"  ,
			     _ETD_int,
			     float(10)) ;

  registerProcessorParameter("EcalBarrelCollection_Integration_Time",
			     "Integration time for the EcalBarrelCollection"  ,
			     _EcalBarrel_int,
			     float(10)) ;

  registerProcessorParameter("EcalBarrelPreShowerCollection_Integration_Time",
			     "Integration time for the EcalBarrelPreShowerCollection"  ,
			     _EcalBarrelPreShower_int,
			     float(10)) ;
  
  registerProcessorParameter("EcalEndcapCollection_Integration_Time",
			    "Integration time for the EcalEndcapCollection"  ,
			    _EcalEndcap_int,
			    float(10)) ;
 
  registerProcessorParameter("EcalEndcapPreShowerCollection_Integration_Time",
			     "Integration time for the EcalEndcapPreShowerCollection"  ,
			     _EcalEndcapPreShower_int,
			     float(10)) ;

  registerProcessorParameter("EcalEndcapRingCollection_Integration_Time",
			     "Integration time for the EcalEndcapRingCollection"  ,
			     _EcalEndcapRing_int,
			     float(10)) ;

  registerProcessorParameter("EcalEndcapRingPreShowerCollection_Integration_Time",
			     "Integration time for the  EcalEndcapRingPreShowerCollection"  ,
			     _EcalEndcapRingPreShower_int,
			     float(10)) ;

  registerProcessorParameter("FTDCollection_Integration_Time",
			     "Integration time for the FTDCollection"  ,
			     _FTD_int,
			     float(10)) ;

  registerProcessorParameter("HcalBarrelRegCollection_Integration_Time",
			     "Integration time for the HcalBarrelRegCollection"  ,
			     _HcalBarrelReg_int,
			     float(10)) ;

  registerProcessorParameter("HcalEndCapRingsCollection_Integration_Time",
			     "Integration time for the HcalEndCapRingsCollection"  ,
			     _HcalEndCapRings_int,
			     float(10)) ;

  registerProcessorParameter("HcalEndCapsCollection_Integration_Time",
			     "Integration time for the HcalEndCapsCollection"  ,
			     _HcalEndCaps_int,
			     float(10)) ;

  registerProcessorParameter("LHcalCollection_Integration_Time",
			     "Integration time for the LHcalCollection"  ,
			     _LHcal_int,
			     float(10)) ;
  
  registerProcessorParameter("LumiCalCollection_Integration_Time",
			     "Integration time for the LumiCalCollection"  ,
			     _LumiCal_int,
			     float(10)) ;
 
  registerProcessorParameter("MuonBarrelCollection_Integration_Time",
			     "Integration time for the MuonBarrelCollection"  ,
			     _MuonBarrel_int,
			     float(10)) ;

  registerProcessorParameter("MuonEndCapCollection_Integration_Time",
			     "Integration time for the MuonEndCapCollection"  ,
			     _MuonEndCap_int,
			     float(10)) ;

  registerProcessorParameter("SETCollection_Integration_Time",
			     "Integration time for the SETCollection"  ,
			     _SET_int,
			     float(10)) ;

  registerProcessorParameter("SITCollection_Integration_Time",
			     "Integration time for the SITCollection"  ,
			     _SIT_int,
			     float(10)) ;
  
  registerProcessorParameter("VXDCollection_Integration_Time",
			     "Integration time for the VXDCollection"  ,
			     _VXD_int,
			     float(10)) ;

 registerProcessorParameter("TPCCollection_Integration_Time",
			     "Integration time for the TPCCollection"  ,
			     _TPC_int,
			     float(10)) ;

 registerProcessorParameter("TPCSpacePointCollection_Integration_Time",
			     "Integration time for the TPCSpacePointCollection"  ,
			     _TPCSpacePoint_int,
			     float(10)) ;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void OverlayTiming::init() { 
  streamlog_out(DEBUG) << " init called  " << std::endl ;
  printParameters() ;
 
  overlay_Eventfile_reader = LCFactory::getInstance()->createLCReader() ;
  
  streamlog_out(WARNING) << "Attention! There are " << _inputFileNames.size() 
			 << " files in the list of background files to overlay. Make sure that the total number of background events is sufficiently large for your needs!!" 
			 << std::endl;

  CLHEP::HepRandom::setTheSeed( _ranSeed ) ;

  _nRun = 0 ;
  _nEvt = 0 ;
}


void OverlayTiming::processRunHeader( LCRunHeader* run) { 
  _nRun++ ; 
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OverlayTiming::modifyEvent( LCEvent * evt ) 
{ 
  //------------------------------------------------------------------------------------------------------------------------------------

  if (_randomBX) 
    {
      _BX_phys =  int(CLHEP::RandFlat::shoot(_nBunchTrain));
      streamlog_out(DEBUG) << "Physics Event was places in the " << _BX_phys << " bunch crossing!" << std::endl;
    }

  //define a permutation for the events to overlay -- the physics event is [er definition at position 0
  std::vector <int>* permutation = new  std::vector <int>;
  for (int i= -(_BX_phys - 1); i<_nBunchTrain-(_BX_phys-1) ; ++i) permutation->push_back(i);
  random_shuffle(permutation->begin(), permutation->end()) ;
  
  int random_file = int(CLHEP::RandFlat::shoot(_inputFileNames.size()-1));
  //Make sure we have filenames to open and that we really want to overlay something
  if(random_file > -1 && _NOverlay > 0.0){
    overlay_Eventfile_reader -> open (_inputFileNames.at(random_file));
    streamlog_out(DEBUG) << "Open background file: " << _inputFileNames.at(random_file) << std::endl;
  }

  //streamlog_out(DEBUG) << "Permutated order of the events: " << std::endl;
  //calculate Time Windows of the different subdetectors
  
  //------------------------------------------------------------------------------------------------------------------------------------

  //We have the physics event in evt. Now we merge the new overlay events with it.
  //A Pointer is needed to acces the events in the overly files
  LCEvent * overlay_Evt;
  int  BX_number_in_train;

  //some pointers to acces the collections in the overlay and physics event
  LCCollection*    Collection_in_overlay_Evt = 0;
  LCCollection*    Collection_in_Physics_Evt = 0;
  std::string Collection_name = "";



  //first cut the collections in the physics event to the defined time windows
  const std::vector<std::string > * collection_names_in_Evt; 
  collection_names_in_Evt = evt -> getCollectionNames ();
  
  for (unsigned int j=0; j < collection_names_in_Evt->size(); ++j)
    {
      Collection_name = collection_names_in_Evt -> at(j);
      Collection_in_Physics_Evt =  evt -> getCollection( Collection_name );
      if (  Collection_in_Physics_Evt ->getTypeName () == (LCIO::SIMCALORIMETERHIT) ||   Collection_in_Physics_Evt ->getTypeName () == (LCIO::SIMTRACKERHIT) )
	{
	  define_time_windows( Collection_name );
	  crop_collection ( Collection_in_Physics_Evt);
	}
    }
  

  if(_inputFileNames.size() > 0 && _NOverlay > 0.0){
  //Now overlay the background evnts to each bunchcrossing in the bunch train
  for(int i=0 ; i < _nBunchTrain ; ++i ) 
    {   

      BX_number_in_train = permutation -> at(i);
      int NOverlay_to_this_BX;
      
	if (_Poisson) 
	  {
	    NOverlay_to_this_BX = int(CLHEP::RandPoisson::shoot(_NOverlay));
	  }
	else
	  {
	    NOverlay_to_this_BX = int(_NOverlay);
	  }
	
	streamlog_out(DEBUG) << "Will overlay " << NOverlay_to_this_BX << " events to BX number " << BX_number_in_train+_BX_phys << std::endl;
     
      
      for (int k =0 ; k<NOverlay_to_this_BX ; ++k)
	{

	  overlay_Evt = overlay_Eventfile_reader -> readNextEvent(LCIO::UPDATE);
	  
	  //if there are no events left in the actual file, open the next one.
	  if ( overlay_Evt == 0)
	    {
	      overlay_Eventfile_reader -> close();
	      random_file = int(CLHEP::RandFlat::shoot(_inputFileNames.size()-1));
	      overlay_Eventfile_reader -> open (_inputFileNames.at(random_file));
	      overlay_Evt = overlay_Eventfile_reader -> readNextEvent(LCIO::UPDATE);
	      streamlog_out(DEBUG) << "Open background file: " << _inputFileNames.at(random_file) << std::endl;
	    }

	  // the overaly_Event is now open, start to merge its collections with the ones of the accumulated overlay events collections
	  // all the preperatoty work has been done now....
	  // first, let's see which collections are in the event
	  
	  
	  //first include the MCParticles into the physics event
	  merge_collections (overlay_Evt -> getCollection( "MCParticle" ) ,evt -> getCollection( "MCParticle" ) ,   0 ); 
	  
	  collection_names_in_Evt =  overlay_Evt -> getCollectionNames ();
	  
	  for (unsigned int j=0; j < collection_names_in_Evt->size(); ++j)
	    {
	      
	      Collection_name = collection_names_in_Evt -> at(j);
	      Collection_in_overlay_Evt =  overlay_Evt -> getCollection( Collection_name );

	      define_time_windows( Collection_name );
	    
	      //the event can only make contributions to the readout, if the bx does not happen after the integration time stopped.
	      //And we are only interested in Calorimeter or Trackerhits.
  
	      if ((this_stop > (BX_number_in_train - _BX_phys)*_T_diff ) &&
		  (Collection_in_overlay_Evt ->getTypeName () == (LCIO::SIMCALORIMETERHIT) ||   Collection_in_overlay_Evt ->getTypeName () == (LCIO::SIMTRACKERHIT)) )
		
		
		//Open the same collection in the physics event -- we use take here to get the
		
		try
		  {
		    Collection_in_Physics_Evt = evt -> getCollection( Collection_name );
		  }
		catch (DataNotAvailableException& e)
		  {
		    streamlog_out(DEBUG) << "Add new Collection" << Collection_in_overlay_Evt->getTypeName() << " with name " << Collection_name <<  std::endl;
		    LCCollectionVec* new_collection = new  LCCollectionVec(Collection_in_overlay_Evt->getTypeName());
		    
		    StringVec stringKeys ;
		    Collection_in_overlay_Evt->getParameters().getStringKeys( stringKeys ) ;
		    for(unsigned i=0; i< stringKeys.size() ; i++ ){
		      StringVec vals ;
		      Collection_in_overlay_Evt->getParameters().getStringVals(  stringKeys[i] , vals ) ;
		      new_collection->parameters().setValues(  stringKeys[i] , vals ) ;   
		    }
		    StringVec intKeys ;
		    Collection_in_overlay_Evt->getParameters().getIntKeys( intKeys ) ;
		    for(unsigned i=0; i< intKeys.size() ; i++ ){
		      IntVec vals ;
		      Collection_in_overlay_Evt->getParameters().getIntVals(  intKeys[i] , vals ) ;
		      new_collection->parameters().setValues(  intKeys[i] , vals ) ;   
		    }
		    StringVec floatKeys ;
		    Collection_in_overlay_Evt->getParameters().getFloatKeys( floatKeys ) ;
		    for(unsigned i=0; i< floatKeys.size() ; i++ ){
		      FloatVec vals ;
		      Collection_in_overlay_Evt->getParameters().getFloatVals(  floatKeys[i] , vals ) ;
		      new_collection->parameters().setValues(  floatKeys[i] , vals ) ;   
		    }
		    
		    //there is a special Treatment for the TPC Hits in Frank's Processor... don't know why, I just do the same
		    if (Collection_name == "TPCCollection")
		      {
			LCFlagImpl thFlag(0) ;
			thFlag.setBit( LCIO::THBIT_MOMENTUM ) ;
			new_collection ->setFlag( thFlag.getFlag()  ) ;
		      }
		    //-----------------------------------------------------------------
		    
		    evt -> addCollection ( new_collection, Collection_name);
		    Collection_in_Physics_Evt = evt -> getCollection( Collection_name ) ;
		  }
	      
	      //Now we merge the collections
	      merge_collections (Collection_in_overlay_Evt, Collection_in_Physics_Evt, BX_number_in_train*_T_diff ); 
	    }
	}
    }
    overlay_Eventfile_reader -> close();
  }//If we have any files, and more than 0 events to overlay end 
  delete permutation;

  ++_nEvt;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void OverlayTiming::define_time_windows(std::string& Collection_name)
{
  this_start = -0.25; //the integration time shall start shortly before the BX with the physics event to avoid timing problems
                      //the value of -0.25 is a arbitrary number for the moment but should be sufficient -- corresponds to 7.5cm of flight at c
   if ( Collection_name == "BeamCalCollection")                      {this_stop = _BeamCal_int;                 TPC_hits = false;} 
   else if ( Collection_name == "ETDCollection")                     {this_stop = _ETD_int;                     TPC_hits = false;} 
   else if ( Collection_name == "EcalBarrelCollection")              {this_stop = _EcalBarrel_int;              TPC_hits = false;} 
   else if ( Collection_name == "EcalBarrelPreShowerCollection")     {this_stop = _EcalBarrelPreShower_int;     TPC_hits = false;} 
   else if ( Collection_name == "EcalEndcapCollection")              {this_stop = _EcalEndcap_int;              TPC_hits = false;} 
   else if ( Collection_name == "EcalEndcapPreShowerCollection")     {this_stop = _EcalEndcapPreShower_int;     TPC_hits = false;} 
   else if ( Collection_name == "EcalEndcapRingCollection")          {this_stop = _EcalEndcapRing_int;          TPC_hits = false;} 
   else if ( Collection_name == "EcalEndcapRingPreShowerCollection") {this_stop = _EcalEndcapRingPreShower_int; TPC_hits = false;} 
   else if ( Collection_name == "FTDCollection")                     {this_stop = _FTD_int;                     TPC_hits = false;} 
   else if ( Collection_name == "HcalBarrelRegCollection")           {this_stop = _HcalBarrelReg_int;           TPC_hits = false;} 
   else if ( Collection_name == "HcalEndCapRingsCollection")         {this_stop = _HcalEndCapRings_int;         TPC_hits = false;} 
   else if ( Collection_name == "HcalEndCapsCollection")             {this_stop = _HcalEndCaps_int;             TPC_hits = false;} 
   else if ( Collection_name == "LHcalCollection")                   {this_stop = _LHcal_int;                   TPC_hits = false;} 
   else if ( Collection_name == "LumiCalCollection")                 {this_stop = _LumiCal_int;                 TPC_hits = false;} 
   else if ( Collection_name == "MuonBarrelCollection")              {this_stop = _MuonBarrel_int;              TPC_hits = false;} 
   else if ( Collection_name == "MuonEndCapCollection")              {this_stop = _MuonEndCap_int;              TPC_hits = false;} 
   else if ( Collection_name == "SETCollection")                     {this_stop = _SET_int;                     TPC_hits = false;} 
   else if ( Collection_name == "SITCollection")                     {this_stop = _SIT_int;                     TPC_hits = false;} 
   else if ( Collection_name == "VXDCollection")                     {this_stop = _VXD_int;                     TPC_hits = false;} 
   else if ( Collection_name == "TPCCollection")                     {this_start = -_TPC_int/2;                     this_stop =  _TPC_int/2; TPC_hits = true;} 
   else if ( Collection_name == "TPCSpacePointCollection")           {this_start = -_TPCSpacePoint_int/2; this_stop =  _TPCSpacePoint_int/2; TPC_hits = true;} 

}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OverlayTiming::crop_collection (LCCollection* collection)
{
 
  if (collection -> getNumberOfElements() >0)
    {


      float _time_of_flight;
      int number_of_elements = collection -> getNumberOfElements ();

      if (collection -> getTypeName () == (LCIO::SIMTRACKERHIT))
	{ 
	  for (int k =  number_of_elements-1; k >= 0; --k)
	    {
	      SimTrackerHit* TrackerHit = dynamic_cast <  SimTrackerHit* > (  collection -> getElementAt (k));
	      _time_of_flight = time_of_flight (TrackerHit -> getPosition ()[0],TrackerHit -> getPosition ()[1], TrackerHit -> getPosition ()[2]);
	      if (!(TrackerHit ->getTime() > this_start + _time_of_flight &&  TrackerHit ->getTime() < this_stop +  _time_of_flight))
		{
		  collection  ->removeElementAt(k);
		  delete TrackerHit;
		}
	    }
	  
  	}
      else if (collection->getTypeName () == (LCIO::SIMCALORIMETERHIT))
	{ 
	  SimCalorimeterHit* CalorimeterHit;
	  SimCalorimeterHitImpl* newCalorimeterHit;
	  int not_within_time_window= 0;

	  //we count from to to bottom, in order not to get confused when removing and adding elements!
	  for (int i =  number_of_elements-1; i >= 0; --i) 
	    {
	      CalorimeterHit = dynamic_cast< SimCalorimeterHit* > (collection  -> getElementAt(i));

	      not_within_time_window= 0;
	      
	      // check whether all entries are within the time window
	      _time_of_flight = time_of_flight (CalorimeterHit -> getPosition ()[0],CalorimeterHit -> getPosition ()[1], CalorimeterHit -> getPosition ()[2]);
	      for( int j=0 ; j < CalorimeterHit->getNMCContributions(); ++j)
		{
		  //we need to shift the time window to account for the time of flight of the particle...

		  if (!(CalorimeterHit->getTimeCont(j) > (this_start + _time_of_flight) && CalorimeterHit->getTimeCont(j) < (this_stop + _time_of_flight)))
		    {
		      ++ not_within_time_window;
		      //std::cout << " calo hit : " << j << " Time : " << CalorimeterHit->getTimeCont(j) << " ?> " << this_start + _time_of_flight << " ?< " << this_stop + _time_of_flight << std::endl; 
		    }
		}     

	      //if one and not all MC contribution is not within the time window....
	      if ( not_within_time_window == 0)
		{
		  destMap.insert( DestMap::value_type(cellID2long(CalorimeterHit->getCellID0(), CalorimeterHit->getCellID1()), CalorimeterHit) );
		}
	      else if (not_within_time_window > 0 && not_within_time_window < CalorimeterHit->getNMCContributions())
		{

 		  newCalorimeterHit = new SimCalorimeterHitImpl();

		  for( int j=0 ; j < CalorimeterHit->getNMCContributions(); ++j)
		    {
		      if ((CalorimeterHit->getTimeCont(j))  > (this_start+ _time_of_flight) && (CalorimeterHit->getTimeCont(j))  < (this_stop  + _time_of_flight))
			{
			  newCalorimeterHit->addMCParticleContribution(  CalorimeterHit ->getParticleCont(j),  CalorimeterHit ->getEnergyCont(j),  CalorimeterHit ->getTimeCont(j), CalorimeterHit ->getPDGCont(j));
			}
		    }

		  newCalorimeterHit -> setCellID0 (CalorimeterHit->getCellID0 ());
		  newCalorimeterHit -> setCellID1 (CalorimeterHit->getCellID1 ());
		  float ort[3] = {CalorimeterHit -> getPosition ()[0],CalorimeterHit -> getPosition ()[1], CalorimeterHit -> getPosition ()[2]};
		  newCalorimeterHit -> setPosition (ort);

		  collection->removeElementAt(i);
		  delete CalorimeterHit;

		  collection->addElement(newCalorimeterHit);
		  destMap.insert( DestMap::value_type(cellID2long(newCalorimeterHit->getCellID0(), newCalorimeterHit->getCellID1()), newCalorimeterHit) );
		}
	      else if (not_within_time_window == CalorimeterHit->getNMCContributions())
		{
		  collection->removeElementAt(i);
		  delete CalorimeterHit;
		}
	    }
      	}
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void OverlayTiming::merge_collections (LCCollection* source_collection, LCCollection* dest_collection, float time_offset)
{

  // first, calculate the integration time, depending on the subdetector
  // time offset is the time of the physics event, after the start of the bunch train
  // adding the time offset shall move the background event relativ to the physics event...

  if (source_collection -> getNumberOfElements() >0)
    {
      float _time_of_flight;
      int number_of_elements = source_collection -> getNumberOfElements ();

      if (source_collection -> getTypeName () == (LCIO::MCPARTICLE))
	{
	  MCParticleImpl* MC_Part;
	  for (int i =  number_of_elements-1; i >= 0; --i) 
	    {
	      MC_Part = dynamic_cast < MCParticleImpl* > ( source_collection -> getElementAt (i) );
	      MC_Part -> setTime(MC_Part -> getTime() + time_offset);
	      dest_collection -> addElement ( MC_Part );
	      //we must delete the element from the old collection, otherwise it vanishes when the event is closed!
	      source_collection -> removeElementAt (i);
	    }
	}
      else if (source_collection -> getTypeName () == (LCIO::SIMTRACKERHIT) && (time_offset == 0 || !TPC_hits))
	{ 
	  for (int k =  number_of_elements-1; k >= 0; --k) 
	    {
	      SimTrackerHitImpl* TrackerHit = dynamic_cast <  SimTrackerHitImpl* > (  source_collection -> getElementAt (k));
	      
	      _time_of_flight = time_of_flight (TrackerHit -> getPosition ()[0],TrackerHit -> getPosition ()[1], TrackerHit -> getPosition ()[2]);
	      
	      if ((TrackerHit ->getTime()+time_offset)  > (this_start + _time_of_flight)  && (TrackerHit ->getTime()+time_offset)  < (this_stop +  _time_of_flight ))
		{
		  TrackerHit ->setTime( TrackerHit->getTime() + time_offset);
		  dest_collection -> addElement ( TrackerHit );
		  source_collection -> removeElementAt (k);
		}
	    }
	}
      else if (source_collection -> getTypeName () == (LCIO::SIMTRACKERHIT) && TPC_hits)
	{ 
	  for (int k =  number_of_elements-1; k >= 0; --k) 
	    {
	      SimTrackerHitImpl* TrackerHit = dynamic_cast <  SimTrackerHitImpl* > (  source_collection -> getElementAt (k));
	      
	      _time_of_flight = time_of_flight (TrackerHit -> getPosition ()[0],TrackerHit -> getPosition ()[1], TrackerHit -> getPosition ()[2]);
	      
	      if ((TrackerHit ->getTime()+time_offset)  > (this_start + _time_of_flight)  && (TrackerHit ->getTime()+time_offset)  < (this_stop +  _time_of_flight ))
		{
		  TrackerHit ->setTime( TrackerHit->getTime() + time_offset);
		  double ort[3] = {TrackerHit-> getPosition ()[0],TrackerHit-> getPosition ()[1], 0 };
		  if (TrackerHit-> getPosition ()[2] <= 0)
		    { 
		      ort[2] = TrackerHit-> getPosition ()[2] - time_offset * _tpcVdrift_mm_ns ;
		    }
		  else 
		    { 
		      ort[2] = TrackerHit-> getPosition ()[2] + time_offset * _tpcVdrift_mm_ns ;
		    }
		  TrackerHit ->setPosition (ort);
		  dest_collection -> addElement ( TrackerHit );
		  source_collection -> removeElementAt (k);
		}
	    }
	}      
      else if (source_collection->getTypeName () == (LCIO::SIMCALORIMETERHIT))
	{ 
	  // create a map of dest Collection
	  DestMap::const_iterator destMapIt;

	  SimCalorimeterHit* CalorimeterHit;
	  SimCalorimeterHitImpl* newCalorimeterHit;

	  for (int k =  number_of_elements-1; k >= 0; --k) 
	    {
	      CalorimeterHit    = dynamic_cast <  SimCalorimeterHit* > (  source_collection -> getElementAt (k));
	      //check whether there is already a hit at this position 
	      _time_of_flight = time_of_flight (CalorimeterHit -> getPosition ()[0],CalorimeterHit -> getPosition ()[1], CalorimeterHit -> getPosition ()[2]);

	      destMapIt = destMap.find(cellID2long(CalorimeterHit -> getCellID0(), CalorimeterHit -> getCellID1()));
	      if (destMapIt == destMap.end())
		{
		  // There is no Hit at this position -- the new hit can be added, if it is not outside the window

		  newCalorimeterHit = new SimCalorimeterHitImpl();
		  bool add_Hit = false;
		  
		  for( int j=0 ; j < CalorimeterHit->getNMCContributions(); ++j)
		    {
		      _time_of_flight = time_of_flight (CalorimeterHit -> getPosition ()[0],CalorimeterHit -> getPosition ()[1], CalorimeterHit -> getPosition ()[2]);
		      if ((CalorimeterHit->getTimeCont(j) +  time_offset)  > (this_start + _time_of_flight) && (CalorimeterHit->getTimeCont(j) +  time_offset)  < (this_stop  + _time_of_flight))	
			{
			  add_Hit = true;
			  newCalorimeterHit->addMCParticleContribution(  CalorimeterHit ->getParticleCont(j),  CalorimeterHit ->getEnergyCont(j),  CalorimeterHit ->getTimeCont(j)+  time_offset, 
									 CalorimeterHit ->getPDGCont(j));
			}
		    }	  
		  if(add_Hit) 
		    {
		      newCalorimeterHit -> setCellID0 (CalorimeterHit->getCellID0 ());
		      newCalorimeterHit -> setCellID1 (CalorimeterHit->getCellID1 ());
		      float ort[3] = {CalorimeterHit -> getPosition ()[0],CalorimeterHit -> getPosition ()[1], CalorimeterHit -> getPosition ()[2]};
		      newCalorimeterHit -> setPosition (ort);
		      dest_collection -> addElement ( newCalorimeterHit );
		      destMap.insert( DestMap::value_type(cellID2long(newCalorimeterHit->getCellID0(), newCalorimeterHit->getCellID1()), newCalorimeterHit) );
		    } else {
		    delete newCalorimeterHit;
		  }
		}
	      else 
		{
		  // there is already a hit at this position.... 
		  newCalorimeterHit = dynamic_cast <  SimCalorimeterHitImpl* > ( destMapIt->second );
		  for( int j=0 ; j < CalorimeterHit->getNMCContributions(); ++j)
		    {
		      if ((CalorimeterHit->getTimeCont(j) +  time_offset)  > (this_start + _time_of_flight) && (CalorimeterHit->getTimeCont(j) +  time_offset)  < (this_stop  + _time_of_flight))
			{
			  newCalorimeterHit->addMCParticleContribution(  CalorimeterHit ->getParticleCont(j),  CalorimeterHit ->getEnergyCont(j),  CalorimeterHit ->getTimeCont(j)
									  +  time_offset, CalorimeterHit ->getPDGCont(j));
			}
		    }
		}
 	    }
	}
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float OverlayTiming::time_of_flight(float x, float y, float z)
{
  //returns the time of flight to the radius in ns
  // mm/m/s = 10^{-3}s = 10^6 ns d.h. 299 mm/ns
  return sqrt(pow(x,2)+pow(y,2)+pow(z,2))/299.792458;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OverlayTiming::check( LCEvent * evt ) { 

  const std::vector<std::string > * collection_names_in_evt = evt -> getCollectionNames ();
  
  for (unsigned int i=0; i < collection_names_in_evt->size(); ++i)
    {
      streamlog_out(DEBUG) << "Collection " << collection_names_in_evt->at(i) << " has now " << evt -> getCollection( collection_names_in_evt->at(i) )->getNumberOfElements() << " elements" << std::endl;
    }
  //we clear the map of calorimeter hits for the next event
  destMap.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OverlayTiming::end(){ 
}

