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
