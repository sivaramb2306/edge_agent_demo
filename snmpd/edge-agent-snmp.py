#!/usr/bin/env python3
import sys
import time
import os
import json

# APC PowerNet MIB prefix
UPS_PREFIX = '.1.3.6.1.4.1.318'

# State file
UPS_STATE_FILE = '/var/lib/snmp/ups_state.json'

# UPS OID mappings
UPS_OIDS = {
    f'{UPS_PREFIX}.1.1.1.1.1.1.0': 'upsBasicIdentModel',        # UPS Model
    f'{UPS_PREFIX}.1.1.1.2.1.1.0': 'upsBasicBatteryStatus',     # Battery Status
    f'{UPS_PREFIX}.1.1.1.2.1.2.0': 'upsBasicBatteryTimeOnBattery', # Time on Battery
    f'{UPS_PREFIX}.1.1.1.2.1.3.0': 'upsBasicBatteryLastReplaceDate', # Battery Last Replace Date
    f'{UPS_PREFIX}.1.1.1.3.1.1.0': 'upsBasicInputPhase',        # Input Phase
    f'{UPS_PREFIX}.1.1.1.4.1.1.0': 'upsBasicOutputStatus',      # Output Status
    f'{UPS_PREFIX}.1.1.1.4.1.2.0': 'upsBasicOutputVoltage',     # Output Voltage
    f'{UPS_PREFIX}.1.1.1.4.1.3.0': 'upsBasicOutputFrequency',   # Output Frequency
    f'{UPS_PREFIX}.1.1.1.4.1.4.0': 'upsBasicOutputLoad'         # Output Load
}

def init_ups_state():
    """Initialize UPS state if it doesn't exist"""
    if not os.path.exists(UPS_STATE_FILE):
        default_state = {
            f'{UPS_PREFIX}.1.1.1.1.1.1.0': 'Smart-UPS 3000',
            f'{UPS_PREFIX}.1.1.1.2.1.1.0': '2',  # Battery Normal
            f'{UPS_PREFIX}.1.1.1.2.1.2.0': '0',  # Time on Battery
            f'{UPS_PREFIX}.1.1.1.2.1.3.0': '2024-01-24',  # Last Replace Date
            f'{UPS_PREFIX}.1.1.1.3.1.1.0': '1',  # Single Phase
            f'{UPS_PREFIX}.1.1.1.4.1.1.0': '2',  # Output Normal
            f'{UPS_PREFIX}.1.1.1.4.1.2.0': '230', # Output Voltage
            f'{UPS_PREFIX}.1.1.1.4.1.3.0': '50',  # Output Frequency
            f'{UPS_PREFIX}.1.1.1.4.1.4.0': '45'   # Output Load
        }
        with open(UPS_STATE_FILE, 'w') as f:
            json.dump(default_state, f, indent=2)

def read_state():
    """Read UPS state from file"""
    try:
        with open(UPS_STATE_FILE, 'r') as f:
            return json.load(f)
    except:
        init_ups_state()
        with open(UPS_STATE_FILE, 'r') as f:
            return json.load(f)

def get_next_oid(current_oid):
    """Get the next OID in sequence"""
    all_oids = sorted(UPS_OIDS.keys())
    try:
        idx = all_oids.index(current_oid)
        if idx + 1 < len(all_oids):
            return all_oids[idx + 1]
    except ValueError:
        # Find the next highest OID
        for oid in all_oids:
            if oid > current_oid:
                return oid
    return None

def get_value_type(oid):
    """Determine the SNMP value type based on the OID"""
    # Numeric values
    if oid in [
        f'{UPS_PREFIX}.1.1.1.2.1.1.0',   # Battery Status
        f'{UPS_PREFIX}.1.1.1.2.1.2.0',   # Time on Battery
        f'{UPS_PREFIX}.1.1.1.3.1.1.0',   # Input Phase
        f'{UPS_PREFIX}.1.1.1.4.1.1.0',   # Output Status
        f'{UPS_PREFIX}.1.1.1.4.1.2.0',   # Output Voltage
        f'{UPS_PREFIX}.1.1.1.4.1.3.0',   # Output Frequency
        f'{UPS_PREFIX}.1.1.1.4.1.4.0'    # Output Load
    ]:
        return 'integer'
    return 'string'

def main():
    command = sys.argv[1] if len(sys.argv) > 1 else None
    
    if command == '-g':  # GET
        oid = sys.argv[2]
        state = read_state()
        if oid in state:
            print(oid)
            print(get_value_type(oid))
            print(state[oid])
            
    elif command == '-n':  # GET NEXT
        oid = sys.argv[2]
        next_oid = get_next_oid(oid)
        if next_oid:
            state = read_state()
            print(next_oid)
            print(get_value_type(next_oid))
            print(state[next_oid])
            
    elif command == '-s':  # SET
        oid = sys.argv[2]
        value_type = sys.argv[3]
        value = sys.argv[4]
        # Implement SET logic if needed
        pass
        
    else:  # Initial ping
        print("1")
        sys.exit(0)

if __name__ == '__main__':
    main()
