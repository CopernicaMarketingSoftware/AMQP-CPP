/**
 *  Support class for boost asio network operations
 * 
 *  @copyright 2014 TeamSpeak Systems GmbH
 */

#ifndef AMQP_CPP_BOOST_BOOSTREADBUFFER_H
#define AMQP_CPP_BOOST_BOOSTREADBUFFER_H

#include <vector>
#include <stdexcept>
/**
 *  Set up namespace
 */
namespace AMQP {

/**
 *  Class definition Boost read buffer can store bytes and have some bytes removed
 */
 class BoostReadBuffer
 {
private:
    /**
     *  The data storing variable
     *  @var    std::vector<char>
     */
	std::vector<char> _buffer;

    /**
     *  The size that the io layer will try to read
     *  @var    std::size_t
     */
	std::size_t _readChunkSize;

	/**
     *  The prefered size of the buffer
     *  @var    std::size_t
     */
	std::size_t _preferredSize;
	
public: 
	/**
	* Construct a Boost Read Buffer
	*
	* @param preferredSize the preferred size in bytes of the buffer
	*/
	BoostReadBuffer(std::size_t readChunkSize, std::size_t preferredSize)
	: _buffer()
	, _readChunkSize(readChunkSize)
	, _preferredSize(preferredSize)
	{}
	
	/**
	* reserve data at the end of the buffer for reading into
	*
	* @return memory pointer where to write into the buffer
	*/
	char* prepare()
	{
		auto oldSize = _buffer.size();
		_buffer.resize(oldSize+_readChunkSize);
		return _buffer.data()+oldSize;
	}
	
	/**
	* commit read bytes to data
	*
	* @param size size of the data to add
	*/
	void commit(std::size_t size)
	{
		auto diff = _readChunkSize - size;
		_buffer.resize(_buffer.size()-diff);
	}
	
	/**
	 * clear all data
	 */
	void clear()
	{
		_buffer.clear();
	}
	
	/**
	* Get the number of bytes to read in a chunk
	*
	* @return std::size_t
	*/
	std::size_t readChunkSize() const
	{
		return _readChunkSize;
	}
	
	/**
	* Remove data at the start of the buffer
	*
	* @param size size of the data to remove.
	*/
	void remove(std::size_t size)
	{
		if (size > _buffer.size()) throw std::out_of_range("size");
		
		if (size == _buffer.size())
		{
			_buffer.clear();
		}
		else
		{
			bool doShrink = _buffer.size() > _preferredSize;
			_buffer.erase(_buffer.begin(), _buffer.begin()+size);
			if (doShrink ) _buffer.shrink_to_fit();
		}
	}
	
	/**
	* Get the number of bytes in the buffer
	*
	* @return std::size_t
	*/
	std::size_t size() const
	{
		return _buffer.size();
	}
	
	/**
	* Get the beginning of the data buffer
	*
	* @return char*
	*/
	char* data()
	{
		return _buffer.data();
	}
 };
 
 /**
* end namespace
*/
}

#endif //AMQP_CPP_BOOST_BOOSTREADBUFFER_H