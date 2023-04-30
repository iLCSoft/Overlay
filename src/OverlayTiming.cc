#include "OverlayTiming.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/ReconstructedParticle.h>
#include <EVENT/SimTrackerHit.h>

#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCFlagImpl.h>
#include <IMPL/MCParticleImpl.h>
#include <IMPL/SimCalorimeterHitImpl.h>
#include <IMPL/SimTrackerHitImpl.h>

#include <marlin/Exceptions.h>
#include <marlin/Global.h>
#include <marlin/ProcessorEventSeeder.h>

#include <algorithm>
#include <limits>
#include <random>
#include <set>

using namespace lcio;
using namespace marlin;

namespace overlay {

  OverlayTiming aOverlayTiming;

  OverlayTiming::OverlayTiming( std::string const& procName) : Processor(procName)
  {}

  OverlayTiming::OverlayTiming() : Processor("OverlayTiming")
  {
    // modify processor description
    _description = "Processor to overlay events from the background taking the timing of the subdetectors into account";

    StringVec files;

    registerProcessorParameter("Delta_t",
			       "Time difference between bunches in the bunch train in ns",
			       _T_diff,
			       float(0.5));

    registerProcessorParameter("NBunchtrain",
			       "Number of bunches in a bunch train",
			       _nBunchTrain,
			       int(1));

    registerProcessorParameter( "BackgroundFileNames",
				"Name of the lcio input file(s) with background - assume one file per bunch crossing.",
				_inputFileNames,
				files);

    registerProcessorParameter("PhysicsBX",
			       "Number of the Bunch crossing of the physics event",
			       _BX_phys,
			       int(1));

    registerProcessorParameter( "TPCDriftvelocity", 
				"[mm/ns] (float) - default 5.0e-2 (5cm/us)",
				_tpcVdrift_mm_ns,
				float(5.0e-2) );

    registerProcessorParameter( "RandomBx", 
				"Place the physics event at an random position in the train: overrides PhysicsBX",
				_randomBX,
				bool(false) );

    registerProcessorParameter( "NumberBackground", 
				"Number of Background events to overlay - either fixed or Poisson mean",
				_NOverlay,
				float(1) );

    registerProcessorParameter( "Poisson_random_NOverlay", 
				"Draw random number of Events to overlay from Poisson distribution with  mean value NumberBackground",
				_Poisson,
				bool(false) );

    registerProcessorParameter("MCParticleCollectionName",
			       "The MC Particle Collection Name",
			       _mcParticleCollectionName,
			       std::string("MCParticle"));

    registerProcessorParameter("MCPhysicsParticleCollectionName",
			       "The output MC Particle Collection Name for the physics event" ,
			       _mcPhysicsParticleCollectionName,
			       std::string("MCPhysicsParticles"));

    //Collections with Integration Times
    registerProcessorParameter("BeamCalCollection_IntegrationTime",
			       "Integration time for the BeamCalCollection",
			       _BeamCal_int,
			       float(10));

    registerProcessorParameter("LumiCalCollection_Integration_Time",
			       "Integration time for the LumiCalCollection",
			       _LumiCal_int,
			       float(10));

    registerProcessorParameter("EcalBarrelCollection_Integration_Time",
			       "Integration time for the EcalBarrelCollection / ECalBarrelCollection",
			       _EcalBarrel_int,
			       float(10));

    registerProcessorParameter("EcalEndcapCollection_Integration_Time",
			       "Integration time for the EcalEndcapCollection / ECalEndcapCollection",
			       _EcalEndcap_int,
			       float(10));

    registerProcessorParameter("HcalBarrelRegCollection_Integration_Time",
			       "Integration time for the HcalBarrelRegCollection / HCalBarrelCollection",
			       _HcalBarrelReg_int,
			       float(10));

    registerProcessorParameter("HcalEndCapsCollection_Integration_Time",
			       "Integration time for the HcalEndCapsCollection / HCalEndcapCollection",
			       _HcalEndCaps_int,
			       float(10));

    registerProcessorParameter("HcalEndCapRingsCollection_Integration_Time",
			       "Integration time for the HcalEndCapRingsCollection / HCalRingCollection",
			       _HcalEndCapRings_int,
			       float(10));

    registerProcessorParameter("MuonBarrelCollection_Integration_Time",
			       "Integration time for the MuonBarrelCollection / YokeBarrelCollection",
			       _MuonBarrel_int,
			       float(10));

    registerProcessorParameter("MuonEndCapCollection_Integration_Time",
			       "Integration time for the MuonEndCapCollection / YokeEndcapCollection",
			       _MuonEndCap_int,
			       float(10));


    // ILD specific

    registerProcessorParameter("LHcalCollection_Integration_Time",
			       "Integration time for the LHcalCollection",
			       _LHcal_int,
			       float(10));

    registerProcessorParameter("EcalEndcapRingCollection_Integration_Time",
			       "Integration time for the EcalEndcapRingCollection",
			       _EcalEndcapRing_int,
			       float(10));

    registerProcessorParameter("EcalBarrelPreShowerCollection_Integration_Time",
			       "Integration time for the EcalBarrelPreShowerCollection",
			       _EcalBarrelPreShower_int,
			       float(10));

    registerProcessorParameter("EcalEndcapPreShowerCollection_Integration_Time",
			       "Integration time for the EcalEndcapPreShowerCollection",
			       _EcalEndcapPreShower_int,
			       float(10));

    registerProcessorParameter("EcalEndcapRingPreShowerCollection_Integration_Time",
			       "Integration time for the  EcalEndcapRingPreShowerCollection",
			       _EcalEndcapRingPreShower_int,
			       float(10));
    
    registerProcessorParameter("ETDCollection_Integration_Time",
			       "Integration time for the ETDCollection",
			       _ETD_int,
			       float(10));

    registerProcessorParameter("FTDCollection_Integration_Time",
			       "Integration time for the FTDCollection",
			       _FTD_int,
			       float(10));

    registerProcessorParameter("SETCollection_Integration_Time",
			       "Integration time for the SETCollection",
			       _SET_int,
			       float(10));

    registerProcessorParameter("SITCollection_Integration_Time",
			       "Integration time for the SITCollection",
			       _SIT_int,
			       float(10));

    registerProcessorParameter("VXDCollection_Integration_Time",
			       "Integration time for the VXDCollection",
			       _VXD_int,
			       float(10));

    registerProcessorParameter("TPCCollection_Integration_Time",
			       "Integration time for the TPCCollection",
			       _TPC_int,
			       float(10));

    registerProcessorParameter("TPCSpacePointCollection_Integration_Time",
			       "Integration time for the TPCSpacePointCollection",
			       _TPCSpacePoint_int,
			       float(10));

    
    // CLIC specific

    registerProcessorParameter("EcalPlugCollection_Integration_Time",
			       "Integration time for the ECalPlugCollection",
			       _EcalPlug_int,
			       float(10));

    registerProcessorParameter("VertexBarrelCollection_Integration_Time",
			       "Integration time for the VertexBarrelCollection",
			       _VXDB_int,
			       float(10));

    registerProcessorParameter("VertexEndcapCollection_Integration_Time",
			       "Integration time for the VertexEndcapCollection",
			       _VXDE_int,
			       float(10));

    registerProcessorParameter("InnerTrackerBarrelCollection_Integration_Time",
			       "Integration time for the InnerTrackerBarrelCollection",
			       _ITB_int,
			       float(10));

    registerProcessorParameter("InnerTrackerEndcapCollection_Integration_Time",
			       "Integration time for the InnerTrackerEndcapCollection",
			       _ITE_int,
			       float(10));

    registerProcessorParameter("OuterTrackerBarrelCollection_Integration_Time",
			       "Integration time for the OuterTrackerBarrelCollection",
			       _OTB_int,
			       float(10));

    registerProcessorParameter("OuterTrackerEndcapCollection_Integration_Time",
			       "Integration time for the OuterTrackerEndcapCollection",
			       _OTE_int,
			       float(10));

    registerProcessorParameter("Start_Integration_Time",
                               "Starting integration time.  Should be shortly before the BX, but may need to be shifted earlier if the vertex is smeared in time.",
                               _DefaultStart_int,
                               float(-0.25));

    registerProcessorParameter("AllowReusingBackgroundFiles",
                               "If true the same background file can be used for the same event",
                               m_allowReusingBackgroundFiles,
                               m_allowReusingBackgroundFiles);

    registerOptionalParameter("StartBackgroundFileIndex",
			       "Which background file to startWith",
			       m_startWithBackgroundFile,
			       m_startWithBackgroundFile);

    registerOptionalParameter("StartBackgroundEventIndex",
			       "Which background event to startWith",
			       m_startWithBackgroundEvent,
			       m_startWithBackgroundEvent);


  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::init()
  {
    streamlog_out(DEBUG) << " init called  " << std::endl;
    printParameters();

    overlay_Eventfile_reader = LCFactory::getInstance()->createLCReader();

    streamlog_out(WARNING) << "Attention! There are " << _inputFileNames.size()
			   << " files in the list of background files to overlay. Make sure that the total number of background events is sufficiently large for your needs!!"
			   << std::endl;

    Global::EVENTSEEDER->registerProcessor(this);

    _nRun = 0;
    _nEvt = 0;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::processRunHeader(EVENT::LCRunHeader*)
  {
    _nRun++;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::modifyEvent(EVENT::LCEvent *evt)
  {

    CLHEP::HepRandom::setTheSeed( Global::EVENTSEEDER->getSeed(this) );

    if (_randomBX) 
      {
        _BX_phys = CLHEP::RandFlat::shootInt(_nBunchTrain);
        streamlog_out(DEBUG) << "Physics Event was placed in the " << _BX_phys << " bunch crossing!" << std::endl;
      }

    std::set<int> usedFiles;

    //define a permutation for the events to overlay -- the physics event is per definition at position 0
    std::vector<int> *permutation = new std::vector<int>;

    for (int i = -(_BX_phys - 1); i < _nBunchTrain - (_BX_phys - 1); ++i)
      {
        permutation->push_back(i);
      }

    std::mt19937 urng( Global::EVENTSEEDER->getSeed(this)  );
    std::shuffle(permutation->begin(), permutation->end(), urng );

    int random_file = CLHEP::RandFlat::shootInt(_inputFileNames.size());

    if( m_startWithBackgroundFile >= 0 ) {
      random_file = m_startWithBackgroundFile;
      m_startWithBackgroundFile = -1;
    }
    //Make sure we have filenames to open and that we really want to overlay something
    if ((random_file > -1) && (_NOverlay > 0.) && (overlay_Evt == nullptr) && (_inputFileNames.size() > 0))
      {
        overlay_Eventfile_reader->open(_inputFileNames.at(random_file));
        m_currentFileIndex = random_file;
        m_eventCounter = -1;
        streamlog_out(MESSAGE) << "Open background file: " << _inputFileNames.at(random_file) << std::endl;
      }

    usedFiles.insert(m_currentFileIndex);

    // read events until we get the event we want to start with
    if( m_startWithBackgroundEvent >= 0 ) {
      streamlog_out(MESSAGE) << "Skipping to event: " << m_startWithBackgroundEvent << std::endl;
      while(  m_eventCounter < m_startWithBackgroundEvent ) {
        overlay_Evt = overlay_Eventfile_reader->readNextEvent(LCIO::UPDATE);
        ++m_eventCounter;
      }
      m_startWithBackgroundEvent = -1;
    }

    if (_inputFileNames.size() > 0) {
      streamlog_out(MESSAGE) << "Using background file: " << " [ " << m_currentFileIndex << " ] "
                             << _inputFileNames.at(m_currentFileIndex)
                             << std::endl;
    } else {
      streamlog_out(MESSAGE) << "No background file " << std::endl;
    }

    streamlog_out(MESSAGE) << "Starting at event: " << m_eventCounter << std::endl;

    //We have the physics event in evt. Now we merge the new overlay events with it.
    //First cut the collections in the physics event to the defined time windows
    const std::vector<std::string> *collection_names_in_Evt = evt->getCollectionNames();

    for (unsigned int j = 0, nCollections = collection_names_in_Evt->size(); j < nCollections; ++j)
      {
        const std::string Collection_name = collection_names_in_Evt->at(j);
        LCCollection *Collection_in_Physics_Evt = evt->getCollection(Collection_name);
	currentDest = Collection_name;
	if ((Collection_in_Physics_Evt->getTypeName() == LCIO::SIMCALORIMETERHIT) || (Collection_in_Physics_Evt->getTypeName() == LCIO::SIMTRACKERHIT))
	  {
            define_time_windows(Collection_name);
	    streamlog_out(DEBUG) << "Cropping collection: " << Collection_name << std::endl;
            crop_collection(Collection_in_Physics_Evt);
	  }

        // copy MCParticles for physics event into a new collection
        if (Collection_in_Physics_Evt->getTypeName() == LCIO::MCPARTICLE)
	  {
            const int number_of_elements = Collection_in_Physics_Evt->getNumberOfElements();
            if (number_of_elements > 0)
	      {
                LCCollectionVec *colPhysicsMc = new LCCollectionVec(LCIO::MCPARTICLE);
                colPhysicsMc->setSubset(true);
                for (int k = 0; k < number_of_elements; ++k)
		  {
                    MCParticle *pMc = static_cast<MCParticle*>(Collection_in_Physics_Evt->getElementAt(k));
                    colPhysicsMc->addElement(pMc);
		  }
                evt->addCollection(colPhysicsMc,_mcPhysicsParticleCollectionName.c_str());
	      }
	  }
      }

    if ((_inputFileNames.size() > 0) && (_NOverlay > 0.))
      {
        //Now overlay the background evnts to each bunchcrossing in the bunch train
        for (int bxInTrain = 0; bxInTrain < _nBunchTrain; ++bxInTrain)
	  {
            const int BX_number_in_train = permutation->at(bxInTrain);

            int NOverlay_to_this_BX = 0;

            if (_Poisson)
	      {
                NOverlay_to_this_BX = int(CLHEP::RandPoisson::shoot(_NOverlay));
	      }
            else
	      {
                NOverlay_to_this_BX = int(_NOverlay);
	      }

            streamlog_out(DEBUG) << "Will overlay " << NOverlay_to_this_BX << " events to BX number " << BX_number_in_train+_BX_phys << std::endl;

            for (int k = 0; k < NOverlay_to_this_BX; ++k)
	      {
                overlay_Evt = overlay_Eventfile_reader->readNextEvent(LCIO::UPDATE);
                ++m_eventCounter;
                //if there are no events left in the actual file, open the next one.
                if (overlay_Evt == 0)
		  {
                    overlay_Eventfile_reader->close();

                    // used all available files
                    if (usedFiles.size() == _inputFileNames.size()) {
                      if (not m_allowReusingBackgroundFiles) {
                        throw marlin::StopProcessingException(this);
                      }
                      usedFiles.clear();
                      if (_inputFileNames.size() > 1) {
                        // do not use the same file immediately if we have more than 1
                        usedFiles.insert(m_currentFileIndex);
                      }
                    }

                    do {
                      m_currentFileIndex = random_file = CLHEP::RandFlat::shootInt(_inputFileNames.size());
                    } while (usedFiles.count(m_currentFileIndex) == 1);
                    usedFiles.insert(m_currentFileIndex);
                    overlay_Eventfile_reader->open (_inputFileNames.at(random_file));
                    overlay_Evt = overlay_Eventfile_reader->readNextEvent(LCIO::UPDATE);
                    m_eventCounter = 0; // this has to be zero, because we just read the first event of the file!
                    streamlog_out(MESSAGE) << "Open background file: " << _inputFileNames.at(random_file) << std::endl;
		  }

                // the overlay_Event is now open, start to merge its collections with the ones of the accumulated overlay events collections
                // all the preparatory work has been done now....
                // first, let's see which collections are in the event

                //first include the MCParticles into the physics event
                try
		  {
		    //Do Not Need DestMap, because this is only MCParticles
		    currentDest=_mcParticleCollectionName;
		    streamlog_out(DEBUG) << "Merging MCParticles " << std::endl;
		    merge_collections(overlay_Evt->getCollection(_mcParticleCollectionName), evt->getCollection(_mcParticleCollectionName), BX_number_in_train * _T_diff);
		  }
                catch (DataNotAvailableException& e)
		  {
                    streamlog_out(ERROR) << "Failed to extract MCParticle collection: " << e.what() << std::endl;
                    throw e;
		  }

                collection_names_in_Evt = overlay_Evt->getCollectionNames();

                for (unsigned int j = 0; j < collection_names_in_Evt->size(); ++j)
		  {
                    const std::string Collection_name = collection_names_in_Evt->at(j);

                    LCCollection *Collection_in_overlay_Evt = overlay_Evt->getCollection(Collection_name);
                    LCCollection *Collection_in_Physics_Evt = 0;

                    //Skip the MCParticle collection
                    if( Collection_name == _mcParticleCollectionName ) {
                      continue;
                    }

                    define_time_windows(Collection_name);

                    //the event can only make contributions to the readout, if the bx does not happen after the integration time stopped.
                    //and we are only interested in Calorimeter or Trackerhits.

                    if ((this_stop > (BX_number_in_train - _BX_phys) * _T_diff) &&
			((Collection_in_overlay_Evt->getTypeName() == LCIO::SIMCALORIMETERHIT) || (Collection_in_overlay_Evt->getTypeName() == LCIO::SIMTRACKERHIT)) )
		      {
                        //Open the same collection in the physics event
                        try
			  {
                            Collection_in_Physics_Evt = evt->getCollection(Collection_name);
			  }
                        catch (DataNotAvailableException& e)
			  {
                            streamlog_out(DEBUG) << "Add new Collection" << Collection_in_overlay_Evt->getTypeName() << " with name " << Collection_name << std::endl;
                            LCCollectionVec *new_collection = new LCCollectionVec(Collection_in_overlay_Evt->getTypeName());

                            StringVec stringKeys;
                            Collection_in_overlay_Evt->getParameters().getStringKeys(stringKeys);
                            for (unsigned i = 0, nStringKeys = stringKeys.size(); i < nStringKeys; ++i)
			      {
                                StringVec vals;
                                Collection_in_overlay_Evt->getParameters().getStringVals(stringKeys[i], vals);
                                new_collection->parameters().setValues(stringKeys[i], vals);
			      }
                            StringVec intKeys;
                            Collection_in_overlay_Evt->getParameters().getIntKeys(intKeys);
                            for (unsigned i = 0, nIntKeys = intKeys.size(); i < nIntKeys; ++i)
			      {
                                IntVec vals;
                                Collection_in_overlay_Evt->getParameters().getIntVals(intKeys[i], vals);
                                new_collection->parameters().setValues(intKeys[i], vals);
			      }
                            StringVec floatKeys;
                            Collection_in_overlay_Evt->getParameters().getFloatKeys(floatKeys);
                            for (unsigned i = 0, nFloatKeys = floatKeys.size(); i < nFloatKeys; ++i)
			      {
                                FloatVec vals;
                                Collection_in_overlay_Evt->getParameters().getFloatVals(floatKeys[i], vals);
                                new_collection->parameters().setValues(floatKeys[i], vals);
			      }
                            //there is a special Treatment for the TPC Hits in Frank's Processor... don't know why, I just do the same
                            if (Collection_name == "TPCCollection")
			      {
                                LCFlagImpl thFlag(0);
                                thFlag.setBit(LCIO::THBIT_MOMENTUM);
                                new_collection->setFlag(thFlag.getFlag());
			      }

                            evt->addCollection(new_collection, Collection_name);
                            Collection_in_Physics_Evt = evt->getCollection(Collection_name);
			  }
			
			//Set DestMap back to the one for the Collection Name...
			currentDest=Collection_name;
			streamlog_out(DEBUG) << "Now overlaying collection " << Collection_name 
					     << " And we have " << collDestMap[currentDest].size() << " Hits in destMap"
					     << std::endl;
			//Now we merge the collections
                        merge_collections(Collection_in_overlay_Evt, Collection_in_Physics_Evt, BX_number_in_train * _T_diff);
		      }
		  }
	      }
	  }
      } //If we have any files, and more than 0 events to overlay end 

    delete permutation;
    ++_nEvt;
    //we clear the map of calorimeter hits for the next event
    collDestMap.clear();
    const std::vector<std::string> *collection_names_in_evt = evt->getCollectionNames();

    for (unsigned int i = 0; i < collection_names_in_evt->size(); ++i)
      {
        streamlog_out(DEBUG) << "Collection " << collection_names_in_evt->at(i) << " has now " << evt->getCollection(collection_names_in_evt->at(i))->getNumberOfElements() << " elements" << std::endl;
      }

  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::define_time_windows(const std::string &Collection_name)
  {
    this_start = _DefaultStart_int; //the integration time shall start shortly before the BX with the physics event to avoid timing problems
                        //the value of -0.25 is an arbitrary number for the moment but should be sufficient -- corresponds to 7.5cm of flight at c
                        //But if vertex smearing is on, this may need to be shifted earlier.

    this_stop = std::numeric_limits<float>::max(); // provide default values for collections not named below;
    TPC_hits = false;
    
    // CLIC / ILD common name collections  
    if ( Collection_name == "BeamCalCollection")                      {this_stop = _BeamCal_int;                 TPC_hits = false;}
    else if ( Collection_name == "LumiCalCollection")                 {this_stop = _LumiCal_int;                 TPC_hits = false;}

    // ILD
    // calo
    else if ( Collection_name == "EcalBarrelCollection")              {this_stop = _EcalBarrel_int;              TPC_hits = false;}
    else if ( Collection_name == "EcalBarrelPreShowerCollection")     {this_stop = _EcalBarrelPreShower_int;     TPC_hits = false;}
    else if ( Collection_name == "EcalEndcapCollection")              {this_stop = _EcalEndcap_int;              TPC_hits = false;}
    else if ( Collection_name == "EcalEndcapPreShowerCollection")     {this_stop = _EcalEndcapPreShower_int;     TPC_hits = false;}
    else if ( Collection_name == "EcalEndcapRingCollection")          {this_stop = _EcalEndcapRing_int;          TPC_hits = false;}
    else if ( Collection_name == "EcalEndcapRingPreShowerCollection") {this_stop = _EcalEndcapRingPreShower_int; TPC_hits = false;}
    else if ( Collection_name == "HcalBarrelRegCollection")           {this_stop = _HcalBarrelReg_int;           TPC_hits = false;}
    else if ( Collection_name == "HcalEndCapRingsCollection")         {this_stop = _HcalEndCapRings_int;         TPC_hits = false;}
    else if ( Collection_name == "HcalEndCapsCollection")             {this_stop = _HcalEndCaps_int;             TPC_hits = false;}
    else if ( Collection_name == "LHcalCollection")                   {this_stop = _LHcal_int;                   TPC_hits = false;}
    // muon system 
    else if ( Collection_name == "MuonBarrelCollection")              {this_stop = _MuonBarrel_int;              TPC_hits = false;}
    else if ( Collection_name == "MuonEndCapCollection")              {this_stop = _MuonEndCap_int;              TPC_hits = false;}
    // tracker
    else if ( Collection_name == "ETDCollection")                     {this_stop = _ETD_int;                     TPC_hits = false;}
    else if ( Collection_name == "FTDCollection")                     {this_stop = _FTD_int;                     TPC_hits = false;}
    else if ( Collection_name == "SETCollection")                     {this_stop = _SET_int;                     TPC_hits = false;}
    else if ( Collection_name == "SITCollection")                     {this_stop = _SIT_int;                     TPC_hits = false;}
    else if ( Collection_name == "VXDCollection")                     {this_stop = _VXD_int;                     TPC_hits = false;}
    else if ( Collection_name == "TPCCollection")                     {this_start = -_TPC_int/2;                     this_stop =  _TPC_int/2; TPC_hits = true;}
    else if ( Collection_name == "TPCSpacePointCollection")           {this_start = -_TPCSpacePoint_int/2; this_stop =  _TPCSpacePoint_int/2; TPC_hits = true;}

    // CLIC
    // calo 
    else if ( Collection_name == "ECalBarrelCollection")              {this_stop = _EcalBarrel_int;              TPC_hits = false;}
    else if ( Collection_name == "ECalEndcapCollection")              {this_stop = _EcalEndcap_int;              TPC_hits = false;}
    else if ( Collection_name == "ECalPlugCollection")                {this_stop = _EcalPlug_int;                TPC_hits = false;}
    else if ( Collection_name == "HCalBarrelCollection")              {this_stop = _HcalBarrelReg_int;           TPC_hits = false;}
    else if ( Collection_name == "HCalEndcapCollection")              {this_stop = _HcalEndCaps_int;             TPC_hits = false;}
    else if ( Collection_name == "HCalRingCollection")                {this_stop = _HcalEndCapRings_int;         TPC_hits = false;}
    // muon system
    else if ( Collection_name == "YokeBarrelCollection")              {this_stop = _MuonBarrel_int;              TPC_hits = false;}
    else if ( Collection_name == "YokeEndcapCollection")              {this_stop = _MuonEndCap_int;              TPC_hits = false;}
    // tracker 
    else if ( Collection_name == "VertexBarrelCollection")            {this_stop = _VXDB_int;                    TPC_hits = false;}
    else if ( Collection_name == "VertexEndcapCollection")            {this_stop = _VXDE_int;                    TPC_hits = false;}
    else if ( Collection_name == "InnerTrackerBarrelCollection")      {this_stop = _ITB_int;                     TPC_hits = false;}
    else if ( Collection_name == "InnerTrackerEndcapCollection")      {this_stop = _ITE_int;                     TPC_hits = false;}
    else if ( Collection_name == "OuterTrackerBarrelCollection")      {this_stop = _OTB_int;                     TPC_hits = false;}
    else if ( Collection_name == "OuterTrackerEndcapCollection")      {this_stop = _OTE_int;                     TPC_hits = false;}
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::crop_collection (EVENT::LCCollection *collection)
  {
    const int number_of_elements = collection->getNumberOfElements();

    if (number_of_elements > 0)
      {
        if (collection->getTypeName() == LCIO::SIMTRACKERHIT)
	  {
            for (int k = number_of_elements - 1; k >= 0; --k)
	      {
                SimTrackerHit *TrackerHit = static_cast<SimTrackerHit*>(collection->getElementAt(k));
                const float _time_of_flight = time_of_flight(TrackerHit->getPosition()[0], TrackerHit->getPosition()[1], TrackerHit->getPosition()[2]);
                if (!((TrackerHit->getTime() > (this_start + _time_of_flight)) && (TrackerHit->getTime() < (this_stop + _time_of_flight))))
		  {
                    collection ->removeElementAt(k);
                    delete TrackerHit;
		  }
	      }
	  }
        else if (collection->getTypeName() == LCIO::SIMCALORIMETERHIT)
	  {
            //we count from top to bottom, in order not to get confused when removing and adding elements!
            for (int i =  number_of_elements - 1; i >= 0; --i) 
	      {
                SimCalorimeterHit *CalorimeterHit = static_cast<SimCalorimeterHit*>(collection->getElementAt(i));
                int not_within_time_window = 0;

                // check whether all entries are within the time window
                const float _time_of_flight = time_of_flight(CalorimeterHit->getPosition()[0], CalorimeterHit->getPosition()[1], CalorimeterHit->getPosition()[2]);

                for (int j = 0; j < CalorimeterHit->getNMCContributions(); ++j)
		  {
                    //we need to shift the time window to account for the time of flight of the particle...
                    if (!((CalorimeterHit->getTimeCont(j) > (this_start + _time_of_flight)) && (CalorimeterHit->getTimeCont(j) < (this_stop + _time_of_flight))))
		      {
                        ++ not_within_time_window;
                        //std::cout << " calo hit : " << j << " Time : " << CalorimeterHit->getTimeCont(j) << " ?> " << this_start + _time_of_flight << " ?< " << this_stop + _time_of_flight << std::endl; 
		      }
		  }

                //if one and not all MC contribution is not within the time window....
                if (not_within_time_window == 0)
		  {
		    collDestMap[currentDest].insert(DestMap::value_type(cellID2long(CalorimeterHit->getCellID0(), CalorimeterHit->getCellID1()), CalorimeterHit));
		  }
                else if ((not_within_time_window > 0) && (not_within_time_window < CalorimeterHit->getNMCContributions()))
		  {
                    SimCalorimeterHitImpl *newCalorimeterHit = new SimCalorimeterHitImpl();

                    for (int j = 0; j < CalorimeterHit->getNMCContributions(); ++j)
		      {
                        if ((CalorimeterHit->getTimeCont(j) > (this_start + _time_of_flight)) && (CalorimeterHit->getTimeCont(j) < (this_stop + _time_of_flight)))
			  {
                            newCalorimeterHit->addMCParticleContribution(CalorimeterHit->getParticleCont(j), CalorimeterHit->getEnergyCont(j), CalorimeterHit->getTimeCont(j));
			  }
		      }

                    newCalorimeterHit->setCellID0(CalorimeterHit->getCellID0());
                    newCalorimeterHit->setCellID1(CalorimeterHit->getCellID1());
                    float ort[3] = {CalorimeterHit->getPosition()[0], CalorimeterHit->getPosition()[1], CalorimeterHit->getPosition()[2]};
                    newCalorimeterHit->setPosition (ort);

                    collection->removeElementAt(i);
                    delete CalorimeterHit;

                    collection->addElement(newCalorimeterHit);
		    collDestMap[currentDest].insert(DestMap::value_type(cellID2long(newCalorimeterHit->getCellID0(), newCalorimeterHit->getCellID1()), newCalorimeterHit));
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

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::merge_collections(EVENT::LCCollection *source_collection, EVENT::LCCollection *dest_collection, float time_offset)
  {
    // first, calculate the integration time, depending on the subdetector
    // time offset is the time of the physics event, after the start of the bunch train
    // adding the time offset shall move the background event relative to the physics event...
    const int number_of_elements = source_collection->getNumberOfElements();
    int mergedN = 0;
    streamlog_out(DEBUG) << "We are starting the merge with " << dest_collection->getNumberOfElements() << std::endl;
    if (number_of_elements > 0)
      {
        if (source_collection->getTypeName() == LCIO::MCPARTICLE)
	  {
            for (int i = number_of_elements - 1; i >= 0; --i)
	      {
                MCParticleImpl *MC_Part = static_cast<MCParticleImpl*>(source_collection->getElementAt(i));
                MC_Part->setTime(MC_Part->getTime() + time_offset);
                dest_collection->addElement(MC_Part);
                source_collection->removeElementAt(i);
	      }
	  }
        else if ((source_collection->getTypeName() == LCIO::SIMTRACKERHIT) && ((std::fabs(time_offset) < std::numeric_limits<float>::epsilon()) || !TPC_hits))
	  {
            for (int k = number_of_elements - 1; k >= 0; --k)
	      {
                SimTrackerHitImpl *TrackerHit = static_cast<SimTrackerHitImpl*>(source_collection->getElementAt(k));
                const float _time_of_flight = time_of_flight(TrackerHit->getPosition()[0], TrackerHit->getPosition()[1], TrackerHit->getPosition()[2]);

                if (((TrackerHit->getTime() + time_offset) > (this_start + _time_of_flight)) && ((TrackerHit->getTime() + time_offset) < (this_stop + _time_of_flight)))
		  {
                    TrackerHit->setTime( TrackerHit->getTime() + time_offset);
                    dest_collection->addElement(TrackerHit);
                    source_collection->removeElementAt(k);
		  }
	      }
	  }
        else if ((source_collection->getTypeName() == LCIO::SIMTRACKERHIT) && TPC_hits)
	  {
            for (int k = number_of_elements - 1; k >= 0; --k) 
	      {
                SimTrackerHitImpl *TrackerHit = static_cast<SimTrackerHitImpl*>(source_collection->getElementAt (k));
                const float _time_of_flight = time_of_flight(TrackerHit->getPosition()[0], TrackerHit->getPosition()[1], TrackerHit->getPosition()[2]);

                if (((TrackerHit->getTime() + time_offset) > (this_start + _time_of_flight)) && ((TrackerHit->getTime() + time_offset) < (this_stop + _time_of_flight)))
		  {
                    TrackerHit->setTime(TrackerHit->getTime() + time_offset);
                    double ort[3] = {TrackerHit->getPosition()[0], TrackerHit->getPosition()[1], 0};
                    if (TrackerHit->getPosition()[2] <= 0.)
		      {
                        ort[2] = TrackerHit->getPosition()[2] - time_offset * _tpcVdrift_mm_ns;
		      }
                    else
		      {
                        ort[2] = TrackerHit->getPosition()[2] + time_offset * _tpcVdrift_mm_ns;
		      }
                    TrackerHit->setPosition(ort);
                    dest_collection->addElement(TrackerHit);
                    source_collection->removeElementAt(k);
		  }
	      }
	  }
        else if (source_collection->getTypeName() == LCIO::SIMCALORIMETERHIT)
	  {
            // create a map of dest Collection
            for (int k =  number_of_elements - 1; k >= 0; --k) 
	      {
                SimCalorimeterHit *CalorimeterHit = static_cast<SimCalorimeterHit*>(source_collection->getElementAt(k));
                const float _time_of_flight = time_of_flight(CalorimeterHit->getPosition()[0], CalorimeterHit->getPosition()[1], CalorimeterHit->getPosition()[2]);

                //check whether there is already a hit at this position
                const unsigned long long lookfor = cellID2long(CalorimeterHit->getCellID0(), CalorimeterHit->getCellID1());
                DestMap::const_iterator destMapIt = collDestMap[currentDest].find(lookfor);
                if (destMapIt == collDestMap[currentDest].end())
		  {
                    // There is no Hit at this position -- the new hit can be added, if it is not outside the window
                    SimCalorimeterHitImpl *newCalorimeterHit = new SimCalorimeterHitImpl();
                    bool add_Hit = false;

                    for (int j = 0; j < CalorimeterHit->getNMCContributions(); ++j)
		      {
                        if (((CalorimeterHit->getTimeCont(j) + time_offset) > (this_start + _time_of_flight)) && ((CalorimeterHit->getTimeCont(j) + time_offset) < (this_stop + _time_of_flight)))
			  {
                            add_Hit = true;
                            newCalorimeterHit->addMCParticleContribution(CalorimeterHit->getParticleCont(j), CalorimeterHit->getEnergyCont(j), CalorimeterHit->getTimeCont(j) + time_offset);
			  }
		      }
                    if (add_Hit)
		      {
                        newCalorimeterHit->setCellID0(CalorimeterHit->getCellID0());
                        newCalorimeterHit->setCellID1(CalorimeterHit->getCellID1());
                        float ort[3] = {CalorimeterHit->getPosition()[0],CalorimeterHit->getPosition()[1], CalorimeterHit->getPosition()[2]};
                        newCalorimeterHit->setPosition(ort);
                        dest_collection->addElement(newCalorimeterHit);
			collDestMap[currentDest].insert(DestMap::value_type(cellID2long(newCalorimeterHit->getCellID0(), newCalorimeterHit->getCellID1()), newCalorimeterHit));
		      }
                    else
		      {
                        delete newCalorimeterHit;
		      }
		  }
                else
		  {
		    // there is already a hit at this position.... 
		    SimCalorimeterHitImpl *newCalorimeterHit = static_cast <SimCalorimeterHitImpl*>(destMapIt->second);
		    ++mergedN;
		    if((newCalorimeterHit->getPosition()[0]-CalorimeterHit->getPosition()[0])*
		       (newCalorimeterHit->getPosition()[0]-CalorimeterHit->getPosition()[0])+
		       (newCalorimeterHit->getPosition()[1]-CalorimeterHit->getPosition()[1])*
		       (newCalorimeterHit->getPosition()[1]-CalorimeterHit->getPosition()[1])+
		       (newCalorimeterHit->getPosition()[2]-CalorimeterHit->getPosition()[2])*
		       (newCalorimeterHit->getPosition()[2]-CalorimeterHit->getPosition()[2]) > 10) {
                      streamlog_out(ERROR) << "HITS DO NOT MATCH in " << currentDest << "!!!" << std::endl;
		      streamlog_out(ERROR) << "X New  " << newCalorimeterHit->getPosition()[0] 
					   << "  Old  " << CalorimeterHit->getPosition()[0] << std::endl;
		      streamlog_out(ERROR) << "Y New  " << newCalorimeterHit->getPosition()[1] 
					   << "  Old  " << CalorimeterHit->getPosition()[1] << std::endl;
		      streamlog_out(ERROR) << "Z New  " << newCalorimeterHit->getPosition()[2] 
					   << "  Old  " << CalorimeterHit->getPosition()[2] << std::endl;
		      streamlog_out(ERROR) << "ID0New  " << newCalorimeterHit->getCellID0() 
					   << "   Old  " << CalorimeterHit->getCellID0() << std::endl;
		      streamlog_out(ERROR) << "ID1New  " << newCalorimeterHit->getCellID1() 
					   << "   Old  " << CalorimeterHit->getCellID1() << std::endl;
		    
		    
		      //		      std::exit(1);
		      streamlog_out(ERROR) << "ID1New  " << cellID2long(newCalorimeterHit->getCellID0(),
                                                                        newCalorimeterHit->getCellID1())
                                           << "   Old  " << cellID2long(CalorimeterHit->getCellID0(),
                                                                        CalorimeterHit->getCellID1())
                                           << std::endl;

		    }
		    for (int j = 0; j < CalorimeterHit->getNMCContributions(); ++j)
		      {
                        if (((CalorimeterHit->getTimeCont(j) + time_offset) > (this_start + _time_of_flight)) && ((CalorimeterHit->getTimeCont(j) + time_offset) < (this_stop + _time_of_flight)))
			  {
                            newCalorimeterHit->addMCParticleContribution(CalorimeterHit->getParticleCont(j), CalorimeterHit->getEnergyCont(j), CalorimeterHit->getTimeCont(j) + time_offset);
			  }
		      }
		  }
	      }
	  }
      }
    streamlog_out(DEBUG) << "We are ending the merge with " << dest_collection->getNumberOfElements() 
			 << " and we merged " << mergedN << "  others  "
			 << std::endl;

  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::check(EVENT::LCEvent* )
  {

  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void OverlayTiming::end()
  {
    delete overlay_Eventfile_reader;
    overlay_Eventfile_reader = nullptr;
  }


} // namespace
