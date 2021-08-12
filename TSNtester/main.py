
'''
 * 2020-08 (Aug 2020)
@author:
@contact:
 *  - Implementation:
 *       Luca Wendling <lwendlin@rhrk.uni-kl.de>
 *  - Design & Implementation:
 *       Dennis Krummacker <dennis.krummacker@gmail.com>
'''
 
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import dpkt
import socket
import math
import re
import copy
import random

program = 'tas-test'
cwd = r'/home/luca/Documents/Git/ns-3-dev'

EmptyGateScedule={"length": 0, "GateArray": []}

for i in range(8):
    EmptyGateScedule["GateArray"].append({"start": [], "stop": [], "startOffset": [], "stopOffset": []})

programConfig = {"Datarate": 1e9, "PathDelay": 2e6, "PacketSize": 100, "MaxPackets": 2000,
                 "simulationDuration": 600e9, "sendPeriod": 20e6, "sendPeriodDrifft": 0,
                 "choseTime": 0, "epoch": 0, "Mtu": 1500, "clientIp": "0.0.0.1", "serverIp": "0.0.0.2",
                 "scheduleClient": copy.deepcopy(EmptyGateScedule), "scheduleServer": copy.deepcopy(EmptyGateScedule)}

def combineScheduls(scheduleArray, newScedhule):
    gateMap = newScedhule["gateMap"]
    duration = newScedhule["duration"]
    length = scheduleArray["length"]

    for prio in range(len(gateMap)):
        if gateMap[prio] > 0:
            scheduleArray["GateArray"][prio]["start"].append(length)
            scheduleArray["GateArray"][prio]["stop"].append(length + duration)
            scheduleArray["GateArray"][prio]["startOffset"].append(0)
            scheduleArray["GateArray"][prio]["stopOffset"].append(0)

    scheduleArray["length"] += duration

    return


def createTimeString(value):
    string = ""
    if value >= 0:
        string += '+'
    else:
        string += "-"

    string += str(value)
    string += ".0ns"

    return string


def createSceduleString(schedule):
    string = ""
    for prio in range(8):
        leng = len(schedule[prio]["start"])
        string += str(leng) + "|"
        if leng > 0:
            for index in range(leng):
                string += "[" + createTimeString(schedule[prio]["start"][index]) + ";"
                string += createTimeString(schedule[prio]["stop"][index]) + ";"
                string += createTimeString(schedule[prio]["startOffset"][index]) + ";"
                string += createTimeString(schedule[prio]["stopOffset"][index]) + "]"
        string += "|"

    return string


def createNetDeviceString(config):
    string = "0" + "|"
    string += createTimeString(config["choseTime"]) + "|"
    string += createTimeString(config["schedule"]["length"]) + "|"
    string += createTimeString(config["epoch"]) + "|"
    string += createSceduleString(config["schedule"]["GateArray"])
    return string


