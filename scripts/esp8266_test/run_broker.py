
import logging
import asyncio
import os
from amqtt.broker import Broker


async def start_broker():
    ip = os.getenv("BROKER_IP")
    port = os.getenv("BROKER_PORT")
    # Minimal config
    config = {
        'listeners': {
            'default': {
                'type': 'tcp',
                'bind': f'{ip}:{port}',
            }
        },
        'sys_interval': 10,
        'auth': {
            'allow-anonymous': True,
        },
        'topic-check': {
            'enabled': True,
            'plugins': ['topic_acl'],
            'acl': {
                'anonymous': ['#']
            }
        }
    }
    broker = Broker(config)
    await broker.start()
    logging.basicConfig(level=logging.INFO)
    logging.info(f"MQTT Broker started on {ip}:{port}")
    try:
        while True:
            await asyncio.sleep(1)
    except KeyboardInterrupt:
        await broker.shutdown()

if __name__ == '__main__':
    formatter = "[%(asctime)s] :: %(levelname)s :: %(name)s :: %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    asyncio.run(start_broker())
