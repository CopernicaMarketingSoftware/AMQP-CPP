"""
Requires aio-pika:

    pip3 install aio-pika

"""

from typing import Tuple
import asyncio
import aio_pika
import sys


PROPERTIES = {
    "x-queue-mode": "lazy",
    "x-dead-letter-exchange": "",
    "x-dead-letter-routing-key": "in"
}

async def declare_queue(channel: aio_pika.Channel) -> aio_pika.Queue:
    """
    Convenience function to declare a queue with fixed properties.
    """
    return await channel.declare_queue(sys.argv[3], durable=True, arguments=PROPERTIES)


async def process_message(message: aio_pika.IncomingMessage) -> None:
    """
    Reject.
    """
    await message.reject()


async def periodic_redeclare(channel: aio_pika.Channel) -> None:
    """
    Periodically redeclare a queue. This allows us to read the message count in the
    queue of interest. When the message count reaches zero, stop the loop.
    """
    while True:
        await asyncio.sleep(5.0)
        queue = await declare_queue(channel)
        if queue.declaration_result.message_count == 0:
            break
    asyncio.get_event_loop().stop()


async def make_connection(loop: asyncio.AbstractEventLoop) -> Tuple[aio_pika.RobustConnection, aio_pika.RobustChannel]:
    """
    Set up a secure connection and start consuming from the queue provided in sys.argv
    """
    connection: aio_pika.RobustConnection = await aio_pika.connect_robust(sys.argv[1], loop=loop, ssl=True)
    channel = await connection.channel(publisher_confirms=False)
    await channel.set_qos(prefetch_count=int(sys.argv[2]))
    queue = await declare_queue(channel)
    await queue.consume(process_message)
    return connection, channel


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    connection, channel = loop.run_until_complete(make_connection(loop))
    try:
        loop.create_task(periodic_redeclare(channel))
        loop.run_forever()
    finally:
        loop.run_until_complete(channel.close())
        loop.run_until_complete(connection.close())
        loop.run_until_complete(loop.shutdown_asyncgens())
        loop.close()