def applyConfig(path, config):
    masterFile = open("src/MasterConfig.txt", "r")
    writeFile = open(path, "w")

    NetDeviceConfigClient = {
        "choseTime": config["choseTime"],
        "epoch": config["epoch"],
        "schedule": copy.copy(config["scheduleClient"])
    }

    NetDeviceConfigServer = {
        "choseTime": config["choseTime"],
        "epoch": config["epoch"],
        "schedule": copy.copy(config["scheduleServer"])
    }
    reDatarate = re.compile(r"\"\d+bps\"")
    reMtu = re.compile(r"Mtu \"\d+\"")
    rePacketSize = re.compile(r"PacketSize \"\d+\"")
    reDelay = re.compile(r"Delay \"[+-]\d+\S+\"")
    reStart = re.compile(r"StartTime \"[+-]\d+\S+\"")
    reStop = re.compile(r"StopTime \"[+-]\d+\S+\"")
    reInterval = re.compile(r"Interval \"[+-]\d+\S+\"")
    reNetDeviceClient = re.compile(r"value \S+0\S+0\S+NetDeviceListConfig \"\S+\"")
    reNetDeviceServer = re.compile(r"value \S+1\S+0\S+NetDeviceListConfig \"\S+\"")
    reNetDeviceChange = re.compile(r"NetDeviceListConfig \"\S+\"")
    reMaxPackets = re.compile(r"MaxPackets \"\d+\"")

    list_of_lines = masterFile.readlines()

    for line in list_of_lines:
        if reDatarate.search(line):
            line = reDatarate.sub("\"" + str(config["Datarate"]) + "bps\"", line)
        elif reMtu.search(line):
            line = reMtu.sub("Mtu \"" + str(config["Mtu"]) + "\"", line)
        elif rePacketSize.search(line):
            line = rePacketSize.sub("PacketSize \"" + str(int(config["PacketSize"])) + "\"", line)
        elif reDelay.search(line):
            line = reDelay.sub("Delay \"" + createTimeString(config["PathDelay"]) + "\"", line)
        elif reStart.search(line):
            line = reStart.sub("StartTime \"" + createTimeString(0) + "\"", line)
        elif reStop.search(line):
            line = reStop.sub("StopTime \"" + createTimeString(config["simulationDuration"]) + "\"", line)
        elif reInterval.search(line):
            line = reInterval.sub(
                "Interval \"" + createTimeString(config["sendPeriod"] - config["sendPeriodDrifft"]) + "\"", line)
        elif reNetDeviceClient.search(line):
            line = reNetDeviceChange.sub("NetDeviceListConfig \"" + createNetDeviceString(NetDeviceConfigClient) + "\"",
                                         line)
        elif reNetDeviceServer.search(line):
            line = reNetDeviceChange.sub("NetDeviceListConfig \"" + createNetDeviceString(NetDeviceConfigServer) + "\"",
                                         line)
        elif reMaxPackets.search(line):
            line = reMaxPackets.sub("MaxPackets \"" + str(config["MaxPackets"]) + "\"", line)
        writeFile.writelines(line)

    return path


def runProgramm(program, cwd, config, verbose):
    prefix = " --configPath=" + applyConfig(cwd + "/contrib/tsn/test/" + program + ".txt", config)
    command = './waf --run=\"' + program + prefix + '\"'

    p = subprocess.Popen([command], stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, text=True, cwd=cwd, shell=True)
    p.wait()

    if verbose:
        output, error = p.communicate()
        print(output, error)

    return


def plotfigure(pcapPath, title, offset, delayCorection, deltaT):
    devices = {}
    deltaT = deltaT / 1e9
    for timestamp, buf in dpkt.pcap.Reader(open(pcapPath, 'rb')):

        eth = dpkt.ethernet.Ethernet(buf)
        ipSrc = ""
        # Make sure the Ethernet frame contains an IP packet
        if isinstance(eth.data, dpkt.ip.IP):
            ip = eth.data
            ipSrc = socket.inet_ntoa(ip.src)
        else:
            continue

        time = 0
        timestamp = math.floor(timestamp * 1e9 + 1)
        timestamp = timestamp / 1e9

        # Path delay Corection
        if ipSrc == delayCorection["ipAdress"]:
            time = timestamp * 1e9 - delayCorection["delay"]
            time = math.ceil(time / deltaT)
            time = math.floor(time / 1e9)
        else:
            time = math.floor(timestamp / deltaT)
        if ipSrc not in devices:
            devices[ipSrc] = []

        extend = True

        while extend:
            if len(devices[ipSrc]) <= time:
                devices[ipSrc].extend([0])
            else:
                extend = False

        devices[ipSrc][time] += 1

    length = 0
    maxLength = 30

    for key in list(devices):
        devices[key] = devices[key][offset:(offset + maxLength)]
        if len(devices[key]) > length:
            length = len(devices[key])

    for key in list(devices):
        while (len(devices[key]) < length):
            devices[key].append(0)

    plotedBars = []
    keys = list(devices)

    if len(keys) == 0:
        print("Warning no Data")
        return

    xticks = [offset * deltaT]

    for i in np.arange(1, length):
        xticks.append(deltaT * (i + offset))

    plt.xlim(offset * deltaT, (offset + length) * deltaT)

    width = deltaT * 0.7

    plotedBars.append(plt.bar(xticks, devices[keys[0]], width, label="src: " + keys[0]))

    baroffset = devices[keys[0]]

    for index in np.arange(1, len(keys)):
        plotedBars.append(plt.bar(xticks, devices[keys[index]], width, label="src: " + keys[index], bottom=baroffset))
        baroffset = np.add(devices[keys[index]], baroffset)

    plt.ylabel('Packages')
    plt.xlabel("seconds")
    plt.legend()
    plt.savefig("Plots/" + title + ".png")
    plt.close()

    return


