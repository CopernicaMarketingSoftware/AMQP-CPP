/**
 *  TcpSecureState.h
 *
 *  Utility class that takes care of setting a new state. It contains
 *  a number of checks that prevents that the state is overwritten
 *  if the object is destructed in the meantime
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class TcpSecureState
{
private:
    /**
     *  Monitor to check the validity of the connection
     *  @var Monitor
     */
    Monitor _monitor;

    /**
     *  Reference to the pointer to the state that should be updated
     *  @var std::unique_ptr<TcpState>
     */
    std::unique_ptr<TcpState> &_state;
    
    /**
     *  The old pointer
     *  @var TcpState*
     */
    const TcpState *_old;

public:
    /**
     *  Constructor
     *  @param  watchable       the object that can be destructor
     *  @param  state           the old state value
     */
    TcpSecureState(Watchable *watchable, std::unique_ptr<TcpState> &state) :
        _monitor(watchable), _state(state), _old(state.get()) {}
        
    /**
     *  No copying
     *  @param  that
     */
    TcpSecureState(const TcpSecureState &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~TcpSecureState() = default;
    
    /**
     *  Expose the monitor
     *  @return Monitor
     */
    const Monitor &monitor() const { return _monitor; }
    
    /**
     *  Assign a new state
     *  @param  state           this is a newly allocated state
     *  @return bool            true if the object is still valid
     */
    bool assign(TcpState *state)
    {
        // do nothing if the state did not change, or if object was destructed
        if (_old == state || state == nullptr) return _monitor.valid();
        
        // can we assign a new state?
        if (_monitor.valid()) 
        {
            // assign the 
            _state.reset(state);
        
            // object is still valid
            return true;
        }
        else
        {
            // otherwise the object is destructed and the new state should be destructed too
            delete state;
            
            // object is no longer valid
            return false;
        }
    }
};

/**
 *  End of namespace
 */
}

