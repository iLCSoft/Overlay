#include "OverlayTimingGeneric.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include <marlin/Global.h>
#include <marlin/ProcessorEventSeeder.h>

OverlayTimingGeneric aOverlayTimingGeneric;

OverlayTimingGeneric::OverlayTimingGeneric(): OverlayTiming("OverlayTimingGeneric")
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
  registerProcessorParameter("Collection_IntegrationTimes",
                             "Integration times for the Collections",
                             _collectionTimesVec,
                             _collectionTimesVec);

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

    registerProcessorParameter("Start_Integration_Time",
                               "Starting integration time.  Should be shortly before the BX, but may need to be shifted earlier if the vertex is smeared in time.",
                               _DefaultStart_int,
                               float(-0.25));
}

void OverlayTimingGeneric::init()
{

  printParameters();

  overlay_Eventfile_reader = LCFactory::getInstance()->createLCReader();

  streamlog_out(WARNING) << "Attention! There are " << _inputFileNames.size()
                         << " files in the list of background files to overlay. Make sure that the total number of background events is sufficiently large for your needs!!"
                         << std::endl;

  marlin::Global::EVENTSEEDER->registerProcessor(this);

  _nRun = 0;
  _nEvt = 0;


  if( _collectionTimesVec.size() % 2 != 0 ){
    std::runtime_error( "bad entries for parameter 'Collection_Integration Times! Need pairs of collection and integration times");
  }

  // parse the collectionTimesVec vector to get the collections and integration times
  for (size_t i = 0; i < _collectionTimesVec.size(); i+=2 ) {
    _collectionIntegrationTimes[ _collectionTimesVec[i] ] = std::atof( _collectionTimesVec[i+1].c_str() );
  }

  for (auto const& entry : _collectionIntegrationTimes) {
    streamlog_out(MESSAGE) << entry.first << ": " << entry.second  << std::endl;
  }


}

//------------------------------------------------------------------------------------------------------------------------------------------

void OverlayTimingGeneric::define_time_windows( std::string const& collectionName ) {

  this_start= _DefaultStart_int; //the integration time shall start shortly before the BX
  //with the physics event to avoid timing problems the
  //value of -0.25 is a arbitrary number for the moment
  //but should be sufficient -- corresponds to 7.5cm of
  //flight at c

  this_stop= 0.0;
  TPC_hits = false;

  auto iter = _collectionIntegrationTimes.find( collectionName );
  if ( iter != _collectionIntegrationTimes.end() ) {
    this_stop = iter->second;
  } else {
    throw std::runtime_error( "Cannot find integration time for collection " + collectionName );
  }

}
