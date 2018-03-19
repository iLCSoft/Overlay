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

namespace overlay {
  
  Overlay aOverlay ;
  
  /// Set the lcio file name
  void LCFileHandler::setFileName(const std::string& fname) {
    _fileName = fname ;
  }
  
  //===========================================================================================================================
  
  /// Get the number of events available in the file
  unsigned int LCFileHandler::getNumberOfEvents() const {
    openFile() ;
    return _lcReader->getNumberOfEvents() ;
  }
  
  //===========================================================================================================================
  
  /// Get the event number at the specified index (look in the event map)
  unsigned int LCFileHandler::getEventNumber(unsigned int index) const {
    openFile() ;
    return _eventMap.at( index * 2 + 1 ) ;
  }
  
  //===========================================================================================================================
  
  /// Get the run number at the specified index (look in the event map)
  unsigned int LCFileHandler::getRunNumber(unsigned int index) const {
    openFile() ;
    return _eventMap.at( index * 2 ) ;
  }
  
  //===========================================================================================================================
  
  /// Read the specified event, by run and event number
  EVENT::LCEvent* LCFileHandler::readEvent(int runNumber, int eventNumber) {
    openFile();
    streamlog_out( DEBUG6 ) << "*** Reading event from file : '" << _fileName 
          << "',  event number " << eventNumber << " of run " << runNumber << "." << std::endl ;
    return _lcReader->readEvent( runNumber, eventNumber, LCIO::UPDATE );
  }
  
  //===========================================================================================================================
  
  /// Proxy method to open the LCIO file
  void LCFileHandler::openFile() const {
    if(nullptr == _lcReader) {
      _lcReader = std::shared_ptr<IO::LCReader>( LCFactory::getInstance()->createLCReader( LCReader::directAccess ) ) ;
      
      streamlog_out( MESSAGE ) << "*** Opening file for overlay, file name:" << _fileName << std::endl ;
            
      _lcReader->open( _fileName ) ;
      _lcReader->getEvents( _eventMap ) ;
      
      streamlog_out( MESSAGE ) << "*** Opening file for overlay : number of available events: " << _lcReader->getNumberOfEvents() << std::endl ;
    }
  }

  //===========================================================================================================================
  //===========================================================================================================================
  
  Overlay::Overlay() : Processor("Overlay") {
    
    // modify processor description
    _description = "Opens a second (chain of) lcio file(s) and overlays events..." ;
  
    // register steering parameters: name, description, class-variable, default value
    registerProcessorParameter( "InputFileNames" , 
				"Name of the lcio input file(s)"  ,
				_fileNames ,
				StringVec( {"undefined.slcio"} ) ) ;
  
    registerProcessorParameter( "NumberOverlayEvents" , 
				"Overlay each event with this number of background events. (default 0)" ,
				_numOverlay ,
				static_cast<int>(0) ) ; 
 
    registerProcessorParameter( "expBG" , 
				"Add additional background events according to a poisson distribution with this expectation value. (non, if parameter not set)" ,
				_expBG ,
				static_cast<double>(1.) ) ;
    
    registerProcessorParameter( "CollectionMap" , 
				"Pairs of collection to be merged"  ,
				_overlayCollections ,
				StringVec( {"MCParticle", "MCParticle"} ) ) ;
        
    registerProcessorParameter( "ExcludeCollections" , 
        "List of collections to exclude for merging"  ,
        _excludeCollections ,
        StringVec() ) ;
  }
  
  //===========================================================================================================================
  
  const std::string & Overlay::name() const {
    return Processor::name() ;
  }

  //===========================================================================================================================

  void Overlay::init() { 
    
    if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
      throw Exception("  Overlay requires LCIO v1.4 or higher \n"
		      "  - please upgrade your LCIO version or disable the Overlay processor ! ") ;
    }

    // usually a good idea to
    printParameters() ;
    
    // prepare the lcio file handlers
    _lcFileHandlerList.resize( _fileNames.size() ) ;
    
    for ( unsigned int i=0 ; i<_fileNames.size() ; i++ )
      _lcFileHandlerList.at( i ).setFileName( _fileNames.at( i ) ) ;
  
    // initalisation of random number generator
    Global::EVENTSEEDER->registerProcessor(this) ;

    // preparing collection map for merge
    StringVec::iterator endIter = _overlayCollections.end() ;
  
    if ( _overlayCollections.size() % 2 ) { 
      streamlog_out( WARNING ) << "Odd number of collection names, last collection ignored." << std::endl ;
      --endIter ;
    }
  
    // treating pairs of collection names
    for ( auto iter =_overlayCollections.begin() ; iter != endIter ; ++iter ) {  
      std::string key = (*iter) ;
      ++iter ;
      _overlayCollectionMap[key] = (*iter) ;
    }

