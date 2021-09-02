# docker run -v /etc/grumpyd:/etc/grumpyd
FROM debian:10

MAINTAINER Petr Bena <petr@bena.rocks>

RUN apt-get update && apt-get install qt5-default libqt5multimedia5 libqt5network5 libqt5qml5 -y

RUN mkdir /var/lib/grumpyd
RUN mkdir /var/lib/grumpyd/scripts
RUN mkdir /etc/grumpyd

RUN useradd grumpyd

RUN chown grumpyd -R /var/lib/grumpyd

COPY grumpy.ini /etc/grumpyd/
COPY server.key /etc/grumpyd/
COPY server.crt /etc/grumpyd/
COPY [ "libgp.so", "libgrumpycore.so", "libircclient.so", "libirc.so", "/usr/lib/" ]
COPY [ "grumpyd", "/usr/bin/" ]

RUN ldconfig

EXPOSE 6200 6208

USER grumpyd

CMD grumpyd -s -v -c /etc/grumpyd/grumpy.ini
