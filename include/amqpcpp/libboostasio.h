#pragma once

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>


/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Forward declarations
 */
class LibBoostAsioChannel;

/**
 *  Class definition
 */
template<class Stream>
class LibBoostAsioConnectionImpl:
	public std::enable_shared_from_this<LibBoostAsioConnectionImpl<Stream>>,
	private ConnectionHandler {
public:
	using executor_type = typename Stream::executor_type;
	using next_layer_type = std::decay_t<Stream>;
	using lowest_layer_type = typename next_layer_type::lowest_layer_type;

private:
	/**
	 *  Method that is called by the connection when data needs to be sent over the network.
	 *  Please note, that blocking Boost.ASIO write is used here and no
	 *  additional buffering is performed. This is to provide back-pressure
	 *  for stalled network connections instead of running out of memory.
	 *  @param  connection      The connection that created this output
	 *  @param  buffer          Data to send
	 *  @param  size            Size of the buffer
	 *
	 *  @see ConnectionHandler::onData
	 */
	void onData(Connection* connection, const char* data, size_t size) override final {
		boost::system::error_code ec;

		/* Sync write is used here since there is no other way for back-pressure. */
		boost::asio::write(_stream, boost::asio::buffer(data, size), ec);

		if (ec) {
			connection->fail(ec.message().c_str());
		}
	}

	/**
	 *  Method that is called when the heartbeat frequency is negotiated
	 *  between the server and the client. This implementation starts an async
	 *  timer with a half of hearbeat interval.
	 *  @param  connection      The connection that suggested a heartbeat interval
	 *  @param  interval        The suggested interval from the server
	 *  @return uint16_t        The interval to use
	 *
	 *  @see ConnectionHandler::onNegotiate
	 */
	uint16_t onNegotiate(Connection *connection, uint16_t interval) override final {
		if (interval == 0) return 0;

		const auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(interval)) / 2;
		do_wait(timeout);

		return interval;
	}

	/**
	 *  Method that is called when the AMQP protocol was gracefully ended.
	 *  This is the counter-part of a call to connection.close().
	 *  Note, that Boost.ASIO underlying connection are shutdown and closed.
	 *  @param  connection  The AMQP connection
	 *
	 *  @see ConnectionHandler::onClose
	 */
	void onClosed(Connection *connection) override final {
		boost::system::error_code ec;

		_stream.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		_stream.lowest_layer().close(ec);
	}

	/**
	 *  Method prepares and launches async Boost.ASIO reading from the stream.
	 */
	void do_read() {
		using namespace std::placeholders;

		auto self = this->shared_from_this();

		boost::asio::async_read(
			_stream,
			_rx_streambuf,
			boost::asio::transfer_at_least(_connection.expected() - _rx_streambuf.size()),
			boost::asio::bind_executor(_strand, std::bind(&LibBoostAsioConnectionImpl::handle_read, std::move(self), _1, _2))
		);
	}

	/**
	 *  Method that is called when async Boost.ASIO operation is done. The
	 *  data received from the stream are passed to AMQP parsing.
	 *  @param  ec                 Error code
	 *  @param  transferred_bytes  Received bytes
	 */
	void handle_read(const boost::system::error_code& ec, std::size_t transferred_bytes) {
		if (ec) {
			_connection.fail(ec.message().c_str());
			return;
		}

		std::size_t consumed_total = 0;

		for (const auto& buffer: _rx_streambuf.data()) {
			consumed_total += _connection.parse(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}

		_rx_streambuf.consume(consumed_total);

		do_read();
	}

	/**
	 *  Method prepares and launches async Boost.ASIO timer.
	 *  @param  timeout  Timer timeout in milliseconds
	 */
	void do_wait(std::chrono::milliseconds timeout) {
		using namespace std::placeholders;

		auto self = this->shared_from_this();

		_heartbeat.expires_after(timeout);
		_heartbeat.async_wait(boost::asio::bind_executor(_strand, std::bind(&LibBoostAsioConnectionImpl::handle_wait, std::move(self), _1, timeout)));
	}

	/**
	 *  Method that is called when async Boost.ASIO timer is expired. The
	 *  connection heartbeat is sent.
	 *  @param  ec                 Error code
	 *  @param  transferred_bytes  Received bytes
	 */
	void handle_wait(const boost::system::error_code& ec, std::chrono::milliseconds timeout) {
		if (ec) {
			_connection.fail(ec.message().c_str());
			return;
		}

		_connection.heartbeat();
		do_wait(timeout);
	}

public:
	/**
	 *  Constructor
	 *  @param  stream      The unerlying Boost.ASIO stream object
	 *  @param  args        Arguments forwared to AMQP::Connection
	 */
	template<class... Args>
	LibBoostAsioConnectionImpl(Stream&& stream, Args&&... args):
		_stream{std::forward<Stream>(stream)},
		_strand{_stream.get_executor()},
		_heartbeat{_stream.get_executor()},
		_connection{this, std::forward<Args>(args)...},
		_rx_streambuf{_connection.maxFrame()} {}

	/**
	 *  Method prepares and launches async Boost.ASIO reading from the stream.
	 */
	void start() {
		do_read();
	}

	LibBoostAsioConnectionImpl(const LibBoostAsioConnectionImpl&) = delete;
	LibBoostAsioConnectionImpl(LibBoostAsioConnectionImpl&&) = delete;
	LibBoostAsioConnectionImpl& operator=(const LibBoostAsioConnectionImpl&) = delete;
	LibBoostAsioConnectionImpl& operator=(LibBoostAsioConnectionImpl&&) = delete;

	/**
	 *  The method creates shared pointer with LibBoostAsioConnectionImpl.
	 *  The method is intended to be used instead of private class
	 *  constructor.
	 *  @param  stream      The unerlying Boost.ASIO stream object
	 *  @param  args        Arguments forwared to AMQP::Connection
	 */
	template<class... Args>
	static std::shared_ptr<LibBoostAsioConnectionImpl<Stream>> create(Stream&& stream, Args&&... args) {
		auto implementation = std::make_shared<LibBoostAsioConnectionImpl<Stream>>(std::forward<Stream>(stream), std::forward<Args>(args)...);

		/* Two-phase construction is required here since shared_from_this() is not available in constructor */
		implementation->start();

		return implementation;
	}

	/**
	 *  Return reference to the underflying AMQP::Connection object.
	 *  @return Connection      The underlying AMQP::Connection object
	 */
	Connection& connection() noexcept {
		return _connection;
	}

	/**
	 *  Return reference to the underflying AMQP::Connection object.
	 *  @return Connection      The underlying AMQP::Connection object
	 */
	const Connection& connection() const noexcept {
		return _connection;
	}

	/**
	 *  Return reference to the Boost.ASIO executor in use
	 *  @return executor_type   Boost.ASIO executor object
	 */
	const executor_type& get_executor() noexcept {
		return _stream.get_executor();
	}

	/**
	 *  Return reference to the lowest underlying Boost.ASIO stream object
	 *  @return lowest_layer_type  The lowest underlying Boost.ASIO stream object
	 */
	lowest_layer_type& lowest_layer() noexcept {
		return _stream.lowest_layer();
	}

	/**
	 *  Return reference to the lowest underlying Boost.ASIO stream object
	 *  @return lowest_layer_type  The lowest underlying Boost.ASIO stream object
	 */
	const lowest_layer_type& lowest_layer() const noexcept {
		return _stream.lowest_layer();
	}

	/**
	 *  Return reference to the underlying Boost.ASIO stream object
	 *  @return next_layer_type  The underlying Boost.ASIO stream object
	 */
	next_layer_type& next_layer() noexcept {
		return _stream;
	}

	/**
	 *  Return reference to the underlying Boost.ASIO stream object
	 *  @return next_layer_type  The underlying Boost.ASIO stream object
	 */
	const next_layer_type& next_layer() const noexcept {
		return _stream;
	}

	/**
	 *  Destructor
	 */
	virtual ~LibBoostAsioConnectionImpl() = default;

private:
	/**
	 *  The underlying Boost.ASIO stream object
	 *  @var _stream
	 */
	Stream _stream;

	/**
	 *  The strand to protect concurrent stream handlers execution
	 *  @var _strand
	 */
	boost::asio::strand<executor_type> _strand;

	/**
	 *  The timer to control heartbeat sending
	 *  @var _heartbeat
	 */
	boost::asio::steady_timer _heartbeat;

	/**
	 *  The underlying AMQP connection
	 *  @var _connection
	 */
	Connection _connection;

	/**
	 *  The buffer to accumulate the data for parsing
	 *  @var _rx_streambuf
	 */
	boost::asio::streambuf _rx_streambuf;
};

