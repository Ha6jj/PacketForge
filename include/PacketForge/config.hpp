#pragma once

// Enables aggressive bitewise copying of primitive types in containers (LE)
// *Ignores custom methods for primitive types
#ifndef PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION
    #define PACKET_FORGE_ENABLE_CONTAINER_FAST_SERIALIZATION 1
#endif

// Default SharedBufferPool size
// *Used only if enbled in CommandSuit
#ifndef DEFAULT_BUFFER_POOL_SIZE  
    #define DEFAULT_BUFFER_POOL_SIZE 1024
#endif