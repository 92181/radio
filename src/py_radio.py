from ctypes import CDLL, c_char_p, CFUNCTYPE
import ctypes
import os

# Path .SO
lib = CDLL(os.path.join("radio.so"))

# Add Stream State
lib.stream_state.restype = ctypes.c_int8

def stream_state():
  res = lib.stream_state()
  if res is None:
    return ""
  return res

# Add Shutdown
def stream_stop():
  lib.stream_stop()

# Title Callback
CALLBACK_TYPE = CFUNCTYPE(None, c_char_p)

# Add Start Radio
global_callback = None
lib.stream_start.argtypes = (CALLBACK_TYPE, ctypes.c_char_p)
lib.stream_start.restype = None

def stream_start(u,m):
  global global_callback
  global_callback = CALLBACK_TYPE(m)
  
  lib.stream_start(global_callback, u.encode("utf-8"))
