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
tform# = 5X

ttype# = FLAG_LengthCheck
tform# = 5X

ttype# = FLAG_TriggerPattern
tform# = 8X

ttype# = FLAG_Trigger
tform# = 8X

ttype# = FLAG_HitPattern
tform# = 2X

ttype# = FLAG_FastBGO
tform# = 2X

#1J or 24X (actually 24-bit width)
ttype# = LiveTime
tform# = 1J

ttype# = NUM_ASIC
tform# = 1B

#------------------------------------------------------------
# Original ASIC Event Data
#------------------------------------------------------------
# Data output from each ASIC have
# "ASIC ID(8bits)", "000 & 5-bit flag(8bits)", "Ch Data Bits(4bytes)",
# "CM(1bit)", "Reference Ch(10bits)", 
# "ADC Data (10bits x 32ch max = 320bits max)",
# "Common Mode Noise Data (10bits)", and 0s for filling.
#Therefore, the maximum length of one ASIC data is
# 1 + 1 + 4 + pack_16bits(1+10+320+10+fill)
# = 6+pack_16bits(341) = 6+pack_16bits(352)
# = 6+22 = 28bytes

# One HXI has 40 ASIC chips, and therefore, the max length of
# the event data part (of the raw event telemetry) becomes
# 40chips x 28bytes = 1120bytes

ttype# = RAW_ASIC_DATA
tform# = 1PB(1120)

#------------------------------------------------------------
#------------------------------------------------------------
# Columns added for SFF
#------------------------------------------------------------
#------------------------------------------------------------

# Name       : Remapped ASIC IDs
# Data type  : Unsinged byte
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = RMAP_ASIC_ID
tform# = 1PB(40)

# Name       : ASIC Header Data
# Data type  : 24 bits information packed to 32bits and treated Integer
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = ASIC_HEADER
tform# = 1PJ(40)

# Name       : ASIC Header Data
# Data type  : 10 bits after the channel number. It represents the 
# reference channel. 
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = REF_CHAN
tform# = 1PJ(40)

# Name       : ASIC Header Data
# Data type  : last 10 bits representing the common noise for this asic 
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = ASIC_CMN
tform# = 1PJ(40)


# Name       : Remapped Channel IDs
# Data type  : Unsigned short (2byte integer = UI)
# Max length : 1280 (40chips*32channels)
ttype# = RMAP_CHANNEL_ID
tform# = 1PJ(1280)

# Name       : Original ASIC IDs
# Data type  : Unsinged byte
# Max length : 40 (limited by the number of ASICs(i.e. 40))
ttype# = ASIC_ID
tform# = 1PB(40)

# Calculated as the sum of the active channel x ASIC. 
# This value is max of 32 for HXI and 64 for SGD
#
ttype# = NUM_ACTIVE_CHAN
tform# = 1PI(40)

# Name       : Original Channel IDs
# Data type  : Unsigned short (2byte integer = UI)
# Max length : 1280 (40chips*32channels)
ttype# = CHANNEL_ID
tform# = 1PJ(1280)

# Name       : Raw ADC Data of each remapped ch
# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)
# Max length : 1280 (40chips*32channels)
ttype# = ADC_PH
tform# = 1PJ(1280)

# Name       : Calibrated ADC PH of each remapped ch
# Data type  : 10-bit pulse height data packet to Unsigned short (16bits = UI)
# Max length : 1280 (40chips*32channels)
ttype# = ADC_PI
tform# = 1PJ(1280)
