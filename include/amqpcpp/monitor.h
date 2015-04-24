/**
 *  Monitor.h
 *
 *  A monitor object monitors if the connection is still valid. When the 
 *  connection is parsing incoming data, it calls the user handler for each
 *  incoming frame. However, it is unknown what this handler is going to do,
 *  it could for example decide to destruct the connection object. In that
 *  case the connection object should stop further handling the data. This
 *  monitor class is used to check if the connection has been destructed.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition
 */
class Monitor
{
private:
    /**
     *  The object being watched
     *  @var    Watchable
     */
    Watchable *_watchable;

    /**
     *  Invalidate the object
     */
    void invalidate()
    {
        _watchable = nullptr;
    }

public:
    /**
     *  Constructor
     *  @param  watchable
     */
    Monitor(Watchable *watchable) : _watchable(watchable)
    {
        // register with the watchable
        _watchable->add(this);
    }
    
    /**
     *  Destructor
     */
    virtual ~Monitor()
    {
        // remove from watchable
        if (_watchable) _watchable->remove(this);
    }
    
    /**
     *  Check if the object is valid
     *  @return bool
     */
    bool valid()
    {
        return _watchable != nullptr;
    }
    
    /**
     *  The watchable can access private data
     */
    friend class Watchable;
};
        


/**
 *  End of namespace
 */
}


 