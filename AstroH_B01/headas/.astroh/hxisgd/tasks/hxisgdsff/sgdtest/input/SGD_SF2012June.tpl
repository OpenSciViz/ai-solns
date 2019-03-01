xtension=bintable
extname = events
telescop= astro-h
instrume = SGD

#number of rows
naxis2 = 1

ttype# = event_description
tform# = 128a

#------------------------------------------------------------
#------------------------------------------------------------
# Columns for FFF
#------------------------------------------------------------
#------------------------------------------------------------

ttype# = TIME
tform# = 1D

ttype# = S_TIME
tform# = 1D

ttype# = CCSDS_Packet_Header
tform# = 20B

#Time Indicator
#ttype# = TI
ttype# = Time_Indicator
tform# = 1J

ttype# = Occurence_ID
tform# = 1J

ttype# = Local_Time
tform# = 1J

#ttype# = FLAG_SEU
#tform# = 1X
ttype# = FLAG_SingleEventUpset
tform# = 1B

ttype# = FLAG_LengthCheck
#tform# = 1X
tform# = 1B

ttype# = FLAG_CalMode
#tform# = 1X
tform# = 1B

ttype# = FLAG_InitialTriggerPattern
#tform# = 31X
tform# = 1J

ttype# = FLAG_TriggerPattern
#tform# = 31X
tform# = 1J

#ttype# = FLAG_Trigger
#tform# = 6X

ttype# = FLAG_CCBusy
#tform# = 3X
tform# = 1B

ttype# = FLAG_HitPatternCC
#tform# = 3X
tform# = 1B

ttype# = FLAG_HitPattern
#tform# = 4X
tform# = 1B

#ttype# = FLAG_FastBGO
#tform# = 4X
ttype# = FLAG_FastShieldHitPattern
tform# = 1B

#1J or 24X (actually 24-bit width)
ttype# = LiveTime
tform# = 1J

ttype# = nHitASICs
#tform# = 1B
tform# = 1I

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

# Name       : ASIC Header Data
# Data type  : 24 bits information packed to 32bits and treated as Unsigned Integer (V)
# hon: or UJ ?
# Max length : 39936 (624 chips * 64 channels)
ttype# = ASIC_Header_Data
# tform# = 1PV(39936)
tform# = 1PJ(39936)

# Name       : Original ASIC ID
# Data type  : Signed short (2byte integer = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Original_ASIC_ID
tform# = 1PI(39936)

# Name       : Remapped ASIC ID
# Data type  : Signed short (2byte integer = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# =  Remapped_ASIC_ID
tform# = 1PI(39936)

# Name       : Original Channel ID
# Data type  : Unsigned byte (B)
# Max length : 39936 (624 chips * 64 channels)
#ttype# =  Original_Channel_ID
#tform# = 1PB(39936)

# Name       : Original Channel BitFlags
# Data type  : Unsigned byte (B)
# Max length : 39936 (624 chips * 64 channels)
ttype# =  Original_Channel_BitFlags
#tform# = 1PB(39936)
# hon: or 624 64bit ints ?
# hon: but is we want to avoid 64 bit ints use 2*624 32 bit ints?
#tform# = 1PUJ(1248)
tform# = 1PUK(624)

# Name       : Remapped Channel ID
# Data type  : Unsigned byte (B)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Remapped_Channel_ID
#tform# = 1PB(39936)
tform# = 1PUI(39936)

# Calculated as the sum of the active channel x ASIC.
# hon -- number of active chanel per asic to sgd 624 asics
# This value is max of 32 for HXI and 64 for SGD

#ttype# = NUM_ACTIVE_CHAN
ttype# = NumberOf_Active_Channels
tform# = 1PB(624)

# Name       : ASIC Reference Channel
# Data type  : 10 bits after the channel number. It represents the
# reference channel.# Max length : 624 (limited by the number of ASICs(i.e. 624))
#ttype# = REF_CHAN
#tform# = 1PJ(624)
ttype# = Reference_Channels
tform# = 1PI(624)

# Name       : ASIC Common Noise Data
# Data type  : last 10 bits representing the common noise for this asic
# Max length : 624 (limited by the number of ASICs(i.e. 624))
#ttype# = ASIC_CMN
#tform# = 1PJ(624)
ttype# = CommonNoise_Channels
tform# = 1PI(624)

# Name       : Raw ADC Data
# Data type  : 10-bit pulse height data packet to signed short (16bits = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Raw_ADC_Pulse_Heights
tform# = 1PI(39936)

# Name       : Calibrated ADC Data
# Data type  : 10-bit pulse height data packet to signed short (16bits = I)
# Max length : 39936 (624 chips * 64 channels)
ttype# = Calibrated_ADC_Pulse_Heights
#tform# = 1PJ(39936)
tform# = 1PI(39936)
