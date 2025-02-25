import certifi
import paho.mqtt.client as mqtt

import time

def initialize():
    client = mqtt.Client(transport='websockets')

    mqtt.Client.message_contents = ""
    mqtt.Client.message_received = False
    mqtt.Client.message_topic = ""

    client.on_connect = on_connect
    client.on_message = on_message

    client.tls_set(ca_certs=certifi.where())

    client.username_pw_set('solace-cloud-client', 'p2i7li6ckbaimoe0draq0qdl82')
    my_url = "mrzpfs1b9tj1n.messaging.solace.cloud"
    client.connect(my_url, port=20553)
    client.loop_start()
    return client

# Callback on connection
def on_connect(client, userdata, flags, rc):
    print('Connected')
    client.subscribe('joystick_axis')
    client.subscribe('buttons')
    client.publish('test_connect', payload='Connected')

# Callback when message is received
def on_message(client, userdata, msg):
    # print(f'Message received on topic: {msg.topic}. Message: {msg.payload.decode()}')
    message_str = msg.payload.decode()
    client.message_received = True
    client.message_topic = msg.topic
    client.message_contents = message_str