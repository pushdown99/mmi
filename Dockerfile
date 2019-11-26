FROM ubuntu:16.04
MAINTAINER FOPIS IoT PLATFORM
ENV DEBIAN_FRONTEND noninteractive

ENV JAVA_HOME /usr/lib/jvm/java-8-oracle

# Copy compiled/downloaded Kaa .DEB file and install
ARG setupfile
COPY ["$setupfile", "/kaa-node.deb"]
RUN dpkg -i /kaa-node.deb \
	&& rm -R /kaa-node.deb \
	&& apt-get autoremove -y && apt-get clean

RUN touch /var/log/kaa/kaa-node.log \
	&& chown kaa:kaa /var/log/kaa/kaa-node.log

# Kaa service & convenience shell scripts
COPY kaa/ /kaa

EXPOSE 9090

ENTRYPOINT ["/kaa/docker-entry.sh"]
