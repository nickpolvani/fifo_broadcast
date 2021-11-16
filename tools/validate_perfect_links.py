

"""
Perfect Links Validation
Ensure:
    Validity: If p_i and p_j are correct, then every message sent by p_i to p_j is eventually delivered by p_j

    No duplication: No message is delivered more than once

    No creation: No message is delivered unless it was sent
"""

import argparse
import os
from tqdm import tqdm
import glob 


def get_process_ids(hosts_path):
    hosts = []
    f = open(hosts_path, mode="r")
    for line in tqdm(f):
        hosts.append((line.split()[0]))
    f.close()
    return hosts
    
def read_config(config_path):
    f = open(config_path, mode="r")
    line = f.readline()
    num_messages = int(line.split()[0])
    id_receiver = line.split()[1]
    f.close()
    return num_messages, id_receiver


def read_sender_file(path):
    messages = set()
    out_file = open(path, "r")
    for line in tqdm(out_file):
        if line.split()[0] != "b":
            raise ValueError("Invalid format for outfile", path, "line read:", line)
        if not line.split()[1].isdigit():
            raise ValueError("Invalid format for outfile", path, "line read:", line)
        messages.add(line.split()[1])
    out_file.close()
    return messages


def read_receiver_file(path):
    # reads and checks for duplication
    delivered = {}
    f = open(path, "r")
    for line in tqdm(f):
        tokens = line.split()
        if tokens[0] != "d" or (not tokens[1].isdigit()) or (not tokens[2].isdigit()):
            raise ValueError("Invalid format for outfile of reader", path, "line read:", line)
        sender_id = tokens[1]
        message_id = tokens[2]
        if sender_id not in delivered:
            delivered[sender_id] = set()
            delivered[sender_id].add(message_id)
        else:
            if message_id in delivered[sender_id]:
                raise ValueError("Found message delivered more than once:", line)
            delivered[sender_id].add(message_id)
    f.close()
    return delivered


def check_no_creation(delivered:dict, map_messages_sent:dict):
    for sender_id in tqdm(delivered):
        sent = map_messages_sent[sender_id]
        for message in delivered[sender_id]:
            if message not in sent:
                raise ValueError("Message was delivered but not sent: sender_id:", sender_id, "message:", message)



def validate(hosts_path:str, config_path:str, out_dir:str):
    hosts = get_process_ids(hosts_path)
    num_messages, id_receiver = read_config(config_path)
    out_paths = glob.glob(os.path.join(out_dir, "*.output"))
    if len(out_paths) != len(hosts):
        raise ValueError("Output directory contains", len(out_paths), "files, expected", len(hosts))
    map_out_paths = {} # maps process_id to their out file
    for out_path in out_paths:
        map_out_paths[out_path.split("/")[-1].split(".")[0]] = out_path
    
    map_messages_sent = {} #stores messages sent by every sender process
    for process_id in hosts:
        if process_id != id_receiver:
            print("Reading sender file for process:", process_id)
            map_messages_sent[process_id] = read_sender_file(map_out_paths[process_id])

    print("Reading receiver file")
    delivered = read_receiver_file(map_out_paths[id_receiver])
    print("Checking no creation")
    check_no_creation(delivered, map_messages_sent)


def check_file(path):
    if os.path.isfile(path):
        return path
    else:
        raise ValueError(path, "is not a file")

def check_dir(path):
    if os.path.isdir(path):
        return path
    else:
        raise ValueError(path, "is not a directory")


def main():
    parser = argparse.ArgumentParser(description="Validate Perfect Links")

    parser.add_argument("--hosts", 
                        required=True, 
                        type=check_file, 
                        dest="hosts_path",
                        help="Path to file containing information about processes in the system",
                        )

    parser.add_argument("--config",  
                        required=True, 
                        type=check_file, 
                        dest="config_path",
                        help="Path to config file",
                        )

    parser.add_argument("--out_dir",
                        required=True,
                        type=check_dir,
                        dest="out_dir",
                        help="Directory containing output files",
                        )

    results = parser.parse_args()

    print("Validating....")
    validate(hosts_path=results.hosts_path, config_path=results.config_path, out_dir=results.out_dir)
    print("Congratulations! Your out files passed the test.")


if __name__ == "__main__":
    main()