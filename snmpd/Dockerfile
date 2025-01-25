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

# Set up MIB directories and download standard MIBs
RUN cd /usr/share/snmp && \
    mkdir -p /usr/share/snmp/mibs/iana && \
    mkdir -p /usr/share/snmp/mibs/ietf && \
    mkdir -p /usr/share/snmp/mibs/site && \
    apt-get update && \
    apt-get install -y wget && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/SNMPv2-MIB.txt -O /usr/share/snmp/mibs/ietf/SNMPv2-MIB.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/SNMPv2-SMI.txt -O /usr/share/snmp/mibs/ietf/SNMPv2-SMI.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/SNMPv2-TC.txt -O /usr/share/snmp/mibs/ietf/SNMPv2-TC.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/SNMPv2-CONF.txt -O /usr/share/snmp/mibs/ietf/SNMPv2-CONF.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/RFC1155-SMI.txt -O /usr/share/snmp/mibs/ietf/RFC1155-SMI.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/RFC1213-MIB.txt -O /usr/share/snmp/mibs/ietf/RFC1213-MIB.txt && \
    wget https://raw.githubusercontent.com/net-snmp/net-snmp/master/mibs/IANAifType-MIB.txt -O /usr/share/snmp/mibs/iana/IANAifType-MIB.txt && \
    apt-get remove -y wget && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

# Copy SNMP configuration and MIBs
COPY snmpd.conf /etc/snmp/snmpd.conf
COPY mibs/EDGE-AGENT-MIB.txt /usr/share/snmp/mibs/site/
COPY powernet455.txt /usr/share/snmp/mibs/site/PowerNet-MIB.txt

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

# Start SNMP daemon in foreground mode
CMD ["snmpd", "-f", "-Lo", "-c", "/etc/snmp/snmpd.conf"]