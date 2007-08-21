#include "lcio.h"
#include "IMPL/SimCalorimeterHitImpl.h"
#include "IMPL/SimTrackerHitImpl.h" 
#include "IMPL/LCEventImpl.h" 

// #include <cstdlib>

using namespace EVENT;

  /**Basic utility for merging two collections. So far only 
   * simulation collections are supported.
   * 
   * @author chiapolini
   * @version $Id: Merger.h,v 1.1 2007-08-21 14:34:32 chiapoli Exp $
   */
  class Merger{
    
    public:
    
        /** merge function, takes two events and tries to merge
         * collections with a name present in both events.<br>
         * 
         * The MC particle collection in srcEvent is merged with 
         * the collection named mcDestString. <br>
         * If more then one collection of type MCPARTICLE esists 
         * in srcEvent the function returns 0 and exits. Otherwise
         * it returns the number of collections merged.<br>
         * 
         * calles mergeMC(LCEvent*, string, LCEvent*, string) internally
         */
        static void mergeMC(LCEvent* srcEvent, LCEvent* destEvent, std::string mcDestString);
      
        /** merge function, takes two events and tries to merge
         * collections with a name present in both events.<br>
         * 
         * The MC particle collection mcSrcString is merged with 
         * the collection named mcDestString. If this collection 
         * does not exist yet, a new collection is created.<br>
         * 
         * returns the number of merged collections.<br>
         * 
         * calles  merge(LCEvent*, LCEvent*) internally (after 
         * merging the MC collections and removing mcSrcString 
         * from the srcEvent)
         */
        static void mergeMC(LCEvent* srcEvent, std::string mcSrcString, LCEvent* destEvent, std::string mcDestString);
        
        /** merge function, takes two events and tries to merge
         * collections with a name present in both events.<br>
         * calles  merge(LCCollection*, LCCollection*) internally
         */
        static void merge(LCEvent* srcEvent, LCEvent* destEvent);
        
        /** merge function, Merges the collections of the two 
         * events according to the map<br> 
         * Map structure: (srcColName, destColName)<br>
         * If srcCol does not exist, pair will be ignored, if destCol 
         * does not exist, a new collection with the same type as 
         * srcCol will be created.<br>
         * calles  merge(LCCollection*, LCCollection*) internally
         */
        static void merge(LCEvent *srcEvent, LCEvent *destEvent, std::map<std::string, std::string> *mergeMap);

        /** merge function, takes two events and the names of the 
         * collections to merge<br>
         * calles  merge(LCCollection*, LCCollection*) internally
         */
        static void merge(LCEvent* srcEvent, std::string srcString, LCEvent* destEvent, std::string destString);
    
        /** merge function, takes two collections and addes the Elements
         * from src to dest. Both collections need to have same type!<br>
         * Types merged:
         *  - MCPARTICLE
         *  - SIMTRACKERHIT
         *  - SIMCALORIMETERHIT
         * 
         * !! It is the callers responsability to make sure the mcParticles
         * pointed to by the hits do exist !!<br>
         * Returns 1 if error occured, 0 if terminated normally
         */
        static void merge(LCCollection* src, LCCollection* dest);

        
    protected:
        /** combines two 32 bit integers into a 64 bit long long of 
         * the form  {id0}{id1}.<br>
         * Used to combine cellID0 and cellID1 for comparison of hits.
         */
//         inline long long cellID2long(int id0, int id1);

  }; // class
