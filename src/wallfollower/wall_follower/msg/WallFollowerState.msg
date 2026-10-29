# Header copied from the latest scan.
# Timestamp is zero before any scan has arrived.
std_msgs/Header header

uint8 PAUSED=0
uint8 FOLLOWING=1
uint8 WAITING_FOR_SCAN=2
uint8 state

# True when following the wall on the left, false for the right.
# Changed at run time with the wall_follower/follow_left service.
bool follow_left

# True when a scan has arrived within the scan timeout.
bool scan_valid
