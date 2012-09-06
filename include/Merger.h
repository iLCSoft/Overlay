#include "lcio.h"
// #include "IMPL/SimCalorimeterHitImpl.h"
// #include "IMPL/SimTrackerHitImpl.h" 
//#include "EVENT/LCEvent.h" 

namespace EVENT{
  class LCEvent ;
  class LCCollection ;
}

namespace overlay {

  /**Basic utility to merge two events or collections. So far only 
   * simulation collections are supported.
   * 
   * @author N. Chiapolini, DESY
   * @version $Id: Merger.h,v 1.5 2007-09-14 23:52:46 chiapoli Exp $
   */
  class Merger{
    
  public:
    
    /** Tries to merge collections with a name present in both events (like merge(EVENT::LCEvent*, EVENT::LCEvent*)
     * but the MC particle collection in srcEvent is merged with 
     * the collection named mcDestString. <br>
     * 
     * @param srcEvent source event.
     * @param destEvent destination event
     * @param mcDestString name of the collection that the MCPARTICLE collection should be merged with. If more then one 
     * collection of type MCPARTICLE exists in srcEvent the function exits without any action.
     * 
     * calles mergeMC(EVENT::LCEvent*, std::string, EVENT::LCEvent*, std::string) internally
     */
    static void mergeMC(EVENT::LCEvent* srcEvent, EVENT::LCEvent* destEvent, std::string mcDestString);
      
    /** merge function, takes two events and tries to merge
     * collections with a name present in both events.<br>
     *
     * @param srcEvent source event.
     * @param destEvent destination event
     * @param mcSrcString The MC particle collection in srcEvent 
     * @param mcDestString The MC particle collection the source particles should be addet to. If this collection 
     * does not exist, a new collection is created.<br>
     * 
     * calles  merge(EVENT::LCEvent*, EVENT::LCEvent*) internally (after 
     * merging the MC collections and removing mcSrcString 
     * from the srcEvent)
     */
    static void mergeMC(EVENT::LCEvent* srcEvent, std::string mcSrcString, EVENT::LCEvent* destEvent, std::string mcDestString);
        
    /** Tries to merge collections with a name present in both events.
     * 
     * @param srcEvent source event.
     * @param destEvent destination event
     * 
     * calles  merge(EVENT::LCCollection*, EVENT::LCCollection*) internally
     */
    static void merge(EVENT::LCEvent* srcEvent, EVENT::LCEvent* destEvent);
        
    /** Merges the collections of the two events according to a given map<br> 
     * 
     * @param srcEvent source event.
     * @param destEvent destination event
     * @param *mergeMap Map containing the src->dest association for the collection names <br>
     * Map structure: (srcColName, destColName)<br>
     * If srcCol does not exist, the pair will be ignored, 
     * if destCol does not exist, a new collection with the 
     * same type as srcCol will be created.<br>
     * 
     * calles  merge(EVENT::LCCollection*, EVENT::LCCollection*) internally
     */
    static void merge(EVENT::LCEvent *srcEvent, EVENT::LCEvent *destEvent, std::map<std::string, std::string> *mergeMap);

    /** Merges the two named collections in the given events 
     * 
     * @param srcEvent source event.
     * @param srcString name of the source collection
     * @param destEvent destination event
     * @param destString name of the destination collection 
     * 
     * calles  merge(EVENT::LCCollection*, EVENT::LCCollection*) internally
     */
    static void merge(EVENT::LCEvent* srcEvent, std::string srcString, EVENT::LCEvent* destEvent, std::string destString);
    
    /** merge function, takes two collections and addes the elements
     * from src to dest. Both collections need to have same type!<br>
     * Types merged:
     *  - MCPARTICLE
     *  - SIMTRACKERHIT
     *  - SIMCALORIMETERHIT
     *  - TRACKERHIT
     *  - CALORIMETERHIT
     * 
     * Algorithm:
     * MCPARTICLE, SIMTRACKERHIT, TRACKERHIT: All Hits from the source 
     * collection are copied into the destination collection.
     * SIMCALORIMETERHIT, CALORIMETERHIT}: If the destination collection 
     * contains a hit with the same cellID, the energy of the source hit 
     * will be added to it. Otherwiese the hit will be copied into the 
     * destination collection. (In case of simulated data the MCParticle 
     * contributions will be preserved.)
     * 
     * !! It is the callers responsability to make sure the mcParticles
     * pointed to by the hits do exist !!<br>
     * 
     * @param src Collection containing the entries that should be added to another collection.
     * @param src Collection to which the new entries should be added.
     */
    static void merge(EVENT::LCCollection* src, EVENT::LCCollection* dest);

        
  protected:
    /** combines two 32 bit integers into a 64 bit long long of 
     * the form  {id0}{id1}.<br>
     * Used to combine cellID0 and cellID1 for comparison of hits.
     */
    //         inline long long cellID2long(int id0, int id1);

  }; // class

} // namespace 
