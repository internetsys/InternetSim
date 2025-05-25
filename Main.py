from Topology import Topology
from Scheduler import Scheduler
from AS import AS
from Interpreter import Interpreter
from Engine import Engine

import configparser
import time
import random


def generate_random_prefix():
    subnet = random.randint(1, 32)
    
    ip_int = random.getrandbits(32)
    
    mask = (0xFFFFFFFF << (32 - subnet)) & 0xFFFFFFFF
    network_int = ip_int & mask
    
    octets = [
        (network_int >> 24) & 0xFF,
        (network_int >> 16) & 0xFF,
        (network_int >> 8) & 0xFF,
        network_int & 0xFF
    ]
    
    return ".".join(map(str, octets)) + "/" + str(subnet)


if __name__ == "__main__":

    config = configparser.ConfigParser()
    config.read("config.ini")

    topology = Topology()
    interpreter = Interpreter()
    interpreter.loadRoutingInformation(config, topology)

    engine = Engine(config)
    engine.run(config, topology)