/**
 *  Class definition
 */
template<class Stream>
class LibBoostAsioConnection {
private:
	/**
	 *  Return reference to the underflying AMQP::Connection object.
	 *  @return Connection      The underlying AMQP::Connection object
	 */
	Connection& connection() {
		return _implementation->connection();
	}

	/**
	 *  Return reference to the underflying AMQP::Connection object.
	 *  @return Connection      The underlying AMQP::Connection object
	 */
	const Connection& connection() const {
		return _implementation->connection();
	}

public:
	using executor_type = typename LibBoostAsioConnectionImpl<Stream>::executor_type;
	using next_layer_type = typename LibBoostAsioConnectionImpl<Stream>::next_layer_type;
	using lowest_layer_type = typename LibBoostAsioConnectionImpl<Stream>::lowest_layer_type;

	/**
	 *  Construct an LibBoostAsioConnection object based on full login data
	 *
	 *  The first parameter is a Boost.ASIO stream object.
	 *
	 *  @param  stream          Unerlying Boost.ASIO stream object
	 *  @param  login           Login data
	 *  @param  vhost           Vhost to use
	 */
	LibBoostAsioConnection(Stream&& stream, const Login &login, const std::string &vhost):
		_implementation{LibBoostAsioConnectionImpl<Stream>::create(std::forward<Stream>(stream), login, vhost)} {}
	/**
	 *  Construct with default vhost
	 *  @param  stream          Unerlying Boost.ASIO stream object
	 *  @param  login           Login data
	 */
	LibBoostAsioConnection(Stream&& stream, const Login &login):
		_implementation{LibBoostAsioConnectionImpl<Stream>::create(std::forward<Stream>(stream), login)} {}
	/**
	 *  Construct with default login data
	 *  @param  stream          Unerlying Boost.ASIO stream object
	 *  @param  vhost           Vhost to use
	 */
	LibBoostAsioConnection(Stream&& stream, const std::string &vhost):
		_implementation{LibBoostAsioConnectionImpl<Stream>::create(std::forward<Stream>(stream), vhost)} {}
	/**
	 *  Construct an LibBoostAsioConnection object with default login data and default vhost
	 *  @param  stream          Unerlying Boost.ASIO stream object
	 */
	LibBoostAsioConnection(Stream&& stream):
		_implementation{LibBoostAsioConnectionImpl<Stream>::create(std::forward<Stream>(stream))} {}

