/**
 *  Watchable.h
 *
 *  Every class that overrides from the Watchable class can be monitored for
 *  destruction by a Monitor object
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
class Watchable
{
private:
    /**
     *  The monitors
     *  @var set
     */
    std::set<Monitor*> _monitors;

    /**
     *  Add a monitor
     *  @param  monitor
     */
    void add(Monitor *monitor)
    {
        _monitors.insert(monitor);
    }
    
    /**
     *  Remove a monitor
     *  @param  monitor
     */
    void remove(Monitor *monitor)
    {
        _monitors.erase(monitor);
    }

public:
    /**
     *  Destructor
     */
    virtual ~Watchable();
    
    /**
     *  Only a monitor has full access
     */
    friend class Monitor;
};     

/**
 *  End of namespace
 */
}
