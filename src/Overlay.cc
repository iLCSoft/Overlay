#define BGNAME "expBG"

#include "Overlay.h"
#include <iostream>

#ifdef MARLIN_USE_AIDA
#include <marlin/AIDAProcessor.h>
#include <marlin/Global.h>
#include "marlin/ProcessorEventSeeder.h"

#include <AIDA/IHistogramFactory.h>
#include <AIDA/ICloud1D.h>
//#include <AIDA/IHistogram1D.h>
#endif

#include "EVENT/LCEvent.h"
#include "IO/LCReader.h"
#include "IO/LCWriter.h"
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <EVENT/MCParticle.h>
#include "IO/LCReader.h"
#include "UTIL/LCTOOLS.h"
#include "Merger.h"

#include "CLHEP/Random/RandPoisson.h"
#include "CLHEP/Random/RandFlat.h"
// #include <time.h>

using namespace lcio ;
using namespace marlin ;

namespace overlay{
  
  
  Overlay aOverlay ;
  
  
  
  Overlay::Overlay() : Processor("Overlay"), 
		       _nSkipEventsRandom(0 ) ,
		       _lcReader(0 ) ,
		       _overlayEvent(0 ) ,
		       _activeRunNumber(0 ) ,
		       _nRun(0 ) ,
		       _nEvt(0 ) ,
		       _events(0 ) {
    
    // modify processor description
    _description = "Opens a second (chain of) lcio file(s) and overlays events..." ;
  
  
    // register steering parameters: name, description, class-variable, default value
  
    StringVec files ;
    files.push_back( "overlay.slcio" )  ;
    registerProcessorParameter( "InputFileNames" , 
				"Name of the lcio input file(s)"  ,
				_fileNames ,
				files ) ;
  
    int num;
    num = 0;
    registerProcessorParameter( "NumberOverlayEvents" , 
				"Overlay each event with this number of background events. (default 0)" ,
				_numOverlay ,
				num ) ;
  
    bool runOverlay;
    runOverlay = false;
    registerProcessorParameter( "runOverlay" , 
				"Overlay each event with the content of one run." ,
				_runOverlay ,
				runOverlay ) ;
 
 
    double exp;
    exp = 1.;
    registerProcessorParameter( BGNAME , 
				"Add additional background events according to a poisson distribution with this expectation value. (non, if parameter not set)" ,
				_expBG ,
				exp ) ;
    
    
    registerProcessorParameter( "NSkipEventsRandom" ,
				"Maximum number of events to skip between overlayd events (choosen from flat intervall [0,NSkipEventsRandom] )" ,
				_nSkipEventsRandom ,
				int(0) ) ;


    StringVec map;
    map.push_back( "mcParticles mcParticlesBG" );
    //   map.push_back( "" );
    registerProcessorParameter( "CollectionMap" , 
				"Pairs of collection to be merged"  ,
				_colVec ,
				map ) ;
  




  }



  void Overlay::init() { 
  

    if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
      throw Exception("  Overlay requires LCIO v1.4 or higher \n"
		      "  - please upgrade your LCIO version or disable the Overlay processor ! ") ;
    }

    // usually a good idea to
    printParameters() ;
  
    // opening background input
    _lcReader = LCFactory::getInstance()->createLCReader( LCReader::directAccess ) ;

    _lcReader->open( _fileNames  ) ; 

    streamlog_out( DEBUG6 ) << "*** opened files for overlay - first file : " << _fileNames[0] 
			    << "    has " << _lcReader->getNumberOfEvents() << "  events. "  << std::endl ;
    
    
    if( _fileNames.size() == 1 ){

      // if we have only one file, we can use direct access to random events
      _lcReader->getEvents( _events ) ;

      streamlog_out( DEBUG6 ) << "    will use direct access from " << _events.size() / 2 << "  events. "  << std::endl ;
    }


    // initalisation of random number generator
    Global::EVENTSEEDER->registerProcessor(this);
    //  CLHEP::HepRandom::setTheSeed(time(NULL));


    // preparing collection map for merge
    StringVec::iterator it;
    StringVec::iterator endIt = _colVec.end();
  
    int oddNumOfCols = _colVec.size() & 0x1;
    if (oddNumOfCols) { 
      streamlog_out( WARNING ) << "Odd number of collection names, last collection ignored." << std::endl;
      --endIt;
    }
  
    for (it=_colVec.begin(); it < endIt; ++it) {  // treating pairs of collection names
      std::string  key = (*it);  //src
      ++it;
      _colMap[key] = (*it);// src -> dest
    }


    // handle runOverlay parameter
    if (_runOverlay) {
      _numOverlay = 1;
      if (parameterSet(BGNAME) || parameterSet("NumberOverlayEvents")) {
	streamlog_out( WARNING4 ) << "Using runOverlay, NumberOverlayEvents and expBG get ignored." << std::endl;
      }
    }

  
  
