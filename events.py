 # -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
from influxdb import client as influxdb
import config
import logging
import sys

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

ch = logging.StreamHandler(sys.stdout)
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
logger.addHandler(ch)

db = influxdb.InfluxDBClient("localhost", 8086, "root", "root", config.database_name)

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

init_influx()

def on_connect(userdata, flags_dict, result):
    cl.subscribe([("/node/light",1), ("outTopic",1)])

def on_message(client, userdata, message):
    try:
        val = int(message.payload)
    except:
        val = None

    typ = message.topic.split("/")[-1]
    client = userdata.get("name")
    data = [
        dict(
            name = "events.{}.{}".format(client, typ),
            columns = ["value"],
            points = [[val]]
        )
    ]
    logger.info("Writing: {}".format(data))
    try:
        res = db.write_points(data)
        logger.info(res)
    except Exception as e:
        logger.warning(e)

cl.on_connect = on_connect
cl.on_message = on_message

cl.loop_forever()