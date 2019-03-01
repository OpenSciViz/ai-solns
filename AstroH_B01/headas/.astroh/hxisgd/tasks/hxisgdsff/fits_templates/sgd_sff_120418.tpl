xtension=bintable

#------------------------------------------------------------
#------------------------------------------------------------
# Columns for FFF
#------------------------------------------------------------
#------------------------------------------------------------

#number of rows
naxis2 = 1


ttype# = TIME
tform# = 1D

ttype# = S_TIME
tform# = 1D

ttype# = Packet_Header
tform# = 20B

#Time Indicator
ttype# = TI
tform# = 1J

ttype# = Occurence_ID
tform# = 1J

ttype# = Local_Time
tform# = 1J

ttype# = FLAG_SEU
tform# = 1X

ttype# = FLAG_LengthCheck
tform# = 1X

ttype# = FLAG_CalMode
tform# = 1X

ttype# = FLAG_TriggerPattern
tform# = 31X

ttype# = FLAG_Trigger
tform# = 6X

ttype# = FLAG_CCBusy
tform# = 3X

ttype# = FLAG_HitPatternCC
tform# = 3X

ttype# = FLAG_HitPattern
tform# = 4X

ttype# = FLAG_FastBGO
tform# = 4X

#1J or 24X (actually 24-bit width)
ttype# = LiveTime
tform# = 1J

ttype# = nHitASICs
tform# = 1B

#------------------------------------------------------------
# Original ASIC Event Data
#------------------------------------------------------------
# Data output from each ASIC have
# flags (4 bits),
# ASIC ID (12 bits),
# DM (1 bit) + channel data (64 bits) + CM (1 bit),
# Reference (10 bits),
# ADC (10 bits x n ; 10*64 = 640 bits max),
# Common mode noise (10 bits),
# and 0s for filling.
#
# Therefore, the maximum length of one ASIC data is
# pack_16bits(4+12+1+64+1+10+640+10+fill)
# = pack_16bits(742+fill) = pack_16bits(752)
# = 47x2 bytes = 94 bytes
#
# One SGD has (208/CC)x3CC=624 ASIC chips, and therefore, the max length of
# the event data part (of the raw event telemetry) becomes
# 624 chips x 94 bytes = 58656 bytes

ttype# = RAW_ASIC_DATA
tform# = 1PB(58656)


#------------------------------------------------------------
#------------------------------------------------------------
# Columns added for SFF
#------------------------------------------------------------
#------------------------------------------------------------

# Name       : Remapped ASIC ID
# Data type  : Signed short (2byte integer = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = ASIC_ID2
tform# = 1PI(39936)

# Name       : Original ASIC ID
# Data type  : Signed short (2byte integer = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = ASIC_ID2
tform# = 1PI(39936)

# Name       : ASIC Header Data
# Data type  : 24 bits information packed to 32bits and treated as Unsigned Integer (V)
# Max length : 39936 (624 chips * 64 channels)
ttype# = ASIC_Header_Data
tform# = 1PV(39936)

# Name       : Remapped Channel ID
# Data type  : Unsigned byte (B)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Channel_ID
tform# = 1PB(39936)

# Name       : Original Channel ID
# Data type  : Unsigned byte (B)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Channel_ID
tform# = 1PB(39936)

# Name       : Raw ADC Data
# Data type  : 10-bit pulse height data packet to signed short (16bits = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Raw_ADC_Pulse_Heights
tform# = 1PI(39936)

# Name       : Calibrated ADC Data
# Data type  : 10-bit pulse height data packet to signed short (16bits = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Calibrated_ADC_Pulse_Heights
tform# = 1PI(39936)
