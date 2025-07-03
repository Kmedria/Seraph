#pragma once

#define DataBlockCodeType unsigned short // This is saved as a hash define, because the type is not decided on.
#define DataBlockSizeType unsigned short // it is also assumed that more than 65k bytes will not be used, so an unsigned short is used, 
										 // but it can easily be changed from the file