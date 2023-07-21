# trend
Truncate files keeping the last specified number of bytes

This works like the Linux truncate utility, but keeps the last number of specified bytes, so is ideal for truncating log files where you want to keep the latest log lines and drop the oldest.

Usage:
    trend -s <size>[K][M][G] file
