#!/usr/bin/env python3

import argparse
import json
import getpass
from concurrent.futures import ThreadPoolExecutor, as_completed

from rffe_nuttx_lib import RFFEFWUpdate
from rffe_tunnel_lib import SSHTunnel


def load_rack_config(file_path):
    with open(file_path) as f:
        return json.load(f)

def tunnel_forward_fw_update(hostname, username, ip, password, fw_file, version, key_path):
    tunnel = SSHTunnel(
            host=hostname,
            username=username, 
            key_path=key_path,
            remote_host=ip,
            remote_port=9090,
            key_pass=password
            )

    try:
        ssh_channel = tunnel.connect()
        rffe = RFFEFWUpdate(ip_addr='localhost', sock=ssh_channel)  
        
        print(f"[INFO] Connected on ip {ip}, hostname {hostname}. Flashing firmware...")
        rffe.reprogram(fw_file, version)
        print(f"[INFO] Firmware update completed on ip {ip}")

        rffe.close()
    except Exception as e:
        print(f"[ERROR] Firmware update failed on {ip}: {e}")
    finally:
        tunnel.close()            

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="RFFE Firmware Update Tool")
    parser.add_argument("--json-in", type=str, help="Path to rack configuration JSON file", required=True)
    parser.add_argument("--fw-ver", type=str, help="Set firmware version string in the format x.y.z (major, minor, bug fix)", required=True)
    parser.add_argument("--bin-file", type=str, help="Path to .bin firmware file", required=True)
    parser.add_argument('-l','--list', nargs='+', help="Rack IDs to update (default: all in config)")
    parser.add_argument("--key-path", type=str, help="Path to SSH private key")
    parser.add_argument("--username", type=str, default="lnls-bpm", help="SSH username (default: lnls-bpm)")
    parser.add_argument("--max-threads", type=int, default=10, help="Maximum number of concurrent threads (default: 10)")

    args = parser.parse_args()
        
    racks = load_rack_config(args.json_in)
    
    ssh_password = getpass.getpass("Enter SSH private key password: ")

    racks_id = args.list or [rack["rack"] for rack in racks]
    
    for rack_id in racks_id:
        rack = next((rack for rack in racks if str(rack["rack"]) == rack_id), None)        
        if not rack:
            print(f"[ERROR] Rack {rack_id} not found in {args.json_in}")
            continue

        with ThreadPoolExecutor(max_workers=args.max_threads) as executor:
            futures = []
            for crate_id, crate_ip in rack["rffe"]["subsector"].items():        
                futures.append(executor.submit(tunnel_forward_fw_update, rack["hostname"], args.username, crate_ip, ssh_password, args.bin_file, args.fw_ver, args.key_path))
            
            for future in as_completed(futures):
                try:
                    future.result()
                except Exception as e:
                    print(f"[ERROR] Task failed: {e}")
