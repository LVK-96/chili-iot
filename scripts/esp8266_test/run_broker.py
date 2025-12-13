
import logging
import asyncio
from amqtt.broker import Broker

# Minimal config: Listen on all interfaces, port 1883
config = {
    'listeners': {
        'default': {
            'type': 'tcp',
            'bind': '0.0.0.0:1883',
        }
    },
    'sys_interval': 10,
    'auth': {
        'allow-anonymous': True,
    }
}

async def start_broker():
    broker = Broker(config)
    await broker.start()
    logging.basicConfig(level=logging.INFO)
    logging.info("MQTT Broker started on port 1883")
    try:
        while True:
            await asyncio.sleep(1)
    except KeyboardInterrupt:
        await broker.shutdown()

if __name__ == '__main__':
    formatter = "[%(asctime)s] :: %(levelname)s :: %(name)s :: %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    asyncio.run(start_broker())
