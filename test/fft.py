""" Play a WAVE file. """

import pyaudio
import wave
import sys
import scipy
import struct
from numpy.fft import rfft
from numpy import real, imag, absolute


chunk = 8
bufferSize = 16 #2 ** 11 #try the value  16 like in the c program
if len(sys.argv) < 2:
    print "Plays a wave file.\n\n" +\
          "Usage: %s filename.wav" % sys.argv[0]
    sys.exit(-1)

wf = wave.open(sys.argv[1], 'rb')

p = pyaudio.PyAudio()


class fft:


	def __init__(self):
		self.maxValue = 0
		self.calls = 0

	def normalize(self, complexDataValue):
		dataValue = absolute(complexDataValue)
		if dataValue > self.maxValue:
			self.maxValue = dataValue
		
		return dataValue / self.maxValue
		
	def membershipFunction(self):
		return [0,0,.05,.15,.6,.15,0.05]
		
	def listMin(self, firstList, secondList):
		listLen = min(len(firstList), len(secondList))
		
		result = []
		for i in range(0, listLen):
			result.append(min(firstList[i], secondList[i]))
			
		return result
		
	def transform(self, data):
		self.calls += 1
		unpackedData=scipy.array(struct.unpack("%dB"%(bufferSize*2),data))
		
		
		normalizedValues = [self.normalize(x) for x in rfft(unpackedData)]
		
		sum = reduce(lambda x,y: y+x, self.listMin(self.membershipFunction(), normalizedValues))
		
		#print sum
		
		if sum > 0.3:
			print "blink"
			
		if self.calls % 1000 == 0:
			print self.calls
		
#		for cValue in normalizedValues:
#			print "{1} Abs {0}".format(cValue, i)
		
	


# open stream
print "Samples: {0}" .format( wf.getframerate())
stream = p.open(format =
                p.get_format_from_width(wf.getsampwidth()),
                channels = wf.getnchannels(),
                rate = wf.getframerate(),
                output = True)

# read data
data = wf.readframes(chunk)

t = fft()

t.transform(data)


# play stream
while data != '':
    stream.write(data)
    data = wf.readframes(chunk)
    t.transform(data)


stream.close()
p.terminate()

