FROM ubuntu:22.04

# Install SNMP daemon and utilities
RUN apt-get update && apt-get install -y \
    snmpd \
    snmp \
    snmp-mibs-downloader \
    python3 \
    && download-mibs \
    && rm -rf /var/lib/apt/lists/*

# Create directory for SNMP daemon and state
RUN mkdir -p /var/run/snmpd /var/lib/snmp /usr/share/snmp/mibs

# Create snmp user and group
RUN groupadd -r snmp && useradd -r -g snmp snmp

# Set up MIB directories
RUN mkdir -p /usr/share/snmp/mibs/iana \
    /usr/share/snmp/mibs/ietf \
    /usr/share/snmp/mibs/site

# Copy SNMP configuration and MIBs
COPY snmpd.conf /etc/snmp/snmpd.conf
COPY powernet455.txt /usr/share/snmp/mibs/site/PowerNet-MIB.txt
COPY mibs/* /usr/share/snmp/mibs/site/

# Set proper permissions
RUN chown -R snmp:snmp /var/lib/snmp /usr/share/snmp/mibs && \
    chmod 755 /usr/share/snmp/mibs/*

# Configure SNMP to load MIBs
ENV MIBS=+ALL
ENV MIBDIRS=/usr/share/snmp/mibs/ietf:/usr/share/snmp/mibs/iana:/usr/share/snmp/mibs/site
ENV SNMP_PERSISTENT_DIR=/var/lib/snmp

# Expose SNMP ports
EXPOSE 161/udp
EXPOSE 162/udp

# Start SNMP daemon
CMD ["snmpd", "-f", "-Lo", "-c", "/etc/snmp/snmpd.conf"]