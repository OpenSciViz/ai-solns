xtension= bintable
extname = events
telescop= astro-h
instrume = HXI

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

ttype# = Occurance_ID
tform# = 1J

ttype# = Local_Time
tform# = 1J

ttype# = FLAG_SingleEventUpset
#tform# = 5X
tform# = 1B

ttype# = FLAG_LengthCheck
#tform# = 5X
tform# = 1B

ttype# = FLAG_InitialTriggerPattern
#tform# = 8X
tform# = 1B

ttype# = FLAG_TriggerPattern
#tform# = 8X
tform# = 1B

#ttype# = FLAG_Trigger
#tform# = 8X

ttype# = FLAG_PreciseShieldHitPattern
tform# = 1B

#ttype# = FLAG_HitPattern
#tform# = 2X

#ttype# = FLAG_FastBGO
#tform# = 2X

ttype# = FLAG_FastShieldHitPattern
tform# = 1B

#1J or 24X (actually 24-bit width)
ttype# = LiveTime
tform# = 1J

ttype# = nHitASICs
tform# = 1B

#------------------------------------------------------------
# Original ASIC Event Data
#------------------------------------------------------------
# Data output from each ASIC have
# ASIC ID(8bits), 000 & 5-bit flag(8bits), Ch Data Bits(4bytes),
# CM(1bit), Reference Ch(10bits), 
# ADC Data (10bits x 32ch max = 320bits max),
# Common Mode Noise Data (10bits), and 0s for filling.
#Therefore, the maximum length of one ASIC data is
# 1 + 1 + 4 + pack_16bits(1+10+320+10+fill)
# = 6+pack_16bits(341) = 6+pack_16bits(352)
# = 6+22 = 28bytes

# One HXI has 40 ASIC chips, and therefore, the max length of
# the event data part (of the raw event telemetry) becomes
# 40chips x 28bytes = 1120bytes

# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = RAW_ASIC_DATA
#tform# = 1PB(1120)
#hon: June15 2012 B34 R256 meeting yielded this:
#hon: each (occurance) group of variable-length blocks of ASIC TLM data
#hon: (or blob of blocks) has a fixed-length TLM (primary) header sized
#hon: to 7 shorts (14 bytes). each individual ASIC block of chan adc data
#hon: has a 59 bit header (of which the active chan bit flag is crucial)
#hon: summing header (i.e. active chan bit flag) bits + ref chan + 
#hon: 32 (max) 10bit adc chans + 10bit noise chan + stop bit yields:
#hon: 16+16+16+11+(32*10)+10+1 == 390 bits
#hon: 390.0/16 == 24.375 must be rounded up to 25 shorts
#hon: 25*40 ==> 1000 + 7 (primary) header shorts ==> 1007 shorts
tform# = 1PB(2014)
#------------------------------------------------------------
#------------------------------------------------------------
# Columns added for SFF
#------------------------------------------------------------
#------------------------------------------------------------

# Name       : ASIC Header Data
# Data type  : 24 bits information packed to 32bits and treated as Unsigned Integer (UK)
# hon: or UJ ? but fancy envent telemetry chart shows 3*16 == 48bits?# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = ASIC_Header_Data
# tform# = 1PUK(40)
tform# = 1PJ(40)

# Name       : Original ASIC IDs
# Data type  : Unsinged byte
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = Original_ASIC_IDs
tform# = 1PB(40)


# Name       : Remapped ASIC IDs
# Data type  : Unsinged byte
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = Remapped_ASIC_IDs
tform# = 1PB(40)

# Name       : Original Channel bit flags
# Data type  : Unsigned short (2byte integer = UI) or X bits?
# Max length : 1280 (40chips*32channels)
ttype# = Original_Channel_BitFlags
#hon: bits?
#tform# = 1PX(1280)
tform# = 1PUJ(40)

# Name       : Remapped Channel IDs
# Data type  : Unsigned short (2byte integer = UI)
# Max length : 1280 (40chips*32channels)
ttype# = Remapped_Channel_IDs
tform# = 1PI(1280)

# Calculated as the sum of the active channel x ASIC.
# hon -- number of active chanel per asic to 40 hxi asics
# This value is max of 32 for HXI and 64 for SGD

#ttype# = NUM_ACTIVE_CHAN
#tform# = 1PI(40)
ttype# = NumberOf_Active_Channels
tform# = 1PB(40)

# Name       : ASIC Reference Channel
# Data type  : 10 bits after the channel number. It represents the
# reference channel.# Max length : 40 (limited by the number of ASICs(i.e. 40))
#ttype# = REF_CHAN
#tform# = 1PJ(40)
ttype# = Reference_Channels
tform# = 1PI(40)

# Name       : ASIC Common Noise Data
# Data type  : last 10 bits representing the common noise for this asic
# Max length : 40 (limited by the number of ASICs(i.e. 40))
#ttype# = ASIC_CMN
#tform# = 1PJ(40)
ttype# = CommonNoise_Channels
tform# = 1PI(40)

# Name       : Raw ADC Data of each remapped ch
# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)
# Max length : 1280 (40chips*32channels)
#ttype# = ADC_PH
ttype# = Raw_ADC_Pulse_Heights
tform# = 1PI(1280)

# Name       : Calibrated ADC Data of each remapped ch
# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)
# Max length : 1280 (40chips*32channels)
#ttype# = ADC_PI
ttype# = Calibrated_ADC_Pulse_Heights
#tform# = 1PJ(1280)
tform# = 1PI(1280)
