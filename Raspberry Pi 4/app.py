import paho.mqtt.client as mqtt
import json
from flask import Flask, render_template
from flask_socketio import SocketIO
app = Flask(__name__)
socketio = SocketIO(app)

# Configurazione MQTT
broker = "192.168.43.175"
topics = [("Luminosity", 0), ("Altitude", 0), ("Pressure", 0), ("Temperature", 0)]

def on_connect(client, userdata, flags, rc):
    print("Connesso al broker MQTT con codice:", rc)
    for topic in topics:
        client.subscribe(topic[0])

def on_message(client, userdata, msg):
    if msg.topic == "Luminosity":
        json_data = json.loads(msg.payload.decode('utf-8'))
        socketio.emit('mqtt_message_L', {'value':json_data['value'],'timestamp':json_data['timestamp']})
    elif msg.topic == "Altitude":
        json_data = json.loads(msg.payload.decode('utf-8'))
        socketio.emit('mqtt_message_A', {'value':json_data['value'],'timestamp':json_data['timestamp']})
    elif msg.topic == "Pressure":
        json_data = json.loads(msg.payload.decode('utf-8'))
        socketio.emit('mqtt_message_P', {'value':json_data['value'],'timestamp':json_data['timestamp']})
    elif msg.topic == "Temperature":
        json_data = json.loads(msg.payload.decode('utf-8'))
        socketio.emit('mqtt_message_T', {'value':json_data['value'],'timestamp':json_data['timestamp']})

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(broker, 1883, 60)
mqtt_client.loop_start()

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
