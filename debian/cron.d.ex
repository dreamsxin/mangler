#
# Regular cron jobs for the mangler package
#
0 4	* * *	root	[ -x /usr/bin/mangler_maintenance ] && /usr/bin/mangler_maintenance
