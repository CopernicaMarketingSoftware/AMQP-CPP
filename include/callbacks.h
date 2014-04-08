/**
 *  Callbacks.h
 *
 *  Class storing deferred callbacks of different type.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class for managing deferred callbacks
 */
class Callbacks
{
private:
    /**
     *  Different callback types supported
     */
    std::tuple<
        std::deque<Deferred<>>,
        std::deque<Deferred<const std::string&, uint32_t, uint32_t>>,
        std::deque<Deferred<uint32_t>>
    > _callbacks;

    /**
     *  If all else fails, we have gotten the wrong
     *  type, which is not present in the arguments.
     *
     *  This should result in a compile error.
     */
    template <class T, std::size_t N, class... Arguments>
    struct getIndex
    {
        // if this structure is used, we went past the last argument
        // and this static_assert should trigger a compile failure.
        static_assert(N < sizeof...(Arguments), "Type T not found in Arguments");

        // we still have to provide this member though
        static constexpr std::size_t value = N;
    };

    /**
     *  This structure has one static member that represents
     *  the index of T in Arguments. This variant is used where U
     *  does equal T, so a match is found, meaning the current
     *  index given is the right one.
     */
    template <class T, std::size_t N, class... Arguments>
    struct getIndex<T, N, T, Arguments...>
    {
        // element is same type as we are looking for
        static constexpr std::size_t value = N;
    };

    /**
     *  This structure has one static member that represents
     *  the index of T in Arguments. This variant is used where U
     *  does not equal T, so we need to look at the next member.
     */
    template <class T, std::size_t N, class U, class... Arguments>
    struct getIndex<T, N, U, Arguments...>
    {
        // current N is not correct, unroll to next element
        static constexpr std::size_t value = getIndex<T, N + 1, Arguments...>::value;
    };

    /**
     *  Retrieve the list of callbacks matching the type
     *
     *  @param  tuple   tuple with callbacks
     */
    template <class T, class... Arguments>
    T& get(std::tuple<Arguments...>& tuple)
    {
        // retrieve the index at which the requested callbacks can be found
        constexpr std::size_t index = getIndex<T, 0, Arguments...>::value;

        // retrieve the callbacks
        return std::get<index>(tuple);
    }
public:
    /**
     *  Add a deferred to the available callbacks
     *
     *  @param  deferred    the deferred to add
     *  @return reference to the inserted deferred
     */
    template <typename... Arguments>
    Deferred<Arguments...>& push_back(Deferred<Arguments...>&& item)
    {
        // retrieve the container
        auto &container = get<std::deque<Deferred<Arguments...>>>(_callbacks);

        // add the element
        container.push_back(std::move(item));

        // return reference to the new item
        return container.back();
    }

    /**
     *  Report success to the relevant callback
     *
     *  @param  mixed...    additional parameters
     */
    template <typename... Arguments>
    void reportSuccess(Arguments ...parameters)
    {
        // retrieve the container and element
        auto &container = get<std::deque<Deferred<Arguments...>>>(_callbacks);
        auto &callback  = container.front();

        // execute the callback
        callback.success(parameters...);

        // remove the executed callback
        container.pop_front();
    }

    /**
     *  Report a failure
     *
     *  @param  error   a description of the error
     */
    template <std::size_t N = 0>
    typename std::enable_if<N == std::tuple_size<decltype(_callbacks)>::value>::type
    reportError(const std::string& message)
    {}

    /**
     *  Report a failure
     *
     *  @param  error   a description of the error
     */
    template <std::size_t N = 0>
    typename std::enable_if<N < std::tuple_size<decltype(_callbacks)>::value>::type
    reportError(const std::string& message)
    {
        // retrieve the callbacks at current index
        auto &callbacks = std::get<N>(_callbacks);

        // report errors to all callbacks of the current type
        for (auto &callback : callbacks) callback.error(message);

        // execute the next type
        reportError<N + 1>(message);
    }
};

/**
 *  End namespace
 */
}
