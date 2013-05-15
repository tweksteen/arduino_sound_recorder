#!/usr/bin/env python

import sys
import struct
from socket import *
from matplotlib.pylab import plot, show
from numpy import linspace

SAMPLES = 256
SAMPLE_WIDTH = 2
PACKET_SIZE = SAMPLES * SAMPLE_WIDTH
PROLOGUE_SIZE = 12

def monitor():
  dropped = 0
  vs = []
  ts = []
  while len(vs) < 4096 * 80:
    s = socket(AF_INET, SOCK_DGRAM)
    s.bind(("0.0.0.0", 8888))
    x = s.recv(PROLOGUE_SIZE + PACKET_SIZE)
    if x[0:3] != "SND":
      print "bad header"
      dropped += 1
      continue
    print x[3]
    start_time = struct.unpack(">L", x[4:8])[0]
    if not vs:
      origin = start_time
    end_time = struct.unpack(">L", x[8:12])[0]
    diff = (end_time-start_time)*1e-6
    print start_time, end_time, diff
    if diff >= 14e-3 or diff < 12e-3:
      print "dropping packet"
      dropped += 1 
      continue
    print "dt=%fms" % (diff * 1e3)
    ts.extend(linspace(start_time, end_time, SAMPLES).tolist())
    for i in range(PROLOGUE_SIZE, len(x), SAMPLE_WIDTH):
      vs.append(struct.unpack("<H", x[i:i+SAMPLE_WIDTH])[0])
    print "frate=%f" % (SAMPLES/diff)
    print "per sample=%fus" % ((diff/SAMPLES) * 1e6)
  
  print "#dropped=", dropped
  vs = [ (v - 512) * 64 for v in vs]
  
  plot(ts, vs)
  show()
  raw_input()
  import wave
  data_size = len(vs)
  frate = int(float(data_size)/ ((end_time-origin)*1e-6))
  print "final frate=%f" % frate
  
  wav_file = wave.open("blah.wav", "w")
  wav_file.setparams((1, SAMPLE_WIDTH, frate, data_size, "NONE", "NONE"))
 
  for v in vs:
    wav_file.writeframes(struct.pack('h', v))
  wav_file.close()
  
monitor()
