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

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Module.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace cheetah_ana_pkg {
    
    /// @addtogroup cheetah_ana_pkg
    
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
        virtual void event(Event& evt, Env& env);
        
        /// Method which is called at the end of the calibration cycle
        virtual void endCalibCycle(Event& evt, Env& env);
        
        /// Method which is called at the end of the run
        virtual void endRun(Event& evt, Env& env);
        
        /// Method which is called once at the end of the job
        virtual void endJob(Event& evt, Env& env);
        
    protected:
        
    private:
        int readTOF(Event & evt, Env & env, int channel, double & TOFtrigtime, 
		      double* & TOFTime, double * & TOFVoltage);
        
        // Data members
        std::string m_key;
        Source m_srcCspad0;
        Source m_srcCspad1;
        Source m_srcCspad2x2;
        Source m_srcEvr;
        Source m_srcBeam;
        Source m_srcFee;
        Source m_srcFeeSpec;
        Source m_srcCav;
        Source m_srcAcq;
        Source m_srcCam;
        Source m_srcSpec;
        Source m_srcPnccd0;
        Source m_srcPnccd1;
    };
    
} // namespace cheetah_ana_pkg

#endif // CHEETAH_ANA_PKG_CHEETAH_ANA_MOD_H
