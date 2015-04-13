#ifndef CHEETAH_ANA_PKG_CHEETAH_ANA_MOD_H
#define CHEETAH_ANA_PKG_CHEETAH_ANA_MOD_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class cheetah_ana_mod.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>
#include <queue>
#include <semaphore.h>

#include <cheetah.h>

//----------------------
// Base Class Headers --
//----------------------
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include "psana/Module.h"
#pragma GCC diagnostic warning "-Wmissing-field-initializers"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wignored-qualifiers"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
class cEventData;
//		---------------------
// 		-- Class Interface --
//		---------------------

namespace cheetah_ana_pkg {
    /// @addtogroup cheetah_ana_pkg
    extern cGlobal cheetahGlobal;
    /**
     *  @ingroup cheetah_ana_pkg
     *
     *  @brief Example module class for psana
     *
     *  This software was developed for the LCLS project.  If you use all or
     *  part of it, please give an appropriate acknowledgment.
     *
     *  @version \$Id$
     *
     *  @author Chunhong Yoon
     */
    
    class cheetah_ana_mod : public Module {
    public:
        
        // Default constructor
        cheetah_ana_mod (const std::string& name) ;
        
        // Destructor
        virtual ~cheetah_ana_mod () ;
        
        /// Method which is called once at the beginning of the job
        virtual void beginJob(Event& evt, Env& env);
        
        /// Method which is called at the beginning of the run
        virtual void beginRun(Event& evt, Env& env);
        
        /// Method which is called at the beginning of the calibration cycle
        virtual void beginCalibCycle(Event& evt, Env& env);
        
        /// Method which is called with event data, this is the only required
        /// method, all other methods are optional
        virtual void event(PSEvt::Event& evt, PSEnv::Env& env);
		void copy_event(boost::shared_ptr<Event> evt, boost::shared_ptr<Env> env);
		
        /// Method which is called at the end of the calibration cycle
        virtual void endCalibCycle(Event& evt, Env& env);
        
        /// Method which is called at the end of the run
        virtual void endRun(Event& evt, Env& env);
        
        /// Method which is called once at the end of the job
        virtual void endJob(Event& evt, Env& env);
        
    protected:
        
    private:
		int readTOF(Event & evt, Env & env,
					cEventData* eventData);
        // Event code present?
        template <typename T>
		bool eventCodePresent(const ndarray<T, 1>& array, unsigned EvrCode);


		void waitForAllWorkers();
		void waitForCheetahWorkers();
		void waitForAnaModWorkers();
		
		// Data members
		pthread_mutex_t  counting_mutex;


		std::string m_key;
		Source m_srcCspad0;
		Source m_srcCspad1;
		Source m_srcRayonix0;
		Source m_srcCspad2x2;
		Source m_srcEvr;
		Source m_srcBeam;
		Source m_srcFee;
		Source m_srcFeeSpec;
		Source m_srcTimeTool;
		Source m_srcCav;
		Source m_srcCam;
		Source m_srcSpec;
		Source m_srcPnccd0;
		Source m_srcPnccd1;
		std::vector<Source> m_srcAcq;

		time_t startT;	

        int finishedAnaThreads;
       
    };
	
	class AnaModEventData {
	public:
		cheetah_ana_mod* module;
		boost::shared_ptr<Event> evtp;
		boost::shared_ptr<Env> envp;
		
		AnaModEventData(cheetah_ana_mod* module, boost::shared_ptr<Event> evtp, boost::shared_ptr<Env> envp) :
			module(module), evtp(evtp), envp(envp) {      
		}
	};
    
    extern std::queue<pthread_t> runningThreads;
    extern volatile bool runCheetahCaller;
    extern pthread_t cheetahCallerThread;
	extern pthread_mutex_t pthread_queue_mutex;
    extern sem_t availableAnaThreads;
    void * threaded_event(void* threadData);
    void * cheetah_caller(void * threadData);
} // namespace cheetah_ana_pkg

#endif // CHEETAH_ANA_PKG_CHEETAH_ANA_MOD_H
