
# This is a stripped server who serve JSON data and video stream:
#
# To get JSON data, call: http://localhost:5000/json
#
# To get the video stream, call: http://localhost:5000/video_feed
#
# If more videostreams will be implemented in the future, you can
# call each using: http://localhost:5000/video_feed/<string:id>/
#
# To see JSON data preety formatted in Chrome we use JSONVue extension

# ================= RTSP test streams =================
# http://hoybakken.dyndns.org:9876/mjpg/video.mjpg
# http://204.69.166.184:9080/mjpg/video.mjpg
# http://namesti.mestovm.cz/mjpg/video.mjpg
# http://kamera.mikulov.cz:8888/mjpg/video.mjpg
# =====================================================

# this script does the following:
#     1. load data to global PARAMETER list
#     2. run Waitress server to serve JSON data upon request


# import modules used for systemc calls, date and time, MODBUS
import os
import sys
import time
import socket
import pickle
import struct
import datetime
import subprocess
from numpy import random

##############################################

# import modules used for JSON
from flask import Flask
from flask import jsonify
from flask import request
from flask import render_template
from flask import Response
import threading
import time
import waitress
import json
import cv2

app = Flask(__name__)


# define parameters list as global so we can access it from thread
global PARAMETERS
PARAMETERS = []

#global frame

# initialize parameters to zero
for i in range(50):
    PARAMETERS.append(0.0)
    
    
def gatheringData():
    # access parameters from global list
    global PARAMETERS
    while True:
        # we read all parameters from MODBUS

        # ---- Here we will read all Modbus parameters over 485,
        #      now we insert random data !!!
        for i in range(30):
            PARAMETERS[i] = random.randint(100) * 1000.0            
        time.sleep(0.5)   


@app.route('/json')
def send_json():
    # get parameters from global list
    global PARAMETERS

    # get the IP of the request
    ip_address = request.remote_addr; 
    print('Parameter list was interogated from IP:', ip_address) 

    # create JSON data packet
    data = {
        "curent_conductor": PARAMETERS[0],
        "temperatura_conductor": PARAMETERS[1],
        "inclinare_conductor": PARAMETERS[2],
        "galopare_conductor_X": PARAMETERS[3],       
        "galopare_conductor_Y": PARAMETERS[4],       
        "galopare_conductor_Z": PARAMETERS[5],
        "forta": PARAMETERS[6],      
        "chiciura": PARAMETERS[7],
        "inclinare_stalp": PARAMETERS[8],
        "vibratii_stalp": PARAMETERS[9],
        "temperatura_ambianta": PARAMETERS[10],
        "umiditate_relativa": PARAMETERS[11],
        "viteza_vant": PARAMETERS[12],
        "directie_vant": PARAMETERS[13],
        "presiune_barometrica": PARAMETERS[14],
        "radiatie_solara": PARAMETERS[15],
        "grindina": PARAMETERS[16],
        "precipitatii_lichide": PARAMETERS[17],
        "curent_A": PARAMETERS[18],
        "curent_B": PARAMETERS[19],
        "curent_C": PARAMETERS[20],
        "tensiune_A": PARAMETERS[21],
        "tensiune_B": PARAMETERS[22],
        "tensiune_C": PARAMETERS[23],
        
        'ip_cerere': ip_address,
    }
    return json.dumps(data)


def genFrames():
    # url de test cand sunt conectat cu pc la internet
    rtsp_url = "http://hoybakken.dyndns.org:9876/mjpg/video.mjpg" 

    cam = rtsp_url
    cap =  cv2.VideoCapture(cam)
    while True:
        # for cap in caps:
        # # Capture frame-by-frame
        success, frame = cap.read()  # read the camera frame
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            # concat frame one by one and show result
            yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')


#@app.route('/video_feed/<string:id>/', methods=["GET"])
@app.route('/video_feed')
def video_feed():

    #if (debug == 1):
    # get the IP of the request
    ip_address = request.remote_addr; 
    print('Camera video stream was accessed from IP:', ip_address) 
    
    """Video streaming route. Put this in the src attribute of an img tag."""
    return Response(genFrames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/', methods=["GET"])
def index():
    return render_template('index.html')



# start MODBUS data acquisition thread in background
gatheringData_thread = threading.Thread(target=gatheringData)
# set the thread as daemon to be killed on exit
gatheringData_thread.setDaemon(True)
# start the data gathering thread
gatheringData_thread.start()


# start Waitress server, so we can serve requests
waitress.serve(app, port=5000, host="0.0.0.0")

'''
# for multiple cameras
# def genFrames(id):
#    if id == 1:
#        rtsp_url = "http://....
#    if id == 2:
#        rtsp_url = "http://....
# to transfer id so you select different cameras
#@app.route('/video_feed/<string:id>/', methods=["GET"])
@app.route('/video_feed/<id>')
def video_feed(id):
    if (debug == 1):
        # get the IP of the request
        ip_address = request.remote_addr; 
        print('Camera video stream was accessed from IP:', ip_address) 
    
    """Video streaming route. Put this in the src attribute of an img tag."""
    return Response(genFrames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')
'''
