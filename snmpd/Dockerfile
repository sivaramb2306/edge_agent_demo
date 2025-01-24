FROM ubuntu:22.04

# Install SNMP daemon and utilities
RUN apt-get update && apt-get install -y \
    snmpd \
    snmp \
    && rm -rf /var/lib/apt/lists/*

# Create directory for SNMP daemon
RUN mkdir -p /var/run/snmpd

# Copy SNMP configuration
COPY snmpd.conf /etc/snmp/snmpd.conf

# Expose SNMP ports
EXPOSE 161/udp
EXPOSE 162/udp

# Start SNMP daemon in foreground mode
CMD ["snmpd", "-f", "-Lo", "-c", "/etc/snmp/snmpd.conf"]