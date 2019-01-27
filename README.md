# ping_with_TTL

TTL = Time To Live is the 8 bit field in the IPv4 header. The maximum value can be 255 and minimum can be 0. This field limits the lifespan of data in the computer network. By default it is set to 64

TTL value decreases by 1 (one) each time it hits any gateway or router. If the value decreases to 0 (zero) and the targer IP is not reach, then the packet is dropped. 

In this tool, the TTL value is manualy configured. The task was to set the TTL value as required so that the packed is dropped at a certain router.
