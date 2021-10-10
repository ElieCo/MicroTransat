import os
import sys
import struct

from google.protobuf.json_format import MessageToJson

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__)) + "/"
SRC_DIR = TOOLS_DIR + "../src/"
PY_PROTO_DIR = SRC_DIR + "protobuf/python/"
EMB_DIR = TOOLS_DIR + "../emb_files/"
JSON_OUTPUT_FILE = EMB_DIR + "conf.json"
BIN_OUTPUT_FILE = EMB_DIR + "bin_conf.txt"
LOG_DIR = TOOLS_DIR + "../log/"

sys.path.append(PY_PROTO_DIR)
from managers_pb2 import *
from utils_pb2 import *

def getLineDic(data, conf):

    mission_data = data.mission_manager
    log_data = data.log_manager
    helm_data = data.helm_manager
    wing_data = data.wing_manager
    captain_data = data.captain_manager
    comm_data = data.comm_manager
    gps_data = data.sensor_manager.gps
    battery_data = data.sensor_manager.battery
    radio_data = data.sensor_manager.radio
    light_data = data.light_manager
    config_data = data.config_manager

    mission_conf =  conf.mission_manager
    log_conf =  conf.log_manager
    helm_conf =  conf.helm_manager
    wing_conf =  conf.wing_manager
    captain_conf =  conf.captain_manager
    comm_conf =  conf.comm_manager
    sensor_conf =  conf.sensor_manager
    gps_conf =  conf.sensor_manager.gps
    light_conf =  conf.light_manager
    config_conf =  conf.config_manager

    # Clear useless mission element parameters

    if mission_data.prev_element.type == DataMissionManager.MissionElement.AWA:
        mission_data.prev_element.coord.latitude = 0
        mission_data.prev_element.coord.longitude = 0
        mission_data.prev_element.corridor_width = 0
        mission_data.prev_element.valid_dist = 0
    else:
        mission_data.prev_element.angle = 0
        mission_data.prev_element.duration = 0

    if mission_data.next_element.type == DataMissionManager.MissionElement.AWA:
        mission_data.next_element.coord.latitude = 0
        mission_data.next_element.coord.longitude = 0
        mission_data.next_element.corridor_width = 0
        mission_data.next_element.valid_dist = 0
    else:
        mission_data.next_element.angle = 0
        mission_data.next_element.duration = 0

    dico = {
        "datetime" : str(gps_data.date) + "_" + str(gps_data.time),

        "wpt_dist": mission_data.wpt_dist,
        "wpt_angle": mission_data.wpt_angle,
        "element_index": mission_data.element_index,
        "dist_to_axis": mission_data.dist_to_axis,
        "in_corridor": mission_data.in_corridor,
        "corridor_angle": mission_data.corridor_angle,
        "setpoint_update_asked": mission_data.setpoint_update_asked,
        "prev_element_type": mission_data.prev_element.type,
        "prev_element_latitude": mission_data.prev_element.coord.latitude,
        "prev_element_longitude": mission_data.prev_element.coord.longitude,
        "prev_element_corridor_width": mission_data.prev_element.corridor_width,
        "prev_element_valid_dist": mission_data.prev_element.valid_dist,
        "prev_element_angle": mission_data.prev_element.angle,
        "prev_element_duration": mission_data.prev_element.duration,
        "prev_element_ephemeral": mission_data.prev_element.ephemeral,
        "next_element_type": mission_data.next_element.type,
        "next_element_latitude": mission_data.next_element.coord.latitude,
        "next_element_longitude": mission_data.next_element.coord.longitude,
        "next_element_corridor_width": mission_data.next_element.corridor_width,
        "next_element_valid_dist": mission_data.next_element.valid_dist,
        "next_element_angle": mission_data.next_element.angle,
        "next_element_duration": mission_data.next_element.duration,
        "next_element_ephemeral": mission_data.next_element.ephemeral,

        "sd_ready" : log_data.sd_ready,

        "helm_angle" : helm_data.angle,
        "helm_cmd_applied" : helm_data.cmd_applied,

        "wing_angle" : wing_data.angle,

        "just_wake_up" : captain_data.just_wake_up,
        "behaviour" : captain_data.behaviour,
        "helm_order" : captain_data.helm_order,

        "gps_latitude" : gps_data.coord.latitude,
        "gps_longitude" : gps_data.coord.longitude,
        "gps_altitude" : gps_data.altitude,
        "gps_fix" : gps_data.fix,
        "gps_fix_quality" : gps_data.fix_quality,
        "gps_satellites" : gps_data.satellites,
        "gps_fix_age" : gps_data.fix_age,
        "gps_speed" : gps_data.speed,
        "gps_course" : gps_data.course,
        "gps_hdop" : gps_data.hdop,
        "gps_recent_data" : gps_data.recent_data,
        "gps_average_course" : gps_data.average_course,
        "gps_average_course_full" : gps_data.average_course_full,
        "gps_ready" : gps_data.ready,

        "voltage" : battery_data.voltage,

        "radio_controlled" : radio_data.radio_controlled,

        "mission_interval" : mission_conf.base.interval,
        "default_corridor_width" : mission_conf.default_corridor_width,
        "default_validation_distance" : mission_conf.default_validation_distance,
        "start_auto_angle" : mission_conf.start_auto_angle,
        "start_auto_duration" : mission_conf.start_auto_duration,

        "log_interval" : log_conf.base.interval,

        "helm_interval" : helm_conf.base.interval,
        "helm_ratio" : helm_conf.ratio,
        "helm_offset" : helm_conf.offset,
        "normal_speed" : helm_conf.normal_speed,
        "tack_speed" : helm_conf.tack_speed,

        "wing_interval" : wing_conf.base.interval,
        "wing_step" : wing_conf.step,
        "wing_calib_offset" : wing_conf.calib_offset,

        "captain_interval" : captain_conf.base.interval,
        "max_upwind" : captain_conf.max_upwind,
        "max_downwind" : captain_conf.max_downwind,
        "sleeping_duration" : captain_conf.sleeping_duration,

        "comm_interval" : comm_conf.base.interval,

        "sensor_interval" : sensor_conf.base.interval,
        "max_valid_hdop" : sensor_conf.gps.max_valid_hdop,

        "light_interval" : light_conf.base.interval,

        "config_interval" : config_conf.base.interval,
    }

    #print(dico["prev_element_valid_dist"])

    # Turn all value into str
    for key in dico.keys():
        if type(dico[key]) == bool:
            dico[key] = str(int(dico[key]))
        if type(dico[key]) == float:
            dico[key] = "%.8f" % dico[key]
        else:
            dico[key] = str(dico[key])

    return dico

if __name__ == "__main__":

    arg = sys.argv
    if len(arg) < 2:
        print("You have to give me the path to the binary file.")
        exit(-1)

    f = open(arg[1], 'rb')

    # GetConf
    l_bin = f.read(4)
    l, = struct.unpack("I", l_bin)
    data_bin = f.read(l)

    conf = Conf()
    conf.ParseFromString(data_bin)

    # Get Data
    all_data = []

    l_bin = f.read(4)
    while len(l_bin):

        l, = struct.unpack("I", l_bin)
        data_bin = f.read(l)

        data = Data()
        data.ParseFromString(data_bin)
        all_data.append(data)

        l_bin = f.read(4)

    #csv_name = "LOG_" + f_name.split("/")[-1].split(".")[0] + ".csv"
    csv_name = "LOG_" + getLineDic(all_data[0], conf)["datetime"] + ".csv"
    f_csv = open(LOG_DIR + csv_name, "w")

    # Write header
    keys = getLineDic(all_data[0], conf).keys()
    f_csv.write(";".join(keys) + "\n")

    # Write lines
    for data in all_data:
        values = getLineDic(data, conf).values()
        f_csv.write(";".join(values) + "\n")

    f_csv.close()