    _nRun = 0 ;
    _nEvt = 0 ;  
  }

  //===========================================================================================================================

  void Overlay::processRunHeader( LCRunHeader* ) {
  
    _nRun++ ;
  } 

  //===========================================================================================================================

  void Overlay::modifyEvent( LCEvent * evt ) {

    if( isFirstEvent() ) {
      // get it here and not in init as files are opened on function call
      _nAvailableEvents = getNAvailableEvents() ;
      
      streamlog_out( MESSAGE ) << "Overlay::modifyEvent: total number of available events to overlay: " << _nAvailableEvents << std::endl ;
    }

    // initalisation of random number generator
    int eventSeed = Global::EVENTSEEDER->getSeed(this);
    CLHEP::HepRandom::setTheSeed( eventSeed );


    unsigned int nEventsToOverlay = _numOverlay ;
  
    if ( parameterSet("expBG") ) {
      nEventsToOverlay += CLHEP::RandPoisson::shoot( _expBG ) ;
    }

    streamlog_out( DEBUG6 ) << "** Processing event nr " << evt->getEventNumber() << " run " <<  evt->getRunNumber() 
     			    << "\n**  overlaying " << nEventsToOverlay << " background events. \n " 
			    << " ( seeded CLHEP::HepRandom with seed = " << eventSeed  << ") " 
			    << std::endl;
  
    int nOverlaidEvents(0);
    EVENT::FloatVec overlaidEventIDs, overlaidRunIDs;
    
    for(unsigned int i=0 ; i < nEventsToOverlay ; i++ ) {

      EVENT::LCEvent *overlayEvent = readNextEvent() ;

      if( nullptr == overlayEvent ) {
	       streamlog_out( ERROR ) << "loop: " << i << " ++++++++++ Nothing to overlay +++++++++++ \n " ;
	       continue ;
      } 
      
      overlaidEventIDs.push_back( overlayEvent->getEventNumber() );
      overlaidRunIDs.push_back( overlayEvent->getRunNumber() );
      
      ++nOverlaidEvents ;

      streamlog_out( DEBUG6 ) << "loop: " << i << " will overlay event " << overlayEvent->getEventNumber() << " - run " << overlayEvent->getRunNumber() << std::endl ;

      std::map<std::string, std::string> collectionMap;
      
      if ( ( _overlayCollectionMap.empty() || ! parameterSet("CollectionMap") ) ) {
        auto collectionNames = overlayEvent->getCollectionNames();
        for ( auto collection : *collectionNames ) {
          collectionMap[collection] = collection;
          streamlog_out( DEBUG6 ) << "Collection map -> " << collection << std::endl;
        }
      }
      else {
        collectionMap = _overlayCollectionMap;
      }
      
      // Remove collections to exclude from the collection map
      if(not _excludeCollections.empty()) {
        for ( auto excludeCol : _excludeCollections ) {
          auto findIter = collectionMap.find( excludeCol );
          if( collectionMap.end() != findIter ) {
            collectionMap.erase( findIter );
          }
        }
      }

	     Merger::merge( overlayEvent, evt, &collectionMap );
    }
    
    _nTotalOverlayEvents += nOverlaidEvents;
    
    // Write info to event parameters
    std::string paramName = "Overlay." + this->name() + ".nEvents";
    evt->parameters().setValue(paramName, nOverlaidEvents);
    paramName = "Overlay." + this->name() + ".eventIDs";
    evt->parameters().setValues(paramName, overlaidEventIDs);
    paramName = "Overlay." + this->name() + ".runIDs";
    evt->parameters().setValues(paramName, overlaidRunIDs);
    
    int totalOverlay = evt->parameters().getIntVal("Overlay.nTotalEvents"); // returns 0 if the key doesn't exists
    totalOverlay += nOverlaidEvents;
    evt->parameters().setValue("Overlay.nTotalEvents", totalOverlay);

    _nEvt ++ ;
  }

  //===========================================================================================================================

  void Overlay::end() { 

    streamlog_out( MESSAGE ) << " ------------------------------------------ " 
			     << "   Overlay processor " << _nTotalOverlayEvents << " background events on " << _nEvt << " physics events.\n"
			     << "      -> mean = " << double(_nTotalOverlayEvents ) / double( _nEvt ) 
			     << "  ±  " <<   double(_nTotalOverlayEvents ) / double( _nEvt ) / sqrt( _nEvt ) << "\n"
			     << std::endl ;
  }


  //===========================================================================================================================

  EVENT::LCEvent* Overlay::readNextEvent() {
    
    EVENT::LCEvent* overlayEvent = nullptr ;
    
    // get the event index to random pick an event among the possible files
    const unsigned int eventIndex = CLHEP::RandFlat::shoot( static_cast<double>( _nAvailableEvents ) ) ;
    unsigned int currentEventIndex(0);
    
    streamlog_out( DEBUG ) << "Overlay::readNextEvent: index = " << eventIndex  << " over " << _nAvailableEvents << std::endl ;
    
    for ( auto &handler : _lcFileHandlerList ) {
      
      if ( currentEventIndex <= eventIndex && eventIndex < currentEventIndex + handler.getNumberOfEvents() ) {
        
        const int eventNumber = handler.getEventNumber( eventIndex-currentEventIndex ) ;
        const int runNumber = handler.getRunNumber( eventIndex-currentEventIndex ) ;
        
        overlayEvent = handler.readEvent( runNumber, eventNumber ) ;
        
        if( nullptr == overlayEvent ) {
          streamlog_out( ERROR ) << "Overlay::readNextEvent: Could not read event " << eventNumber << "  from  run " <<  runNumber << std::endl ;
        }
        
        break;
      }
      currentEventIndex += handler.getNumberOfEvents() ;
    }
    
    return overlayEvent ;
  }
  
  //===========================================================================================================================
  
  unsigned int Overlay::getNAvailableEvents() const
  {
    unsigned int totalNEvents(0);
    
    for ( auto &handler : _lcFileHandlerList ) {
      totalNEvents += handler.getNumberOfEvents();
    }
    
    return totalNEvents;
  }

} // end namespace 
