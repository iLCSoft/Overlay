#include "JoinEvents.h"

#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCEventImpl.h>

#include "UTIL/LCTOOLS.h"

#include <sstream>

using namespace lcio ;
using namespace marlin ;


JoinEvents aJoinEvents ;


JoinEvents::JoinEvents() : Processor("JoinEvents") {
  
  // modify processor description
  _description = "Opens an lcio file and merges corresponding (run/evet-number) events to the current event." ;
  
  
  // register steering parameters: name, description, class-variable, default value
  registerProcessorParameter( "InputFileName" , 
			      "Name of the lcio input file"  ,
			      _fileName ,
 			      std::string("joinevents.slcio") ) ;
  
}


void JoinEvents::init() { 
  
  if( ! LCIO_PATCHVERSION_GE( 1 , 8 , 4  ) ){  // FIXME - need 1.8.5 to work properly
    throw Exception("  JoinEvents requires LCIO v1.8.4 or higher \n"
  		    "  - please upgrade your LCIO version or disable the JoinEvents processor ! ") ;
  }
  if( ! LCIO_PATCHVERSION_GE( 1 , 8 , 5  ) ){  // FIXME - need 1.8.5 to work properly

    streamlog_out( WARNING ) << " with LCIO 1.8.4 JoinEvents does not work properly " << std::endl ;
  }

  // usually a good idea to
  printParameters() ;
  
  // opening input file
  _lcReader = LCFactory::getInstance()->createLCReader() ;

  streamlog_out( DEBUG ) << " opening file for joining events : " << _fileName << std::endl ;
  
  _lcReader->open( _fileName ) ; 
  
  _nRun = 0 ;
  _nEvt = 0 ;
  
}



void JoinEvents::processRunHeader( LCRunHeader* run) { 
  
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
  StringVec::const_iterator end = colNames->end() ;

  for( it = colNames->begin() ; it != end ; ++it ) {
    
    std::string name( *it ) ;


    LCCollectionVec* col = dynamic_cast<LCCollectionVec*> ( joinEvt->getCollection( name )  ) ;

    if( col != 0 ) {

      try{

	
 	joinEvt->takeCollection( name ) ; // remove the collection from the join-event 
 	col->setTransient( false ) ;

	//	joinEvt->removeCollection( name ) ;
	//	name += "_1" ;

	evt->addCollection( col , name ) ;

      } catch( EventException& e) {
	
	streamlog_out( DEBUG ) << " collection " << name 
			       << " allready exists in event - not added " << std::endl ;
	
      }

    } else {

	streamlog_out( ERROR ) << " collection " << name 
			       << " dynamic_cast<LCCollectionVec*> failed  " << std::endl ;
	
    }
  }
  
  _nEvt ++ ;
}



void JoinEvents::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void JoinEvents::end(){ 

  _lcReader->close()  ;
}