def isAllowedToSend(time, schedule, queue):
    # time in Ns
    if schedule["length"] == 0 or len(schedule['GateArray'][queue]["stop"]) == 0:
        return True

    sceduleTime = math.floor(time % schedule["length"])

    for index in range(len(schedule['GateArray'][queue]["stop"])):
        if sceduleTime < schedule['GateArray'][queue]["stop"][index]:
            if schedule['GateArray'][queue]["start"][index] <= sceduleTime:
                return True
            else:
                return False

    return False


def checkErrors(program, cwd, config, title):
    returndirect = {"program": program,
                    "counter": 0,
                    "errorCounter": 0,
                    "ipcounter": 0,
                    "tcpcounter": 0,
                    "udpcounter": 0,
                    "errorlist": []
                    }

    filenameServer = cwd + '/' + program + '-1-0.pcap'
    filenameClient = cwd + '/' + program + '-0-0.pcap'

    for timestamp, buf in dpkt.pcap.Reader(open(filenameServer, 'rb')):

        returndirect["counter"] += 1
        eth = dpkt.ethernet.Ethernet(buf)

        # Make sure the Ethernet frame contains an IP packet
        if not isinstance(eth.data, dpkt.ip.IP):
            continue

        ip = eth.data
        returndirect["ipcounter"] += 1
        ipSrc = socket.inet_ntoa(ip.src)

        timestampNS = int(timestamp * 1e9)

        # Path delay Corection
        if ipSrc == config["clientIp"]:
            timestampNS -= config["PathDelay"]
            if not isAllowedToSend(timestampNS, config["scheduleClient"], 0):
                returndirect["errorCounter"] += 1
                returndirect["errorlist"].append({
                    "timestamp": timestampNS / 1e9,
                    "index": returndirect["counter"],
                    "src": ipSrc
                })
        else:
            if not isAllowedToSend(timestampNS, config["scheduleServer"], 0):
                returndirect["errorCounter"] += 1
                returndirect["errorlist"].append({
                    "timestamp": timestampNS / 1e9,
                    "index": returndirect["counter"],
                    "src": ipSrc
                })

        if ip.p == dpkt.ip.IP_PROTO_TCP:
            returndirect["tcpcounter"] += 1
        if ip.p == dpkt.ip.IP_PROTO_UDP:
            returndirect["udpcounter"] += 1

    if title:
        deltaT = 0.1e9

        if not len(config["scheduleServer"]["GateArray"][0]["stop"]) == 0:
            deltaT = (config["scheduleServer"]["GateArray"][0]["stop"][0] -
                      config["scheduleServer"]["GateArray"][0]["start"][0])

        plotfigure(filenameServer, title + "_Server", 0, {
            "ipAdress": config["clientIp"],
            "delay": config["PathDelay"]},
                   deltaT)

        if not len(config["scheduleClient"]["GateArray"][0]["stop"]) == 0:
            deltaT = (config["scheduleClient"]["GateArray"][0]["stop"][0] -
                      config["scheduleClient"]["GateArray"][0]["start"][0])

        plotfigure(filenameClient, title + "_Client", 0, {
            "ipAdress": config["serverIp"],
            "delay": config["PathDelay"]},
                   deltaT
                   )
    return returndirect


