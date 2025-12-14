
import logging
import asyncio
import os
import struct
from datetime import datetime
from amqtt.client import MQTTClient, ConnectException
from amqtt.mqtt.constants import QOS_0

logger = logging.getLogger(__name__)

async def subscriber():
    ip = os.getenv("BROKER_IP", "127.0.0.1")
    port = os.getenv("BROKER_PORT", "1883")
    # No auth for now, or use allow-anonymous
    url = f'mqtt://{ip}:{port}'

    client = MQTTClient()
    try:
        await client.connect(url)
    except ConnectException as ce:
        logger.error("Connection failed: %s", ce)
        return

    await client.subscribe([
        ('sensors/temperature', QOS_0),
    ])
    logger.info(f"Subscribed to sensors/temperature on {url}")

    try:
        while True:
            message = await client.deliver_message()
            packet = message.publish_packet
            try:
                if len(packet.payload.data) == 8:
                   (val,) = struct.unpack('<d', packet.payload.data)
                   payload = f"{val:.2f}"
                else:
                   payload = packet.payload.data.decode('utf-8')
            except Exception:
                payload = f"HEX: {packet.payload.data.hex()}"
            
            print(f"[{datetime.now().strftime('%H:%M:%S.%f')}] Received: {payload} on topic {packet.variable_header.topic_name}")
    except KeyboardInterrupt:
        pass
    finally:
        await client.disconnect()

if __name__ == '__main__':
    formatter = "[%(asctime)s] :: %(levelname)s :: %(name)s :: %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    asyncio.run(subscriber())
