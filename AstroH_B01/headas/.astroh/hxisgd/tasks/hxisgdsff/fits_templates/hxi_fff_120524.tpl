xtension=bintable

#number of rows
naxis2 = 1

# Description: Time, second from the epoch (TBD)
# Origin     : Pipe-line process, by time assignment process
# Origin     : Blank
ttype# = TIME
tform# = 1D

# Description: Time when the space packet was sent
# Origin     : Pre-Pipe line, calculated from SDTP header by SIRIUS
ttype# = S_TIME
tform# = 1D

# Description: CCSDS packet header
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = Packet_Header
tform# = 20B

# Description: Time Indicator
# Origin     : Pre-Pipe line, copy from CCSDS 2ndary header
ttype# = TI
tform# = 1J

# Description: Occurrence ID
# Origin     : Pre-Pipe line or Pipe line (to be discussed)
# Origin     : Blank or calcurated (TBD)
ttype# = Occurence_ID
tform# = 1J

# Description: Local time
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = Local_Time
tform# = 1J

# Description: single event upset occurred (1) or not (0)
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_SEU
tform# = 1B

# Description: length error (1) or ok (0)
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_LengthCheck
tform# = 1B

# Description: origin of trigger(s)
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_InitialTriggerFlag
tform# = 1B

# Description: trigger pattern during the occurrence
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_TriggerPattern
tform# = 1B

# Description: hit information by BGO shield
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_PreciseShieldHitPattern
tform# = 1B

# Description: fast veto signal from the BGO shield
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = FLAG_FastShieldHitPattern
tform# = 1B

# Description: time since the previous occurrence
# Origin     : Pre-Pipe line, copy from the telemetry
#1J or 24X (actually 24-bit width)
ttype# = LiveTime
tform# = 1J

# Description: number of asics of hits
# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = nHitASICs
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

# Origin     : Pre-Pipe line, copy from the telemetry
ttype# = RAW_ASIC_DATA
tform# = 1PB(1120)