	/**
	 *  Return reference to the Boost.ASIO executor in use
	 *  @return executor_type   Boost.ASIO executor object
	 */
	const executor_type& get_executor() noexcept {
		return _implementation->get_executor();
	}

	/**
	 *  Return reference to the lowest underlying Boost.ASIO stream object
	 *  @return lowest_layer_type  The lowest underlying Boost.ASIO stream object
	 */
	lowest_layer_type& lowest_layer() noexcept {
		return _implementation->lowest_layer();
	}

	/**
	 *  Return reference to the lowest underlying Boost.ASIO stream object
	 *  @return lowest_layer_type  The lowest underlying Boost.ASIO stream object
	 */
	const lowest_layer_type& lowest_layer() const noexcept {
		return _implementation->lowest_layer();
	}

	/**
	 *  Return reference to the underlying Boost.ASIO stream object
	 *  @return next_layer_type  The underlying Boost.ASIO stream object
	 */
	next_layer_type& next_layer() noexcept {
		return _implementation->next_layer();
	}

	/**
	 *  Return reference to the underlying Boost.ASIO stream object
	 *  @return next_layer_type  The underlying Boost.ASIO stream object
	 */
	const next_layer_type& next_layer() const noexcept {
		return _implementation->next_layer();
	}

	/**
	 *  Retrieve the login data
	 *  @return Login
	 */
	const Login& login() const {
		return connection().login();
	}

	/**
	 *  Retrieve the vhost
	 *  @return string
	 */
	const std::string& vhost() const {
		return connection().vhost();
	}

	/**
	 *  Max frame size
	 *  @return size_t
	 */
	uint32_t maxFrame() const {
		return connection().maxFrame();
	}

	/**
	 *  Expected number of bytes for the next parse() call.
	 *
	 *  @return size_t
	 */
	uint32_t expected() const {
		return connection().expected();
	}

	/**
	 *  Is the connection ready to accept instructions / has passed the login handshake and not closed?
	 *  @return bool
	 */
	bool ready() const {
		return connection().ready();
	}

	/**
	 *  Is (or was) the connection initialized
	 *  @return bool
	 */
	bool initialized() const {
		return connection().initialized();
	}

	/**
	 *  Is the connection in a usable state, or is it already closed or
	 *  in the process of being closed?
	 *  @return bool
	 */
	bool usable() const {
		return connection().usable();
	}

	/**
	 *  Close the connection
	 *  This will close all channels
	 *  @return bool
	 */
	bool close() {
		return connection().close();
	}

	/**
	 *  Retrieve the number of channels that are active for this connection
	 *  @return std::size_t
	 */
	std::size_t channels() const {
		return connection().channels();
	}

	/**
	 *  Is the connection busy waiting for an answer from the server? (in the
	 *  meantime you can already send more instructions over it)
	 *  @return bool
	 */
	bool waiting() const {
		return connection().waiting();
	}

	/**
	 *  Some classes have access to private properties
	 */
	friend LibBoostAsioChannel;

private:
	/**
	 *  The pointer to implementation of the Boost.ASIO connection
	 *  @var _implementation
	 */
	std::shared_ptr<LibBoostAsioConnectionImpl<Stream>> _implementation;
};

/**
 *  Class definition
 */
class LibBoostAsioChannel: public Channel {
public:
	/**
	 *  Constructor
	 *
	 *  The passed in connection pointer must remain valid for the
	 *  lifetime of the channel. A constructor is thrown if the channel
	 *  cannot be connected (because the connection is already closed or
	 *  because max number of channels has been reached)
	 *
	 *  @param  connection
	 *  @throws std::runtime_error
	 */
	template<class Stream>
	LibBoostAsioChannel(LibBoostAsioConnection<Stream>* connection):
		Channel(&connection->connection()) {}

	/**
	 *  Destructor
	 */
	virtual ~LibBoostAsioChannel() = default;
};

/**
 *  End of namespace
 */
}
