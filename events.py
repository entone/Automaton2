 # -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
#from influxdb import client as influxdb
import config
import logging
import sys
import os
from Crypto.Cipher import AES
import random

MODE = AES.MODE_CBC
BLOCK_SIZE = 16
INTERRUPT = u'\u0000'
PAD = u'\u0000'
SKEY = bytearray(["1","1","1","1","1","1","1","1","1","1","1","1","1","1","1",0x00])
KEY = buffer(SKEY)

def pad(st):
    st = ''.join([st, config.INTERRUPT])
    st_len = len(st)
    rem_len = config.BLOCK_SIZE-st_len
    padding_len = rem_len%config.BLOCK_SIZE
    padding = config.PAD*padding_len
    final_st = ''.join([st, padding])
    return final_st

def encrypt(st, key):
    st = pad(st)
    IV = ''.join(chr(random.randint(0, 0xFF)) for i in range(16))
    CIPHER = AES.new(key, MODE, IV)
    res = CIPHER.encrypt(st)
    res = IV+res
    return base64.urlsafe_b64encode(res)

def decrypt(st, key):
    logger.info(st)
    SIV = bytearray(st[:16])
    IV = buffer(SIV)
    CIPHER = AES.new(key, MODE, IV)
    res = CIPHER.decrypt(st[16:])
    return res.split("\x00")[0]

def generate_key():
    return os.urandom(16)



logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

ch = logging.StreamHandler(sys.stdout)
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
logger.addHandler(ch)

#db = influxdb.InfluxDBClient("localhost", 8086, "root", "root", config.database_name)

cl = mqtt.Client(client_id="client123456", userdata={"name":"client123456"})
cl.connect("localhost")

def init_influx():
    data = {
        "name":config.database_name,
        "spaces":config.spaces,
        "continuousQueries":config.queries
    }
    try:
        res = db.request(
            url="cluster/database_configs/{}".format(config.database_name),
            method="POST",
            data=data,
            status_code=201
        )
        logger.debug(res)
    except Exception as e:
        logger.warning(e)

#init_influx()

def on_connect(userdata, flags_dict, result):
    logger.info("Open")
    cl.subscribe([("/node/#",1), ("outTopic",1)])

def on_message(client, userdata, message):
    try:
        val = decrypt(message.payload, KEY)
        val = int(val)
    except:
        val = 0

    typ = message.topic.split("/")[-1]
    if typ == "encrypted":
        d = decrypt(message.payload, KEY)
        parts = d.split("\x00")
        logger.info(parts[0]);
        return

    client = userdata.get("name")
    data = [
        dict(
            name = "events.{}".format(client),
            columns = ["value", "type"],
            points = [[val, typ]]
        )
    ]
    logger.info("Writing: {}".format(data))
    try:
        pass
        #res = db.write_points(data)
        #logger.info(res)
    except Exception as e:
        logger.warning(e)

cl.on_connect = on_connect
cl.on_message = on_message

cl.loop_forever()
