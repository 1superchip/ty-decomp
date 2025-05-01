"""
YAML / JSON file helpers
"""

import json
import pickle
from typing import Dict

import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

########
# Yaml #
########

def dump_to_yaml(path: str, data):
    """Dumps an object to a yaml file"""

    with open(path, 'w') as f:
        f.write(yaml.dump(data))

def load_from_yaml(path: str, default=None):
    """Loads an object from a yaml file"""

    if default is None:
        default = {}
    with open(path) as f:
        ret = yaml.load(f.read(), Loader)
        if ret is None:
            ret = default
        return ret

def load_from_yaml_str(s: str, default=None):
    """Loads an object from a yaml string"""

    if default is None:
        default = {}
    ret = yaml.load(s, Loader)
    if ret is None:
        ret = default
    return ret

##########
# Pickle #
##########

def dump_to_pickle(path: str, data: Dict):
    """Dumps an object to a pickle file"""

    with open(path, 'wb') as f:
        pickle.dump(data, f)

def load_from_pickle(path: str) -> Dict:
    """Loads an object from a pickle file"""

    with open(path, 'rb') as f:
        return pickle.load(f)

########
# Json #
########

def dump_to_json_str(data: Dict):
    """Dumps an object to a json string"""

    return json.dumps(data)
