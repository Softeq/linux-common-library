#
# Docker image to test Softeq linux-common-library
#
FROM linux_common_builder

# copy system dbus service mockup
COPY components/system/tests/env/net.Sdbus.Moduletest.conf /usr/share/dbus-1/system.d

# dir for sysv service .pid file
RUN mkdir -p -m=777 /var/run/user/1000
