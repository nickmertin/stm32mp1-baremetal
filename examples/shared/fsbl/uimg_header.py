import struct
import sys
import time
import zlib

magic = 0x27051956
image_type_kernel = 2
image_type_firmware = 5
os_uboot = 17
arch_arm = 2
compress_none = 0
compress_gzip = 1
compress_bzip2 = 2

with open(sys.argv[1], "rb") as bin_file:
    payload = bin_file.read();

#TODO: these should be cmdline args:
loadaddr = 0xC2000040
entryaddr = 0xC2000040
image_name = bytes("stm32mp1-baremetal image", "ascii")
compress = compress_none
image_type = image_type_firmware

datalen = len(payload)
tmstamp = int(time.time())

data_crc = zlib.crc32(payload) & 0xffffffff

# To calc the header CRC, we generate a header with the CRC zero'ed out
# Then calc the CRC of that, then fill it back in
header_no_crc = struct.pack(">IIIIIIIbbbb32s", 
    magic,                   # Image Header Magic Number	
    0,
    tmstamp,                 # Image Creation Timestamp	
    datalen,                 # Image Data Size		
    loadaddr,                # Data Load Address		
    entryaddr,               # Entry Point Address		
    data_crc,                # Image Data CRC Checksum	
    os_uboot,                # Operating System		
    arch_arm,                # CPU architecture		
    image_type,              # Image Type			
    compress,                # Compression Type		
    image_name,              # Image Name		
    )

hcrc = (zlib.crc32(header_no_crc) & 0xffffffff).to_bytes(4, 'big')
header = bytearray(header_no_crc)
header[4] = hcrc[0]
header[5] = hcrc[1]
header[6] = hcrc[2]
header[7] = hcrc[3]

with open(sys.argv[2], "wb") as uimg_file:
    uimg_file.write(header + payload)