def createSchedule(array, duration=0.1e9):
    EmptyGateScedule = {"length": 0, "GateArray": []}

    for i in range(8):
        EmptyGateScedule["GateArray"].append({"start": [], "stop": [], "startOffset": [], "stopOffset": []})

    configON = {
        "gateMap": [1, 1, 1, 1, 1, 1, 1, 1],
        "duration": duration,
    }

    configOFF = {
        "gateMap": [0, 0, 0, 0, 0, 0, 0, 1],
        "duration": duration,
    }

    returnSchedule = copy.deepcopy(EmptyGateScedule)

    for toggle in array:
        if toggle:
            combineScheduls(returnSchedule, configON)
        else:
            combineScheduls(returnSchedule, configOFF)

    return returnSchedule


def buildWaf(cwd):
    p = subprocess.Popen(["./waf build"], stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, text=True, cwd=cwd, shell=True)
    p.wait()
    output, error = p.communicate()

    print(output, error)
    return


def fixedTest(cwd, program, programConfig, array):
    invarray = []
    buildWaf(cwd)
    for i in range(len(array)):
        if array[i] > 0:
            invarray.append(0)
        else:
            invarray.append(1)

    programConfig["scheduleClient"] = createSchedule(array)
    programConfig["scheduleServer"] = createSchedule(invarray)
    runProgramm(program, cwd, programConfig, False)

    return checkErrors(program, cwd, programConfig, False)


def scheduleChecker(array):
    foundOne = False,
    foundZero = False,
    for i in range(len(array)):
        if array[i] == 1:
            foundOne = True
        else:
            foundZero = True
        if (foundOne == True) and (foundZero == True):
            return array

    index = int(random.randrange(0, len(array), 1))
    if array[index] == 1:
        array[index] = 0
    else:
        array[index] = 1
    return array

def randomTest(numberOfTests, maxScheduleLength, programConfig, cwd, program):
    errors = []
    buildWaf(cwd)
    errorCount = 0
    succsess = 0
    paketCount = 1
    ErrorRun = 0
    errorexistinRun = 0
    for test in range(numberOfTests):
        for i in range(100):
            if test == numberOfTests / 100 * i:
                print("Processed " + str(i) + "%")

        rendSchedule = []
        rendScheduleInV = []

        temp = [1, 0]
        for i in range(random.randrange(2, maxScheduleLength)):
            onOFF = round(random.random())
            rendSchedule.append(onOFF)
            rendScheduleInV.append(temp[onOFF])

        rendSchedule = scheduleChecker(rendSchedule)
        rendScheduleInV = scheduleChecker(rendScheduleInV)

        duration = random.randrange(int(10e6), int(1000e6), int(1e6))
        programConfig["scheduleClient"] = createSchedule(rendSchedule, duration)
        programConfig["scheduleServer"] = createSchedule(rendScheduleInV, duration)
        programConfig["PathDelay"] = random.randrange(int(100e3), int(duration/5), int(1e3))

        runProgramm(program, cwd, programConfig, False)

        returnval = checkErrors(program, cwd, programConfig, False)
        paketCount += returnval["ipcounter"]
        succsess += returnval["ipcounter"] - returnval["errorCounter"]
        errorCount += returnval["errorCounter"]

        if not returnval["errorCounter"] == 0 or returnval["counter"] < programConfig["MaxPackets"]/2:
            newError = {
                "Schedule": rendSchedule,
                "Inv": rendScheduleInV,
                "PathDelay": programConfig["PathDelay"],
                "duration": duration,
                "error Count:": returnval["errorCounter"],
                "counter": returnval["counter"],
            }
            errors.append(newError)
            print(newError)
            errorexistinRun += 1
            print(str((succsess / paketCount) * 100) + "% success Rate", "\tErrors over all:" + str(int((errorexistinRun/(test+1)) * 100 * 100)/100) + "%")

            if returnval["counter"] < programConfig["MaxPackets"]/2:
                ErrorRun += 1
                print(str(ErrorRun / (test+1) * 100) + "% Fehlerhafte Durchläufe " + str(duration / programConfig["PathDelay"]) )

    print("Processed 100%")
    print(str(ErrorRun/numberOfTests * 100) + "% Fehlerhafte Durchläufe")
    print(str((succsess / paketCount) * 100) + "% Rate")
    return errors


TASpythonBib.randomTest(10000, 30, programConfig, cwd, program)