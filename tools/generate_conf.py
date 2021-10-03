import os
import sys

from google.protobuf.json_format import MessageToJson

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__)) + "/"
SRC_DIR = TOOLS_DIR + "../src/"
PY_PROTO_DIR = SRC_DIR + "protobuf/python/"
EMB_DIR = TOOLS_DIR + "../emb_files/"
JSON_OUTPUT_FILE = EMB_DIR + "conf.json"
BIN_OUTPUT_FILE = EMB_DIR + "bin_conf.txt"

sys.path.append(PY_PROTO_DIR)
from managers_pb2 import *
from utils_pb2 import *

conf = Conf()

conf.mission_manager.base.interval = 500
conf.mission_manager.default_corridor_width = 40
conf.mission_manager.default_validation_distance = 15
conf.mission_manager.start_auto_angle = 90
conf.mission_manager.start_auto_duration = 10

conf.log_manager.base.interval = 500

conf.helm_manager.base.interval = 100
conf.helm_manager.ratio = -0.56
conf.helm_manager.offset = 90
conf.helm_manager.normal_speed = 20
conf.helm_manager.tack_speed = 90

conf.wing_manager.base.interval = 100
conf.wing_manager.step = 10
conf.wing_manager.calib_offset = 0

conf.captain_manager.base.interval = 100
conf.captain_manager.max_upwind = 45
conf.captain_manager.max_downwind = 130
conf.captain_manager.speeping_duration = 15000

conf.comm_manager.base.interval = 0

conf.sensor_manager.base.interval = 0
conf.sensor_manager.gps.max_valid_hdop = 500

conf.light_manager.base.interval = 500

conf.config_manager.base.interval = 10000

json_str = MessageToJson(conf)
f = open(JSON_OUTPUT_FILE, "w")
f.write(json_str)
f.close()

data = conf.SerializeToString()
f = open(BIN_OUTPUT_FILE, "wb")
f.write(data)
f.close()
