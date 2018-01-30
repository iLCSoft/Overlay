#include "JoinEvents.h"

#include <EVENT/LCCollection.h>
#include <EVENT/LCEvent.h>

//#include "UTIL/LCTOOLS.h"

//#include <sstream>

using namespace lcio ;
using namespace marlin ;


namespace overlay {

  JoinEvents aJoinEvents ;


  JoinEvents::JoinEvents() : Processor("JoinEvents") {
  
    // modify processor description
    _description = "Opens an lcio file and adds all collections of the corresponding "
      "(run/event-number) events to the current event." ;
  
  
    // register steering parameters: name, description, class-variable, default value
    registerProcessorParameter( "InputFileName" , 
				"Name of the lcio input file"  ,
				_fileName ,
				std::string("joinevents.slcio") ) ;
  
    registerOptionalParameter( "ColNamePostFix" , 
			       "post fix appended to all collection names"  ,
			       _postFix ,
			       std::string("_1") ) ;
  
  }


  void JoinEvents::init() { 
  
    if( ! LCIO_PATCHVERSION_GE( 1 , 8 , 5  ) ){  
      throw Exception("  JoinEvents requires LCIO v1.8.5 or higher \n"
		      "  - please upgrade your LCIO version or disable the JoinEvents processor ! ") ;
    }

    // usually a good idea to
    printParameters() ;
  
    // opening input file
    _lcReader = LCFactory::getInstance()->createLCReader( LCReader::directAccess ) ;

    streamlog_out( DEBUG ) << " opening file for joining events : " << _fileName << std::endl ;
  
    _lcReader->open( _fileName ) ; 
  
    _nRun = 0 ;
    _nEvt = 0 ;
  
  }



  void JoinEvents::processRunHeader( LCRunHeader* ) {
  
    _nRun++ ;
  } 




  void JoinEvents::processEvent( LCEvent * evt ) {


    streamlog_out( DEBUG ) << " Processing event " 
			   << evt->getRunNumber() << " " 
			   << evt->getEventNumber() 
			   << std::endl ;
  
  
    //   LCEvent* jEvt = _lcReader->readEvent( evt->getRunNumber() , evt->getEventNumber()   ) ;
    //   LCEventImpl* joinEvt = dynamic_cast<LCEventImpl*>( jEvt ) ;

    LCEvent* joinEvt = _lcReader->readEvent( evt->getRunNumber() , evt->getEventNumber()   ) ;

    if( joinEvt == 0 ) {
    
      streamlog_out( WARNING ) <<  " Event not found: " 
			       << evt->getRunNumber()   << " " 
			       << evt->getEventNumber() << std::endl ;
      return ;
    }
  
  
    // join the events - add all collections to the current event

    const StringVec* colNames = joinEvt->getCollectionNames()  ;

    StringVec::const_iterator it ;
    StringVec::const_iterator endIter = colNames->end() ;

    for( it = colNames->begin() ; it != endIter ; ++it ) {
    
      std::string collectionName( *it ) ;


      LCCollection* col =  joinEvt->getCollection( collectionName )  ;

      if( col != 0 ) {

	try{

	
	  if( parameterSet( "ColNamePostFix" ) ) {
	    collectionName += _postFix ;
	  }

	  evt->addCollection( col , collectionName ) ;

	  joinEvt->takeCollection( *it ) ; // remove the collection from the join-event 

	  int flag = col->getFlag() ;
	  flag &= ~( 1 << LCCollection::BITTransient ) ; 
	  col->setFlag(flag) ;

	  //fg:  --- equivalent to setTransient( false ) which really should be part of the LCCollection interface...
	  // 	col->setTransient( false ) ;


	} catch( EventException& e) {
	
	  streamlog_out( MESSAGE ) << " collection " << collectionName 
				   << " allready exists in event " 
				   << evt->getRunNumber()   << " " 
				   << evt->getEventNumber() << " - not added " << std::endl ;
	
	}

      } else {

	streamlog_out( ERROR ) << " collection " << collectionName 
			       << " dynamic_cast<LCCollectionVec*> failed  " << std::endl ;
	
      }
    }
  
    _nEvt ++ ;
  }



  void JoinEvents::check( LCEvent * ) {
    // nothing to check here - could be used to fill checkplots in reconstruction processor
  }


  void JoinEvents::end(){ 

    _lcReader->close()  ;
  }

} // namespace 