    _nRun = 0 ;
    _nEvt = 0 ;
  
  }



  void Overlay::processRunHeader( LCRunHeader* run) { 
  
    _nRun++ ;
  } 




  void Overlay::modifyEvent( LCEvent * evt ) {

    // initalisation of random number generator
    int eventSeed = Global::EVENTSEEDER->getSeed(this);


    CLHEP::HepRandom::setTheSeed( eventSeed  );


    long num = _numOverlay;
  
    if ( parameterSet(BGNAME) && !_runOverlay ) {
      num += CLHEP::RandPoisson::shoot( _expBG );
    }


    streamlog_out( DEBUG6 ) << "** Processing event nr " << evt->getEventNumber() << " run " <<  evt->getRunNumber() 
     			    << "\n**  overlaying " << num << " background events. \n " 
			    << " ( seeded CLHEP::HepRandom with seed = " << eventSeed  << ") " 
			    << std::endl;
  

    // // initialise static event storage overlay Event
    // if( isFirstEvent() ) {
    //   int nSkip = CLHEP::RandFlat::shoot( double(_nSkipEventsRandom ) ) ;
    //   streamlog_out( DEBUG5 ) << "   will skip " << nSkip << " events ... " << std::endl ;
    //   _lcReader->skipNEvents( nSkip ) ;
    //   _overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
    //   _activeRunNumber = _overlayEvent->getRunNumber();
    // } 
  

    // core - add correct number of bg events to EVT
    // if runs are added, the loop counter  will be 
    // reset to zero in every pass
    for(long i=0; i < num  ; i++ ) {

      if( _events.size() == 0 ) {   // in order to be reproduceable, we have to reset the file stream when using the skip mechanism

	_lcReader->close() ; 
	_lcReader->open( _fileNames  ) ; 
      }

      _overlayEvent = readNextEvent() ;

      if( !_overlayEvent ) {

	streamlog_out( ERROR ) << "loop: " << i << " ++++++++++ Nothing to overlay +++++++++++ \n " ;

	continue ;
      } 

      streamlog_out( DEBUG6 ) << "loop: " << i << " will overlay event " << _overlayEvent->getEventNumber() << " - run " << _overlayEvent->getRunNumber() << std::endl ;

      // merge event from storage with EVT

      if ( _colMap.empty() ) {
	
	Merger::merge( _overlayEvent, evt );
	
      } else {
	
	Merger::merge( _overlayEvent, evt, &_colMap );
      }

      // //      int nSkip = CLHEP::RandPoisson::shoot(  double(_nSkipEventsRandom ) ) ;  
      // int nSkip = CLHEP::RandFlat::shoot(  double(_nSkipEventsRandom ) ) ; 
      // streamlog_out( DEBUG5 ) << "   will skip " << nSkip << " events ... " << std::endl ;
      // _lcReader->skipNEvents( nSkip ) ;
      // // load new event into storage, restart input "stream" if necessary
      // _overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;  
      
      // if( !_overlayEvent ) {
      // 	_lcReader->close() ; 
      // 	_lcReader->open( _fileNames  ) ; 
      // 	streamlog_out( WARNING4 ) << "Overlay stream has been reset to first element."
      // 				  << std::endl ;
      // 	_overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
      // 	_activeRunNumber = _overlayEvent->getRunNumber();
      // 	i = num;
      // }


      // if processing runs: 
      // check exit criteria and reset loop counter if necessary
      if ( _runOverlay ) {
	i=0;
	int runNumber = _overlayEvent->getRunNumber();
	if (runNumber != _activeRunNumber) {
	  _activeRunNumber = runNumber;
	  i=num;
	}
      }
    }

  
#ifdef MARLIN_USE_AIDA
#endif
    _nEvt ++ ;
  }



  void Overlay::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
  }


  void Overlay::end(){ 
  }


  //===========================================================================================================================

  LCEvent* Overlay::readNextEvent() {
    
    LCEvent* overlayEvent = 0 ;
    
    // if we are reading from more than one file, we need to use the skipNEvents method,
    // otherwise we can directly access a random event
    
    int nEvts = _events.size() / 2 ;
    
    streamlog_out( DEBUG3 ) << "  --- Overlay::readNextEvent()  ---- will use "  << ( nEvts ?  " direct access " : " skipNEvents " )    << std::endl ;
    
    if( nEvts > 0 ){
      
      int iEvt = CLHEP::RandFlat::shoot( nEvts  ) ;

      int runNum = _events[ 2 * iEvt     ] ;
      int evtNum = _events[ 2 * iEvt + 1 ] ;

      streamlog_out( DEBUG3 ) << "   will read event " << evtNum << "  from  run " <<  runNum << std::endl ;
      
      overlayEvent = _lcReader->readEvent(  runNum , evtNum , LCIO::UPDATE ) ;

      if( !overlayEvent ) {

	streamlog_out( ERROR ) << "   +++++++++ could not read event " << evtNum << "  from  run " <<  runNum << std::endl ;
	return 0 ;
      }

    } else { 
      
      int nSkip = CLHEP::RandFlat::shoot( double(_nSkipEventsRandom ) ) ;
      
      streamlog_out( DEBUG3 ) << "   will skip " << nSkip << " events ... " << std::endl ;
      
      _lcReader->skipNEvents( nSkip ) ;
      
      overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
      
      if( !overlayEvent ) {
	
	_lcReader->close() ; 
	_lcReader->open( _fileNames  ) ; 

	streamlog_out( DEBUG3 ) << "Overlay stream has been reset to first element." << std::endl ;
	
	overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
      }
    }
    _activeRunNumber = overlayEvent->getRunNumber();
    
    return overlayEvent ;
  }
  

} // end namespace 
